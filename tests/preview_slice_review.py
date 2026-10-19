# SPDX-License-Identifier: MIT
import csv
import hashlib
import importlib.util
import itertools
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('reviewer', ROOT / 'scripts/review-preview-slices.py')
reviewer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reviewer)
SOURCE = 'a' * 40


class PreviewEvidenceReview(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        (self.root / 'sawstar_premium_native_pair').write_bytes(b'never executed synthetic binary')
        (self.root / 'CMakeCache.txt').write_bytes(b'synthetic cache')
        self.manifest = dict(schema=2, state='complete', source_commit=SOURCE,
            executable_sha256=reviewer.sha(self.root / 'sawstar_premium_native_pair'),
            cmake_cache_sha256=reviewer.sha(self.root / 'CMakeCache.txt'),
            cold_runs_per_architecture=4, native_host_acceptance=False,
            production_promotion_allowed=False, results=[])
        for wave, arch, cold in ([(w, a, 0) for w in range(4) for a in ('arm64', 'x86_64')] +
                                [(2, a, c) for c in range(1, 4) for a in ('arm64', 'x86_64')]):
            name = arch + '-waveform-' + str(wave) + ('-cold-' + str(cold) if cold else '') + '.txt'
            with (self.root / name).open('w', newline='') as stream:
                writer = csv.writer(stream, lineterminator='\n')
                writer.writerow(reviewer.BASE_FIELDS + (reviewer.DIGEST_FIELDS if wave == 2 else ()))
                rows = []
                for rate, mode, voice, fx in itertools.product(
                        (48000, 96000, 176400, 192000, 384000), range(4), range(3), range(2)):
                    row = [str(v) for v in (wave, rate, mode, voice, fx)] + ['0'] * 4
                    if wave == 2:
                        row += ['0123456789abcdef'] * 4
                    writer.writerow(row)
                    rows.append(row)
            entry = dict(waveform=wave, architecture=arch, cold_run=cold,
                         translated=arch == 'x86_64', returncode=0, log_file=name,
                         log_sha256=reviewer.sha(self.root / name))
            if wave == 2:
                # Fixture digest from declared rows, not the function under test.
                table = sorted(tuple(row[:5] + row[9:]) for row in rows)
                entry['fingerprint_table_sha256'] = hashlib.sha256(
                    json.dumps(table, separators=(',', ':')).encode()).hexdigest()
            self.manifest['results'].append(entry)
        self.save()

    def save(self):
        (self.root / 'manifest.json').write_text(json.dumps(self.manifest), encoding='utf-8')

    def review(self):
        return reviewer.review(self.root, SOURCE)

    def mutate_csv(self, index, mutate):
        entry = self.manifest['results'][index]
        path = self.root / entry['log_file']
        with path.open(newline='') as stream:
            rows = list(csv.reader(stream))
        mutate(rows)
        with path.open('w', newline='') as stream:
            csv.writer(stream, lineterminator='\n').writerows(rows)
        entry['log_sha256'] = reviewer.sha(path)
        self.save()
        return entry, rows

    def test_complete_archive_is_read_only_and_never_executes(self):
        before = {p.name: p.read_bytes() for p in self.root.iterdir()}
        with patch.object(subprocess, 'run', side_effect=AssertionError('must not execute')), \
             patch.object(subprocess, 'check_output', side_effect=AssertionError('must not execute')):
            result = self.review()
        self.assertEqual(result['streams'], 14)
        self.assertEqual(result['case_observations'], 1680)
        self.assertFalse(result['native_host_acceptance'])
        self.assertFalse(result['production_promotion_allowed'])
        self.assertFalse(result['original_numerical_failure_resolved'])
        self.assertEqual(before, {p.name: p.read_bytes() for p in self.root.iterdir()})

    def test_every_binary_cache_and_log_hash_is_checked(self):
        for name in ['sawstar_premium_native_pair', 'CMakeCache.txt'] + [
                e['log_file'] for e in self.manifest['results']]:
            with self.subTest(name=name):
                path = self.root / name
                original = path.read_bytes()
                path.write_bytes(original + b'changed')
                with self.assertRaisesRegex(ValueError, 'SHA-256 mismatch'):
                    self.review()
                path.write_bytes(original)

    def test_rehashed_duplicate_case_is_rejected(self):
        self.mutate_csv(0, lambda rows: rows.__setitem__(2, list(rows[1])))
        with self.assertRaisesRegex(ValueError, 'case'):
            self.review()

    def test_rehashed_nonzero_and_truncated_csv_are_rejected(self):
        self.mutate_csv(0, lambda rows: rows[1].__setitem__(5, '0.000001'))
        with self.assertRaisesRegex(ValueError, 'Nonzero'):
            self.review()
        self.mutate_csv(0, lambda rows: rows.pop())
        with self.assertRaisesRegex(ValueError, 'Incomplete'):
            self.review()

    def test_all_zero_exits_cannot_hide_changed_cold_process(self):
        entry, rows = self.mutate_csv(8, lambda rows: rows[1].__setitem__(9, 'fedcba9876543210'))
        with self.assertRaisesRegex(ValueError, 'Reference/Candidate'):
            self.review()
        entry, rows = self.mutate_csv(8, lambda rows: rows[1].__setitem__(10, 'fedcba9876543210'))
        table = sorted(tuple(row[:5] + row[9:]) for row in rows[1:])
        entry['fingerprint_table_sha256'] = hashlib.sha256(
            json.dumps(table, separators=(',', ':')).encode()).hexdigest()
        self.save()
        with self.assertRaisesRegex(ValueError, 'Cold-process signal mismatch'):
            self.review()

    def test_failed_manifest_source_and_acceptance_are_rejected(self):
        for field, value in [('state', 'failed'), ('source_commit', 'b' * 40),
                             ('native_host_acceptance', True),
                             ('production_promotion_allowed', True),
                             ('cold_runs_per_architecture', 3)]:
            with self.subTest(field=field):
                original = self.manifest[field]
                self.manifest[field] = value
                self.save()
                with self.assertRaises(ValueError):
                    self.review()
                self.manifest[field] = original
        self.save()

    def test_failed_baseline_and_incomplete_or_duplicate_streams_rejected(self):
        self.manifest['results'][4]['returncode'] = 1
        self.save()
        with self.assertRaisesRegex(ValueError, 'Failed numerical stream'):
            self.review()
        self.manifest['results'][4]['returncode'] = 0
        last = self.manifest['results'].pop()
        self.save()
        with self.assertRaisesRegex(ValueError, 'Incomplete stream'):
            self.review()
        self.manifest['results'].append(dict(self.manifest['results'][12]))
        self.save()
        with self.assertRaisesRegex(ValueError, 'Stream identity/order'):
            self.review()
        self.manifest['results'][-1] = last

    def test_manifest_duplicate_keys_and_unsafe_paths_rejected(self):
        path = self.root / 'manifest.json'
        path.write_text('{"schema":2,"schema":2}', encoding='utf-8')
        with self.assertRaisesRegex(ValueError, 'Duplicate manifest key'):
            self.review()
        self.manifest['results'][0]['log_file'] = '../outside.txt'
        self.save()
        with self.assertRaisesRegex(ValueError, 'Unexpected log filename'):
            self.review()

    def test_cli_failure_exit_and_machine_readable_report(self):
        command = [sys.executable, str(ROOT / 'scripts/review-preview-slices.py'),
                   str(self.root), '--expected-source', SOURCE]
        result = subprocess.run(command, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(json.loads(result.stdout)['state'], 'verified')
        (self.root / 'CMakeCache.txt').unlink()
        result = subprocess.run(command, capture_output=True, text=True)
        self.assertEqual(result.returncode, 1)
        self.assertEqual(json.loads(result.stdout)['state'], 'failed')


if __name__ == '__main__':
    unittest.main()
