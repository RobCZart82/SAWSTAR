# SPDX-License-Identifier: MIT
import importlib.util
from pathlib import Path
import unittest
from unittest.mock import patch
import contextlib
import io
import tempfile
import json
import hashlib
import subprocess
spec = importlib.util.spec_from_file_location("benchmark", Path(__file__).resolve().parents[1] / "scripts/benchmark-audio.py")
benchmark = importlib.util.module_from_spec(spec)
spec.loader.exec_module(benchmark)
HEADER = "scene,voices,realtime_percent,peak_dbfs,rms_dbfs,dc\n"

def full_report(cpu=2):
    return HEADER + "".join(
        f"{scene},{voices},{cpu},-3,-12,0\n"
        for scene in ("dry_fx", "full_fx") for voices in (1, 8, 16)
    )

class ReportTest(unittest.TestCase):
    def test_evidence_preserves_all_attempts_without_changing_order_or_results(self):
        with tempfile.TemporaryDirectory() as temp:
            directory = Path(temp) / "evidence"
            evidence = benchmark.EvidenceRecorder(directory, {"test": True})
            with patch.object(benchmark, "run", return_value=full_report()) as run, \
                    contextlib.redirect_stdout(io.StringIO()):
                reports = benchmark.collect_reports({"baseline": "old", "current": "new"}, 2, evidence)
            self.assertEqual(run.call_count, 6)
            self.assertEqual(len(reports["current"]), 2)
            records = json.loads((directory / "manifest.json").read_text())["records"]
            self.assertEqual([(r["label"], r["phase"], r["repeat"]) for r in records],
                [("baseline", "warmup", 0), ("current", "warmup", 0),
                 ("baseline", "timed", 1), ("current", "timed", 1),
                 ("current", "timed", 2), ("baseline", "timed", 2)])
            for record in records:
                data = (directory / record["file"]).read_bytes()
                self.assertEqual(data, full_report().encode())
                self.assertEqual(hashlib.sha256(data).hexdigest(), record["sha256"])
            with self.assertRaises(FileExistsError):
                benchmark.EvidenceRecorder(directory, {})

    def test_evidence_keeps_invalid_warmup_and_timed_output_before_rejection(self):
        for outputs in ([HEADER], [full_report(), full_report(), HEADER]):
            with self.subTest(attempt=len(outputs)), tempfile.TemporaryDirectory() as temp:
                evidence = benchmark.EvidenceRecorder(Path(temp) / "evidence", {})
                with patch.object(benchmark, "run", side_effect=outputs) as run, \
                        contextlib.redirect_stdout(io.StringIO()):
                    with self.assertRaises(RuntimeError):
                        benchmark.collect_reports({"baseline": "old", "current": "new"}, 2, evidence)
                self.assertEqual(run.call_count, len(outputs))
                records = evidence.manifest["records"]
                self.assertEqual(len(records), len(outputs))
                self.assertEqual((evidence.directory / records[-1]["file"]).read_text(), HEADER)
                self.assertEqual(evidence.manifest["status"], "incomplete")

    def test_evidence_preserves_failed_process_and_stops(self):
        with tempfile.TemporaryDirectory() as temp:
            evidence = benchmark.EvidenceRecorder(Path(temp) / "evidence", {})
            error = subprocess.CalledProcessError(7, "old", output="partial output\n")
            with patch.object(benchmark, "run", side_effect=error) as run:
                with self.assertRaises(subprocess.CalledProcessError):
                    benchmark.collect_reports({"baseline": "old", "current": "new"}, 2, evidence)
            self.assertEqual(run.call_count, 1)
            record = evidence.manifest["records"][0]
            self.assertEqual(record["returncode"], 7)
            self.assertEqual((evidence.directory / record["file"]).read_text(), "partial output\n")
            evidence.finish("failed", "test failure")
            manifest = json.loads((evidence.directory / "manifest.json").read_text())
            self.assertEqual(manifest["status"], "failed")
            self.assertEqual(len(manifest["records"]), 1)

    def test_build_identity_records_flags_and_binary_hash(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            binary = root / "binary"
            binary.write_bytes(b"test executable")
            (root / "CMakeCache.txt").write_text(
                "CMAKE_CXX_FLAGS_RELEASE:STRING=-O3 -DNDEBUG\nPRIVATE_TOKEN:STRING=excluded\n")
            flags = root / "CMakeFiles/sawstar_engine.dir/flags.make"
            flags.parent.mkdir(parents=True)
            flags.write_text("CXX_FLAGS = -O3 -DNDEBUG\n")
            compiler = root / "CMakeFiles/version/CMakeCXXCompiler.cmake"
            compiler.parent.mkdir(parents=True)
            compiler.write_text('set(CMAKE_CXX_COMPILER_ID "AppleClang")\nset(CMAKE_CXX_COMPILER_VERSION "21.0")\n')
            identity = benchmark.build_identity(root, binary)
            self.assertEqual(identity["binary_sha256"], hashlib.sha256(binary.read_bytes()).hexdigest())
            self.assertEqual(identity["cmake_cache"], ["CMAKE_CXX_FLAGS_RELEASE:STRING=-O3 -DNDEBUG"])
            self.assertEqual(identity["target_flags"]["CMakeFiles/sawstar_engine.dir/flags.make"], flags.read_text())
            self.assertIn('set(CMAKE_CXX_COMPILER_VERSION "21.0")', identity["compiler"]["CMakeFiles/version/CMakeCXXCompiler.cmake"])

    def test_valid_report(self):
        report = benchmark.parse_report(full_report())
        self.assertEqual(len(report), 6)
        self.assertEqual(report[("dry_fx",1)]["realtime_percent"],2)
        self.assertEqual(benchmark.median_report([report]*3),report)
    def test_balanced_order_and_warmup_exclusion(self):
        calls = []
        def fake_run(binary):
            calls.append(binary)
            value = 999 if len(calls) <= 2 else (2 if binary == "old" else 3)
            return full_report(value)
        with patch.object(benchmark, "run", side_effect=fake_run), contextlib.redirect_stdout(io.StringIO()):
            reports = benchmark.collect_reports({"baseline": "old", "current": "new"}, 4)
        self.assertEqual(calls, ["old", "new", "old", "new", "new", "old", "old", "new", "new", "old"])
        self.assertEqual([x[("dry_fx",1)]["realtime_percent"] for x in reports["baseline"]], [2]*4)
        self.assertEqual([x[("dry_fx",1)]["realtime_percent"] for x in reports["current"]], [3]*4)
    def test_invalid_warmup_is_not_ignored(self):
        with patch.object(benchmark, "run", return_value=HEADER + "dry,1,nan,-3,-12,0\n"):
            with self.assertRaisesRegex(RuntimeError, "finite"):
                benchmark.collect_reports({"baseline": "old", "current": "new"}, 2)
    def test_unbalanced_measurement_count_rejected(self):
        for count in (0, 1, 3):
            with self.assertRaises(ValueError):
                benchmark.collect_reports({}, count)
    def test_invalid_numbers(self):
        for field in range(4):
            for bad in ("nan", "inf", "-inf", "1e999"):
                with self.subTest(field=field,bad=bad):
                    values=["2","-3","-12","0"]
                    values[field]=bad
                    with self.assertRaisesRegex(RuntimeError,"finite"):
                        benchmark.parse_report(HEADER + "dry,1," + ",".join(values) + "\n")
    def test_duplicate_and_empty(self):
        for text in (HEADER,HEADER+"dry,1,2,-3,-12,0\n"*2):
            with self.assertRaises(RuntimeError): benchmark.parse_report(text)

    def test_each_required_scenario_must_be_present(self):
        lines = full_report().splitlines(keepends=True)
        for index in range(1, 7):
            with self.subTest(missing=index):
                with self.assertRaisesRegex(RuntimeError, "missing="):
                    benchmark.parse_report("".join(lines[:index] + lines[index + 1:]))

    def test_matching_incomplete_warmups_cannot_start_measurements(self):
        incomplete = full_report().replace("full_fx,16,2,-3,-12,0\n", "")
        with patch.object(benchmark, "run", return_value=incomplete) as run:
            with self.assertRaisesRegex(RuntimeError, "missing="):
                benchmark.collect_reports({"baseline": "old", "current": "new"}, 2)
        self.assertEqual(run.call_count, 1)

    def test_matching_incomplete_timed_results_rejected(self):
        incomplete = full_report().replace("full_fx,16,2,-3,-12,0\n", "")
        with patch.object(benchmark, "run", side_effect=[full_report(), full_report(), incomplete]), \
                contextlib.redirect_stdout(io.StringIO()):
            with self.assertRaisesRegex(RuntimeError, "missing="):
                benchmark.collect_reports({"baseline": "old", "current": "new"}, 2)

    def test_unknown_scenario_or_voice_count_rejected(self):
        for old, new in (("dry_fx,1,", "dry,1,"), ("full_fx,16,", "full_fx,32,")):
            with self.assertRaisesRegex(RuntimeError, "unexpected="):
                benchmark.parse_report(full_report().replace(old, new))

    def test_zero_or_negative_cpu_rejected(self):
        for cpu in (0, -1):
            with self.assertRaisesRegex(RuntimeError, "Non-positive"):
                benchmark.parse_report(full_report(cpu))

    def test_malformed_headers_and_rows_rejected(self):
        for text in (full_report().replace("scene,voices,", "scene,scene,"),
                     full_report().replace("rms_dbfs,dc\n", "rms_dbfs,dc,extra\n"),
                     full_report().replace("dry_fx,1,2,-3,-12,0", "dry_fx,1,2,-3,-12"),
                     full_report().replace("dry_fx,1,2,-3,-12,0", "dry_fx,1,2,-3,-12,0,extra"),
                     full_report().replace("dry_fx,1,2,", "dry_fx,bad,2,"),
                     full_report().replace("dry_fx,1,2,", "dry_fx,1,bad,")):
            with self.assertRaises(RuntimeError):
                benchmark.parse_report(text)

    def test_current_dc_outlier_cannot_hide_in_median(self):
        for dc in (0.2, -0.2):
            bad = full_report().replace("full_fx,16,2,-3,-12,0\n",
                                        f"full_fx,16,2,-3,-12,{dc}\n")
            # One of eight current reports is unsafe; the old median was zero.
            outputs = [full_report()] * 18
            outputs[8] = bad
            with self.subTest(dc=dc), patch.object(benchmark, "run", side_effect=outputs), \
                    contextlib.redirect_stdout(io.StringIO()):
                with self.assertRaisesRegex(RuntimeError, "DC"):
                    benchmark.collect_reports({"baseline": "old", "current": "new"})

    def test_current_warmup_dc_is_checked(self):
        bad = full_report().replace(",-3,-12,0\n", ",-3,-12,0.2\n")
        with patch.object(benchmark, "run", side_effect=[full_report(), bad] + [full_report()] * 16) as run:
            with self.assertRaisesRegex(RuntimeError, "DC"):
                benchmark.collect_reports({"baseline": "old", "current": "new"})
        self.assertEqual(run.call_count, 2)

    def test_current_dc_boundaries_and_baseline_policy(self):
        baseline = full_report().replace(",-3,-12,0\n", ",-3,-12,0.2\n")
        for dc in (-0.01, 0.01):
            current = full_report().replace(",-3,-12,0\n", f",-3,-12,{dc}\n")
            with patch.object(benchmark, "run", side_effect=lambda binary: baseline if binary == "old" else current), \
                    contextlib.redirect_stdout(io.StringIO()):
                reports = benchmark.collect_reports({"baseline": "old", "current": "new"}, 2)
            self.assertEqual(len(reports["current"]), 2)

    def test_shuffled_complete_report_is_valid(self):
        lines = full_report().splitlines(keepends=True)
        self.assertEqual(benchmark.parse_report(lines[0] + "".join(reversed(lines[1:]))),
                         benchmark.parse_report(full_report()))
if __name__ == "__main__": unittest.main()
