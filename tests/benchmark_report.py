# SPDX-License-Identifier: MIT
import importlib.util
from pathlib import Path
import unittest
spec = importlib.util.spec_from_file_location("benchmark", Path(__file__).resolve().parents[1] / "scripts/benchmark-audio.py")
benchmark = importlib.util.module_from_spec(spec)
spec.loader.exec_module(benchmark)
HEADER = "scene,voices,realtime_percent,peak_dbfs,rms_dbfs,dc\n"
class ReportTest(unittest.TestCase):
    def test_valid_report(self):
        report = benchmark.parse_report(HEADER + "dry,1,2,-3,-12,0\n")
        self.assertEqual(report[("dry",1)]["realtime_percent"],2)
        self.assertEqual(benchmark.median_report([report]*3),report)
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
