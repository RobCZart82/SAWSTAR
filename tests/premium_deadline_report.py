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
                                out.writerow((engine, rate, 16, mode, 20, 1, buffer, 256, pair,
                                              "study-first" if pair % 2 else "reference-first", 1, 1, 1, 1, 0, .1, .01, 0))
                                blocks.writerows((engine, rate, mode, buffer, pair, block, 1) for block in range(256))

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    def run_report(self, study, high_factor=4, backend="SSE2"):
        (self.root / "backend.txt").write_text(backend)
        (self.root / "factors.csv").write_text("rate,reference_factor,study_factor\n48000,4,4\n96000,4,4\n192000," + str(high_factor) + "," + str(high_factor) + "\n")
        with patch.dict(os.environ, SAWSTAR_PREMIUM_STUDY=study), patch.object(deadline.subprocess, "check_output", return_value="0" * 40), patch.object(deadline.platform, "platform", return_value="test-platform"), contextlib.redirect_stdout(io.StringIO()):
            deadline.report(self.root)
        return json.loads((self.root / "metadata.json").read_text())

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

    def test_incorrect_compiled_routing_rejected(self):
        for study, wrong in (("gain-normalization", 2), ("rate-gain-normalization", 4), ("rate-simd-fir", 4), ("rate-lookup", 4)):
            with self.subTest(study=study), self.assertRaisesRegex(ValueError, "routing"):
                self.run_report(study, wrong)

    def test_unknown_study_and_incorrect_backend_rejected(self):
        with self.assertRaisesRegex(ValueError, "study kind"):
            self.run_report("unknown")
        with self.assertRaisesRegex(ValueError, "backend"):
            self.run_report("rate-gain-normalization", 2, "scalar-source")
        with self.assertRaisesRegex(ValueError, "backend"):
            self.run_report("rate-lookup", 2, "scalar-source")


if __name__ == "__main__":
    unittest.main()
