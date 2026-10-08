# SPDX-License-Identifier: MIT
import contextlib
import csv
import importlib.util
import io
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('report', Path(__file__).resolve().parents[1] / 'scripts/report-oscillator-cache.py')
reporter = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reporter)

class ReportTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.rows = []
        for path in ('reference', 'study'):
            for rate in (48000, 96000, 192000):
                for wave in range(4):
                    for moving in range(2):
                        for pair in range(4):
                            self.rows.append(dict(path=path, rate=rate, waveform=wave, modulated=moving,
                                pair=pair, order='study-first' if pair % 2 else 'reference-first',
                                oscillators=32, frames=8192,
                                seconds=((2, 8, 30, 100) if path == 'study' else (1, 2, 10, 20))[pair], energy=10))
    def tearDown(self):
        self.temp.cleanup()
    def run_report(self, source_sha='0' * 40):
        with (self.root / 'oscillators.csv').open('w', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=self.rows[0].keys())
            writer.writeheader(); writer.writerows(self.rows)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            result = reporter.report(self.root, source_sha, True)
        self.output = output.getvalue()
        return result
    def test_pairs_and_provenance(self):
        result = self.run_report()
        self.assertEqual(len(result['summary']), 24)
        self.assertEqual({row['median_paired_seconds_ratio'] for row in result['summary']}, {3.5})
        self.assertEqual(result['summary'][0]['paired_ratios'], [2, 4, 3, 5])
        self.assertFalse(result['metadata']['native_host_acceptance'])
        self.assertFalse(result['metadata']['production_activation'])
        self.assertEqual(result['metadata']['ratio_direction'], 'study/reference')
        self.assertEqual(len(result['metadata']['raw_sha256']), 64)
        self.assertEqual(len(result['metadata']['source_file_sha256']), 9)
        self.assertEqual(json.loads((self.root / 'report.json').read_text()), result)
        line = next(line for line in self.output.splitlines() if line.startswith('OSCILLATOR_CI_REPORT='))
        self.assertEqual(json.loads(line.split('=', 1)[1]), result)
    def test_missing_and_duplicate_pairs(self):
        removed = self.rows.pop()
        with self.assertRaisesRegex(ValueError, 'Incomplete'): self.run_report()
        self.rows.append(removed); self.rows.append(self.rows[0])
        with self.assertRaisesRegex(ValueError, 'duplicate'): self.run_report()
    def test_bad_signal_order_and_duration(self):
        original = self.rows[0].copy()
        for key, value, message in [('seconds', 0, 'measurement'), ('seconds', 'nan', 'measurement'),
                                    ('energy', 'inf', 'measurement'), ('order', 'study-first', 'order'),
                                    ('frames', 1, 'frame count'), ('oscillators', 1, 'bank')]:
            with self.subTest(key=key, value=value):
                self.rows[0] = dict(original, **{key: value})
                with self.assertRaisesRegex(ValueError, message): self.run_report()
        self.rows[0] = original
        self.rows[-1]['energy'] = 11
        with self.assertRaisesRegex(ValueError, 'energy differs'): self.run_report()
    def test_requires_exact_source(self):
        with self.assertRaisesRegex(ValueError, 'source SHA'): self.run_report('main')

if __name__ == '__main__':
    unittest.main()
