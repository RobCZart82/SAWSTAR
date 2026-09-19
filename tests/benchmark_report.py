# SPDX-License-Identifier: MIT
import importlib.util
from pathlib import Path
import unittest
from unittest.mock import patch
import contextlib
import io
spec = importlib.util.spec_from_file_location("benchmark", Path(__file__).resolve().parents[1] / "scripts/benchmark-audio.py")
benchmark = importlib.util.module_from_spec(spec)
spec.loader.exec_module(benchmark)
HEADER = "scene,voices,realtime_percent,peak_dbfs,rms_dbfs,dc\n"
class ReportTest(unittest.TestCase):
    def test_valid_report(self):
        report = benchmark.parse_report(HEADER + "dry,1,2,-3,-12,0\n")
        self.assertEqual(report[("dry",1)]["realtime_percent"],2)
        self.assertEqual(benchmark.median_report([report]*3),report)
    def test_balanced_order_and_warmup_exclusion(self):
        calls = []
        def fake_run(binary):
            calls.append(binary)
            value = 999 if len(calls) <= 2 else (2 if binary == "old" else 3)
            return HEADER + f"dry,1,{value},-3,-12,0\n"
        with patch.object(benchmark, "run", side_effect=fake_run), contextlib.redirect_stdout(io.StringIO()):
            reports = benchmark.collect_reports({"baseline": "old", "current": "new"}, 4)
        self.assertEqual(calls, ["old", "new", "old", "new", "new", "old", "old", "new", "new", "old"])
        self.assertEqual([x[("dry",1)]["realtime_percent"] for x in reports["baseline"]], [2]*4)
        self.assertEqual([x[("dry",1)]["realtime_percent"] for x in reports["current"]], [3]*4)
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
if __name__ == "__main__": unittest.main()
