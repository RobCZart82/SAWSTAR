#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Offline rejection controls for the archived-binary replay evidence parser."""
import csv
import importlib.util
import io
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location("replay_k1",
    Path(__file__).resolve().parents[1] / "scripts/replay-k1-original.py")
replay = importlib.util.module_from_spec(spec)
spec.loader.exec_module(replay)


def fixture(wave=2):
    fields = replay.FIELDS + (replay.DIGEST_FIELDS if wave == 2 else ())
    rows = []
    for rate in ("48000", "96000", "176400", "192000", "384000"):
        for mode in range(4):
            for voice in range(3):
                for fx in range(2):
                    row = [str(wave), rate, str(mode), str(voice), str(fx)] + ["0"] * 4
                    if wave == 2:
                        row += ["0123456789abcdef"] * 2 + ["fedcba9876543210"] * 2
                    rows.append(row)
    return fields, rows


def encode(fields, rows):
    stream = io.StringIO()
    writer = csv.writer(stream)
    writer.writerow(fields)
    writer.writerows(rows)
    return stream.getvalue()


class ReplayEvidenceTests(unittest.TestCase):
    def test_original_failure_cli_and_preceding_cases(self):
        self.assertEqual(replay.arguments(2), ["--diagnostic-waveform", "2"])
        self.assertEqual(replay.arguments(0), ["--waveform", "0"])
        fields, rows = fixture()
        self.assertEqual(rows[13][:5], ["2", "48000", "2", "0", "1"])
        self.assertEqual(len(rows[:13]), 13)
        self.assertIsNotNone(replay.validate_table(encode(fields, rows), 2))

    def test_original_fourteen_process_order(self):
        planned = replay.plan()
        self.assertEqual(len(planned), 14)
        self.assertEqual(planned[:6], [(0, "arm64", 0), (0, "x86_64", 0),
            (1, "arm64", 0), (1, "x86_64", 0), (2, "arm64", 0), (2, "x86_64", 0)])
        self.assertEqual(planned[-2:], [(2, "arm64", 3), (2, "x86_64", 3)])
        self.assertEqual(sum(wave == 2 for wave, _, _ in planned), 8)

    def test_native_intel_plan_preserves_historical_x86_order(self):
        self.assertEqual(replay.execution_plan("native-intel"), [
            (0, "x86_64", 0), (1, "x86_64", 0), (2, "x86_64", 0),
            (3, "x86_64", 0), (2, "x86_64", 1), (2, "x86_64", 2),
            (2, "x86_64", 3)])

    def test_default_plan_unchanged(self):
        self.assertEqual(replay.execution_plan("apple-silicon"), replay.plan())

    def test_unknown_execution_mode_rejected(self):
        with self.assertRaises(ValueError):
            replay.execution_plan("silent-fallback")

    def test_absent_translation_oid_is_not_rosetta(self):
        self.assertFalse(replay.translation_reported(0, ""))
        self.assertFalse(replay.translation_reported(1, ""))

    def test_positive_translation_probe_is_detected(self):
        self.assertTrue(replay.translation_reported(0, "1\n"))

    def test_explicit_native_translation_probe(self):
        self.assertFalse(replay.translation_reported(0, "0\n"))

    def test_unknown_translation_probe_value_rejected(self):
        with self.assertRaises(ValueError):
            replay.translation_reported(0, "unexpected")

    def test_normal_nine_column_control(self):
        fields, rows = fixture(0)
        self.assertIsNone(replay.validate_table(encode(fields, rows), 0))

    def test_nondiagnostic_triangle_supported_explicitly(self):
        fields, rows = fixture()
        text = encode(fields[:9], [row[:9] for row in rows])
        self.assertIsNone(replay.validate_table(text, 2, diagnostic=False))
        with self.assertRaisesRegex(ValueError, "header"):
            replay.validate_table(text, 2)

    def test_missing_case_rejected(self):
        fields, rows = fixture()
        with self.assertRaisesRegex(ValueError, "Incomplete"):
            replay.validate_table(encode(fields, rows[:-1]), 2)

    def test_duplicate_and_reordered_cases_rejected(self):
        fields, rows = fixture()
        rows[1] = rows[0][:]
        with self.assertRaisesRegex(ValueError, "Reordered"):
            replay.validate_table(encode(fields, rows), 2)
        fields, rows = fixture()
        rows[0], rows[1] = rows[1], rows[0]
        with self.assertRaisesRegex(ValueError, "Reordered"):
            replay.validate_table(encode(fields, rows), 2)

    def test_changed_audio_fingerprint_rejected(self):
        fields, rows = fixture()
        rows[0][10] = "0000000000000000"
        with self.assertRaisesRegex(ValueError, "fingerprints differ"):
            replay.validate_table(encode(fields, rows), 2)

    def test_cross_process_fingerprint_change_visible(self):
        fields, rows = fixture()
        first = replay.validate_table(encode(fields, rows), 2)
        rows[0][9] = rows[0][10] = "0000000000000000"
        self.assertNotEqual(first, replay.validate_table(encode(fields, rows), 2))

    def test_error_and_nonfinite_values_rejected(self):
        for value in ("1e-9", "nan", "inf", ""):
            fields, rows = fixture()
            rows[0][5] = value
            with self.subTest(value=value), self.assertRaises(ValueError):
                replay.validate_table(encode(fields, rows), 2)

    def test_malformed_digest_and_row_rejected(self):
        fields, rows = fixture()
        rows[0][9] = "not-a-digest"
        with self.assertRaisesRegex(ValueError, "Invalid signal fingerprint"):
            replay.validate_table(encode(fields, rows), 2)
        fields, rows = fixture()
        rows[0].append("unexpected")
        with self.assertRaisesRegex(ValueError, "Malformed"):
            replay.validate_table(encode(fields, rows), 2)

    def test_missing_fingerprint_header_rejected(self):
        fields, rows = fixture(0)
        with self.assertRaisesRegex(ValueError, "header"):
            replay.validate_table(encode(fields, rows), 2)

    def test_wrong_archive_rejected_before_any_extraction(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            archive = root / "untrusted.zip"
            archive.write_bytes(b"not the pinned original artifact")
            with self.assertRaisesRegex(ValueError, "ZIP SHA256"):
                replay.unpack_verified(archive, "candidate", root / "original")
            self.assertFalse((root / "original").exists())


if __name__ == "__main__":
    unittest.main()
