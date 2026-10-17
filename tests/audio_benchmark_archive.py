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
