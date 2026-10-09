# SPDX-License-Identifier: MIT
"""Exercise the real deadline reporter, including compiled routing records."""
import contextlib
import csv
import importlib.util
import io
import json
import os
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location("deadline", Path(__file__).resolve().parents[1] / "scripts/report-premium-deadline.py")
deadline = importlib.util.module_from_spec(spec)
spec.loader.exec_module(deadline)


class DeadlineReport(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.root = Path(cls.temp.name)
        with (cls.root / "summary.csv").open("w", newline="") as summary, (cls.root / "blocks.csv").open("w", newline="") as raw:
            out, blocks = csv.writer(summary), csv.writer(raw)
            out.writerow("engine,rate,voices,filter_mode,drive_db,fx,buffer,blocks,pair,order,median_block_percent,p95_block_percent,p99_block_percent,worst_block_percent,over_budget_blocks,peak,rms,checksum".split(","))
            blocks.writerow("engine,rate,filter_mode,buffer,pair,block,block_percent".split(","))
            for engine in ("reference", "study"):
                for rate in (48000, 96000, 192000):
                    for mode in range(4):
                        for buffer in (32, 64, 128):
                            for pair in range(4):
                                percent = 2 if engine == "study" else 1
                                if rate == 192000 and buffer == 32:
                                    # Median of paired ratios = 3.5; ratio of medians differs.
                                    percent = ((2, 8, 30, 101) if engine == "study" else (1, 2, 10, 20))[pair]
                                if engine == "study" and rate == 96000 and buffer == 32 and pair == 3:
                                    percent = 100  # Exactly at budget is not a miss.
                                out.writerow((engine, rate, 16, mode, 20, 1, buffer, 256, pair,
                                              "study-first" if pair % 2 else "reference-first", percent, percent, percent, percent, 256 if percent > 100 else 0, .1, .01, 0))
                                blocks.writerows((engine, rate, mode, buffer, pair, block, percent) for block in range(256))

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    def run_report(self, study, high_factor=4, backend="SSE2", workload="stationary-v1", requested="stationary-v1"):
        if study == 'rate-saw-dispatch':
            (self.root / 'study.txt').write_text('saw-dispatch-v1')
            (self.root / 'waveform.txt').write_text('0')
            (self.root / 'compiler').mkdir(exist_ok=True)
            (self.root / 'compiler/CMakeCXXCompiler.cmake').write_text('synthetic compiler fixture')
        if study == 'rate-unrolled-fir':
            (self.root / 'study.txt').write_text('2x-interpolation-unroll-v1')
        (self.root / "workload.txt").write_text(workload)
        (self.root / "backend.txt").write_text(backend)
        (self.root / "factors.csv").write_text("rate,reference_factor,study_factor\n48000,4,4\n96000,4,4\n192000," + str(high_factor) + "," + str(high_factor) + "\n")
        captured = io.StringIO()
        with patch.dict(os.environ, SAWSTAR_PREMIUM_STUDY=study, SAWSTAR_PREMIUM_WORKLOAD=requested, SAWSTAR_OSC_WAVEFORM='0'), patch.object(deadline.subprocess, "check_output", return_value="0" * 40), patch.object(deadline.platform, "platform", return_value="test-platform"), contextlib.redirect_stdout(captured):
            deadline.report(self.root)
        self.output = captured.getvalue()
        return json.loads((self.root / "metadata.json").read_text())

    def test_machine_report_preserves_pairs_and_provenance(self):
        for workload in ('stationary-v1', 'modulated-v1'):
            with self.subTest(workload=workload):
                metadata = self.run_report('rate-unrolled-fir', 2, workload=workload, requested=workload)
                payload = json.loads((self.root / 'paired-results.json').read_text())
                emitted = [line for line in self.output.splitlines() if line.startswith('DEADLINE_CI_REPORT=')]
                self.assertEqual(len(emitted), 1)
                self.assertEqual(json.loads(emitted[0].split('=', 1)[1]), payload)
                self.assertEqual(payload['metadata'], metadata)
                self.assertEqual(payload['schema_version'], 1)
                self.assertEqual(payload['ratio_direction'], 'study/reference')
                self.assertFalse(payload['native_host_acceptance'])
                rows = payload['paired_results']
                self.assertEqual({(row['rate'], row['buffer']) for row in rows},
                                 {(rate, buffer) for rate in (48000, 96000, 192000) for buffer in (32, 64, 128)})
                self.assertEqual(len(rows), 9)
                for row in rows:
                    expected = 3.5 if (row['rate'], row['buffer']) == (192000, 32) else 2
                    self.assertEqual(row['median_paired_p50_ratio'], expected)
                    self.assertEqual(row['median_paired_p99_ratio'], expected)
                    self.assertEqual(row['paired_observations'], 16)
                    self.assertEqual(row['blocks_per_path'], 4096)
                    self.assertEqual(row['reference_over_budget_blocks'], 0)
                    expected_misses = 1024 if (row['rate'], row['buffer']) == (192000, 32) else 0
                    self.assertEqual(row['study_over_budget_blocks'], expected_misses)
                    self.assertIn(f"| {row['rate']} | {row['buffer']} | {expected:.6f} | {expected:.6f} | 0 | {expected_misses} |", self.output)
        self.run_report('rate-lookup', 2)
        self.assertNotIn('DEADLINE_CI_REPORT=', self.output)

    def test_fixed_and_adaptive_normalization_are_distinct(self):
        fixed = self.run_report("gain-normalization")
        adaptive = self.run_report("rate-gain-normalization", 2)
        self.assertEqual(fixed["drive_factor"], 4)
        self.assertIsNone(adaptive["drive_factor"])
        self.assertEqual(adaptive["drive_factors_by_rate"], {"48000": 4, "96000": 4, "192000": 2})
        self.assertEqual(adaptive["rate_policy"], "normalized-176400-boundary")
        self.assertFalse(adaptive["native_host_acceptance"])
        lookup = self.run_report("rate-lookup", 2)
        self.assertEqual(lookup["study_kind"], "rate-lookup")
        self.assertEqual(lookup["drive_factors_by_rate"], adaptive["drive_factors_by_rate"])
        self.assertEqual(lookup["workload"], "stationary-v1")
        self.assertFalse(lookup["timed_control_events"])

    def test_workload_labels_and_mismatches(self):
        result = self.run_report("rate-lookup", 2, workload="modulated-v1", requested="modulated-v1")
        self.assertEqual(result["workload"], "modulated-v1")
        self.assertTrue(result["timed_control_events"])
        self.assertEqual(result["drive_targets_db"], [0, 12, 24])
        self.assertEqual(set(result["measurement_sha256"]), {"summary.csv", "blocks.csv", "factors.csv", "backend.txt", "workload.txt"})
        for compiled, requested in (("modulated-v1", "stationary-v1"), ("stationary-v1", "modulated-v1"), ("unknown", "unknown")):
            with self.subTest(compiled=compiled, requested=requested), self.assertRaisesRegex(ValueError, "workload"):
                self.run_report("rate-lookup", 2, workload=compiled, requested=requested)
        (self.root / "workload.txt").unlink()
        with self.assertRaises(FileNotFoundError):
            deadline.report(self.root)

    def test_incorrect_compiled_routing_rejected(self):
        for study, wrong in (("gain-normalization", 2), ("rate-gain-normalization", 4), ("rate-simd-fir", 4), ("rate-lookup", 4), ("rate-unrolled-fir", 4)):
            with self.subTest(study=study), self.assertRaisesRegex(ValueError, "routing"):
                self.run_report(study, wrong)

    def test_unknown_study_and_incorrect_backend_rejected(self):
        with self.assertRaisesRegex(ValueError, "study kind"):
            self.run_report("unknown")
        with self.assertRaisesRegex(ValueError, "backend"):
            self.run_report("rate-gain-normalization", 2, "scalar-source")
        with self.assertRaisesRegex(ValueError, "backend"):
            self.run_report("rate-lookup", 2, "scalar-source")
        with self.assertRaisesRegex(ValueError, "backend"):
            self.run_report("rate-unrolled-fir", 2, "scalar-source")

    def test_dispatch_has_distinct_engine_waveform_and_provenance(self):
        metadata = self.run_report('rate-saw-dispatch', 2)
        self.assertEqual(metadata['study_variant'], 'rate-lookup-saw-dispatch-v1')
        self.assertEqual(metadata['oscillator_waveform'], 0)
        self.assertFalse(metadata['production_activation'])
        self.assertIn('waveform.txt', metadata['measurement_sha256'])
        self.assertIn('experiments/oscillator/SawDispatchSevenSaw.cpp', metadata['source_file_sha256'])
        self.assertIn('experiments/premium_filter/PremiumDrive.h', metadata['source_file_sha256'])
        self.assertIn('compiler/CMakeCXXCompiler.cmake', metadata['compiler_files_sha256'])
        self.assertIn('DEADLINE_CI_REPORT=', self.output)
        for waveform in ('1', '2', '3'):
            (self.root / 'waveform.txt').write_text(waveform)
            with patch.dict(os.environ, SAWSTAR_PREMIUM_STUDY='rate-saw-dispatch', SAWSTAR_PREMIUM_WORKLOAD='stationary-v1', SAWSTAR_OSC_WAVEFORM=waveform), patch.object(deadline.subprocess, 'check_output', return_value='0' * 40), contextlib.redirect_stdout(io.StringIO()):
                deadline.report(self.root)
            self.assertEqual(json.loads((self.root / 'metadata.json').read_text())['oscillator_waveform'], int(waveform))
        with patch.dict(os.environ, SAWSTAR_PREMIUM_STUDY='rate-saw-dispatch', SAWSTAR_PREMIUM_WORKLOAD='stationary-v1', SAWSTAR_OSC_WAVEFORM='0'):
            for invalid in ('1', '4', 'nan'):
                (self.root / 'waveform.txt').write_text(invalid)
                with self.assertRaisesRegex(ValueError, 'waveform'): deadline.report(self.root)
            (self.root / 'waveform.txt').write_text('0')
            (self.root / 'study.txt').write_text('2x-interpolation-unroll-v1')
            with self.assertRaisesRegex(ValueError, 'oscillator study'): deadline.report(self.root)
        with self.assertRaisesRegex(ValueError, 'routing'): self.run_report('rate-saw-dispatch', 4)
        with self.assertRaisesRegex(ValueError, 'backend'): self.run_report('rate-saw-dispatch', 2, 'scalar-source')
        self.run_report('rate-saw-dispatch', 2)
        (self.root / 'compiler/CMakeCXXCompiler.cmake').unlink()
        with patch.dict(os.environ, SAWSTAR_PREMIUM_STUDY='rate-saw-dispatch', SAWSTAR_PREMIUM_WORKLOAD='stationary-v1', SAWSTAR_OSC_WAVEFORM='0'), patch.object(deadline.subprocess, 'check_output', return_value='0' * 40):
            with self.assertRaisesRegex(ValueError, 'compiler provenance'): deadline.report(self.root)

    def test_dispatch_rejects_different_paired_signal(self):
        summary = self.root / 'summary.csv'
        original = summary.read_bytes()
        try:
            for field in ('peak', 'rms', 'checksum'):
                with summary.open(newline='') as handle:
                    reader = csv.DictReader(handle)
                    fields, rows = reader.fieldnames, list(reader)
                row = next(row for row in rows if row['engine'] == 'study')
                row[field] = str(float(row[field]) + .001)
                with summary.open('w', newline='') as handle:
                    writer = csv.DictWriter(handle, fieldnames=fields)
                    writer.writeheader()
                    writer.writerows(rows)
                with self.subTest(field=field), self.assertRaisesRegex(ValueError, 'paired signal'):
                    self.run_report('rate-saw-dispatch', 2)
                summary.write_bytes(original)
        finally:
            summary.write_bytes(original)

    def test_unrolled_study_identity_and_provenance(self):
        result = self.run_report('rate-unrolled-fir', 2, workload='modulated-v1', requested='modulated-v1')
        self.assertEqual(result['reference_variant'], 'rate-lookup-loop')
        self.assertEqual(result['study_variant'], 'rate-lookup-unrolled')
        self.assertEqual(result['fir_unroll_scope'], '2x-interpolation-only-v1')
        self.assertIn('study.txt', result['measurement_sha256'])
        (self.root / 'study.txt').write_text('rate-lookup')
        with patch.dict(os.environ, SAWSTAR_PREMIUM_STUDY='rate-unrolled-fir', SAWSTAR_PREMIUM_WORKLOAD='modulated-v1'):
            with self.assertRaisesRegex(ValueError, 'Compiled FIR study'):
                deadline.report(self.root)
        (self.root / 'study.txt').unlink()
        with patch.dict(os.environ, SAWSTAR_PREMIUM_STUDY='rate-unrolled-fir', SAWSTAR_PREMIUM_WORKLOAD='modulated-v1'):
            with self.assertRaises(FileNotFoundError):
                deadline.report(self.root)


if __name__ == "__main__":
    unittest.main()

