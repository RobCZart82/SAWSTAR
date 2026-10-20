#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
import importlib.util
from pathlib import Path
import unittest

spec=importlib.util.spec_from_file_location("sanitized_runner",
    Path(__file__).resolve().parents[1]/"scripts/run-k1-sanitized.py")
runner=importlib.util.module_from_spec(spec)
spec.loader.exec_module(runner)


class SanitizerControlTests(unittest.TestCase):
    def test_address_report_accepted_with_failure(self):
        runner.classify_control("asan",-6,"ERROR: AddressSanitizer: heap-buffer-overflow")

    def test_undefined_report_accepted_with_failure(self):
        runner.classify_control("ubsan",-6,"runtime error: signed integer overflow: 2147483647 + 1")

    def test_success_never_counts_as_working_sanitizer(self):
        with self.assertRaises(ValueError):
            runner.classify_control("asan",0,"ERROR: AddressSanitizer: heap-buffer-overflow")

    def test_generic_crash_is_not_sanitizer_detection(self):
        for name in ("asan","ubsan"):
            with self.subTest(name=name),self.assertRaises(ValueError):
                runner.classify_control(name,-11,"Segmentation fault")

    def test_wrong_sanitizer_report_rejected(self):
        with self.assertRaises(ValueError):
            runner.classify_control("ubsan",1,"ERROR: AddressSanitizer: heap-buffer-overflow")


if __name__=="__main__":
    unittest.main()
