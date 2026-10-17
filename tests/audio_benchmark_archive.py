# SPDX-License-Identifier: MIT
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('audit',
    Path(__file__).resolve().parents[1] / 'scripts/review-audio-benchmark.py')
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


class ArchiveTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.folder = Path(self.temp.name) / 'evidence'
        b = audit.benchmark
        self.recorder = b.EvidenceRecorder(self.folder, dict(baseline_commit=b.base,
            current_commit='a' * 40, workload_sha256='b' * 64, repeats=8,
            thresholds=dict(cpu_relative=b.CPU_RELATIVE_LIMIT, cpu_absolute=b.CPU_ABSOLUTE_LIMIT,
                            level_db=b.LEVEL_DB_LIMIT, dc=b.MAX_CURRENT_DC)))
        self.recorder.manifest['builds'] = {label: dict(binary_sha256='c' * 64)
                                           for label in ('baseline', 'current')}
        self.schedule = [('baseline', 'warmup', 0), ('current', 'warmup', 0)]
        for repeat in range(8):
            labels = ('baseline', 'current') if repeat % 2 == 0 else ('current', 'baseline')
            self.schedule.extend((label, 'timed', repeat + 1) for label in labels)

    def populate(self, current_cpu=2):
        for label, phase, repeat in self.schedule:
            cpu = current_cpu if label == 'current' else 2
            text = 'scene,voices,realtime_percent,peak_dbfs,rms_dbfs,dc\n'
            text += ''.join(f'{scene},{voices},{cpu},-3,-12,0\n'
                            for scene, voices in sorted(audit.benchmark.EXPECTED_SCENARIOS))
            self.recorder.record(label, phase, repeat, text)
        self.recorder.finish('failed' if current_cpu > 3 else 'success')

    def save(self):
        self.recorder.write_manifest()

    def mutate_raw(self, text):
        record = self.recorder.manifest['records'][1]
        (self.folder / record['file']).write_bytes(text.encode())
        record['sha256'] = hashlib.sha256(text.encode()).hexdigest()
        self.save()

    def test_success_and_negative_result_are_both_preserved(self):
        self.populate(current_cpu=4)
        result = audit.review(self.folder)
        self.assertTrue(result['regression_detected'])
        self.assertEqual(result['recorded_status'], 'failed')
        self.assertEqual(result['records'], 18)
        self.assertEqual(len(result['scenes']), 6)
        self.assertFalse(result['production_promotion_allowed'])
        self.assertFalse(result['native_host_acceptance'])

    def test_exact_absolute_boundary_is_not_relaxed(self):
        self.populate(current_cpu=3)
        self.assertFalse(audit.review(self.folder)['regression_detected'])

    def test_pairs_retain_outlier_and_order_without_replacing_median_gate(self):
        self.populate()
        record = self.recorder.manifest['records'][3]
        path = self.folder / record['file']
        data = path.read_bytes().replace(b',2,-3,-12,0', b',20,-3,-12,0')
        path.write_bytes(data)
        record['sha256'] = hashlib.sha256(data).hexdigest()
        self.save()
        result = audit.review(self.folder)
        self.assertFalse(result['regression_detected'])
        for row in result['scenes']:
            timing = row['timing_diagnostics']
            self.assertEqual(len(timing['pairs']), 8)
            self.assertEqual([p['repeat'] for p in timing['pairs']], list(range(1, 9)))
            self.assertEqual([p['first'] for p in timing['pairs']], ['baseline', 'current'] * 4)
            self.assertEqual(timing['pairs'][0]['current_cpu'], 20)
            self.assertEqual(timing['pair_threshold_exceedances'], 1)
            self.assertEqual(timing['variation']['current']['max'], 20)
            self.assertEqual(timing['variation']['current']['min'], 2)
            self.assertEqual(timing['variation']['current']['span_pct_of_median'], 900)
            self.assertFalse(timing['replaces_production_gate'])

    def test_observed_main_failure_cannot_be_accepted_by_pair_majority(self):
        self.populate()
        # The preserved main dry_fx/16 data: only two paired ratios exceed
        # the limit, but the required comparison of medians still fails.
        values = dict(
            baseline=[11.5395708, 11.3939709, 10.6453916, 15.6788125,
                      10.8907625, 10.7539667, 10.9694083, 13.5140833],
            current=[16.3196875, 11.3804542, 12.3714625, 16.8464125,
                     11.7780583, 17.1935, 12.86495, 15.4219417])
        for record in self.recorder.manifest['records']:
            if record['phase'] != 'timed':
                continue
            path = self.folder / record['file']
            value = values[record['label']][record['repeat'] - 1]
            data = path.read_bytes().replace(b'dry_fx,16,2,', f'dry_fx,16,{value},'.encode())
            path.write_bytes(data)
            record['sha256'] = hashlib.sha256(data).hexdigest()
        self.recorder.finish('failed')
        result = audit.review(self.folder)
        row = next(r for r in result['scenes'] if (r['scene'], r['voices']) == ('dry_fx', 16))
        self.assertTrue(row['cpu_regression'])
        self.assertTrue(result['regression_detected'])
        self.assertAlmostEqual(row['cpu_change_pct'], 26.48756, places=4)
        self.assertEqual(row['timing_diagnostics']['pair_threshold_exceedances'], 2)

    def test_changed_raw_bytes_rejected(self):
        self.populate()
        p = self.folder / self.recorder.manifest['records'][0]['file']
        p.write_bytes(p.read_bytes() + b'\n')
        with self.assertRaisesRegex(ValueError, 'hash'): audit.review(self.folder)

    def test_reordered_records_rejected(self):
        self.populate()
        m = self.recorder.manifest
        m['records'][4], m['records'][5] = m['records'][5], m['records'][4]
        self.save()
        with self.assertRaises(ValueError): audit.review(self.folder)

    def test_missing_pair_rejected(self):
        self.populate()
        self.recorder.manifest['records'].pop()
        self.save()
        with self.assertRaises(ValueError): audit.review(self.folder)

    def test_forged_success_rejected(self):
        self.populate(current_cpu=4)
        self.recorder.finish('success')
        with self.assertRaisesRegex(ValueError, 'status'): audit.review(self.folder)

    def test_extra_files_rejected(self):
        self.populate()
        (self.folder / 'unexpected.txt').write_text('not recorded')
        with self.assertRaises(ValueError): audit.review(self.folder)

    def test_threshold_edit_rejected(self):
        self.populate()
        self.recorder.manifest['metadata']['thresholds']['cpu_relative'] = 2
        self.save()
        with self.assertRaises(ValueError): audit.review(self.folder)

    def test_path_escape_rejected(self):
        self.populate()
        self.recorder.manifest['records'][0]['file'] = '../outside.csv'
        self.save()
        with self.assertRaises(ValueError): audit.review(self.folder)

    def test_warmup_dc_and_incomplete_scenes_rejected_even_with_valid_hash(self):
        self.populate()
        for content in ('scene,voices,realtime_percent,peak_dbfs,rms_dbfs,dc\n',
                        'scene,voices,realtime_percent,peak_dbfs,rms_dbfs,dc\n' + ''.join(
                            f'{s},{v},2,-3,-12,0.02\n' for s, v in sorted(audit.benchmark.EXPECTED_SCENARIOS))):
            with self.subTest(content=content):
                self.mutate_raw(content)
                with self.assertRaises(RuntimeError): audit.review(self.folder)

    def test_duplicate_json_key_rejected(self):
        self.populate()
        p = self.folder / 'manifest.json'
        p.write_text(p.read_text().replace('"schema": 1', '"schema": 1, "schema": 1'))
        with self.assertRaisesRegex(ValueError, 'Duplicate'): audit.review(self.folder)


if __name__ == '__main__':
    unittest.main()
