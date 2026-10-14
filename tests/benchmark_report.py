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

def full_report(cpu=2):
    return HEADER + "".join(
        f"{scene},{voices},{cpu},-3,-12,0\n"
        for scene in ("dry_fx", "full_fx") for voices in (1, 8, 16)
    )

class ReportTest(unittest.TestCase):
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

    def test_shuffled_complete_report_is_valid(self):
        lines = full_report().splitlines(keepends=True)
        self.assertEqual(benchmark.parse_report(lines[0] + "".join(reversed(lines[1:]))),
                         benchmark.parse_report(full_report()))
if __name__ == "__main__": unittest.main()
