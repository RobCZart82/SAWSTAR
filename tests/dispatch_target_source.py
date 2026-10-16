# SPDX-License-Identifier: MIT
"""Real Git source/checkout regressions; no timing campaigns or CPU claims."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('source_preflight', ROOT / 'scripts/check-dispatch-target-source.py')
preflight = importlib.util.module_from_spec(spec)
spec.loader.exec_module(preflight)


class SourcePreflight(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.folder = Path(self.temp.name)
        self.source = self.folder / 'source with spaces'
        self.daisy = self.source / 'third_party/DaisySP'
        for path in (self.source, self.daisy):
            path.mkdir(parents=True, exist_ok=True)
            self.git(path, 'init', '-q')
            self.git(path, 'config', 'user.name', 'SAWSTAR test')
            self.git(path, 'config', 'user.email', 'test@example.invalid')
            self.git(path, 'config', 'core.autocrlf', 'false')
        (self.daisy / 'dsp.h').write_bytes(b'// pinned dependency\n')
        self.git(self.daisy, 'add', 'dsp.h')
        self.git(self.daisy, 'commit', '-qm', 'dependency')
        daisy_sha = self.git(self.daisy, 'rev-parse', 'HEAD')
        (self.source / '.gitattributes').write_bytes(b'*.cpp text eol=lf\n')
        (self.source / 'engine.cpp').write_bytes(b'// engine\n')
        self.git(self.source, 'add', '.gitattributes', 'engine.cpp')
        self.git(self.source, 'update-index', '--add', '--cacheinfo',
                 '160000,' + daisy_sha + ',third_party/DaisySP')
        self.git(self.source, 'commit', '-qm', 'source')
        self.evidence = dict(state='complete', raw_measurements_verified=True,
                             native_host_acceptance=False, production_promotion_allowed=False,
                             source_sha=self.git(self.source, 'rev-parse', 'HEAD'), DaisySP_sha=daisy_sha,
                             source_file_sha256={'engine.cpp': hashlib.sha256(b'// engine\n').hexdigest()})
        self.reference = self.folder / 'reference.json'
        self.save()

    def git(self, folder, *args):
        return subprocess.check_output(['git', '-C', str(folder), *args],
                                       text=True, stderr=subprocess.PIPE).strip()

    def save(self):
        self.reference.write_text(json.dumps(self.evidence), encoding='utf-8')

    def check(self):
        return preflight.check(self.source, self.reference)

    def test_clean_exact_source_passes_without_timing_or_acceptance(self):
        result = self.check()
        self.assertTrue(result['source_preflight_passed'])
        self.assertEqual(result['source_files_checked'], 1)
        for key in ('measurement_started', 'native_host_acceptance', 'production_promotion_allowed'):
            self.assertFalse(result[key])
        self.assertEqual(self.git(self.source, 'status', '--porcelain'), '')

    def test_newer_main_commit_rejected_even_if_engine_bytes_match(self):
        self.git(self.source, 'commit', '--allow-empty', '-qm', 'docs only')
        with self.assertRaisesRegex(ValueError, 'Source commit differs'):
            self.check()

    def test_clean_git_crlf_checkout_is_not_normalized_for_hash(self):
        (self.source / '.gitattributes').write_bytes(b'*.cpp text eol=crlf\n')
        self.git(self.source, 'add', '.gitattributes')
        self.git(self.source, 'commit', '-qm', 'CRLF checkout policy')
        self.evidence['source_sha'] = self.git(self.source, 'rev-parse', 'HEAD')
        self.save()
        (self.source / 'engine.cpp').unlink()
        self.git(self.source, 'checkout', '--', 'engine.cpp')
        # Git checks out CRLF from the LF blob and considers it clean; raw bytes must fail.
        self.assertEqual(self.git(self.source, 'status', '--porcelain'), '')
        with self.assertRaisesRegex(ValueError, 'Source byte mismatch'):
            self.check()
        self.assertEqual((self.source / 'engine.cpp').read_bytes(), b'// engine\r\n')

    def test_untracked_source_and_dirty_dependency_rejected(self):
        for path in (self.source / 'extra.txt', self.daisy / 'extra.txt'):
            path.write_bytes(b'untracked')
            with self.assertRaisesRegex(ValueError, 'clean source'):
                self.check()
            path.unlink()

    def test_changed_dependency_commit_rejected(self):
        self.git(self.daisy, 'commit', '--allow-empty', '-qm', 'changed dependency')
        with self.assertRaisesRegex(ValueError, 'DaisySP checkout differs'):
            self.check()

    def test_invalid_or_escaping_reference_paths_rejected(self):
        for name in ('../reference.json', '/engine.cpp', 'dir\\engine.cpp'):
            self.evidence['source_file_sha256'] = {name: 'a' * 64}
            self.save()
            with self.assertRaisesRegex(ValueError, 'Unsafe source path'):
                self.check()

    def test_unreviewed_reference_cannot_pass(self):
        self.evidence['raw_measurements_verified'] = False
        self.save()
        with self.assertRaisesRegex(ValueError, 'raw-reviewed'):
            self.check()

    def test_duplicate_reference_key_rejected(self):
        self.reference.write_text('{"state":"complete","state":"complete"}')
        with self.assertRaisesRegex(ValueError, 'Duplicate reference key'):
            self.check()

    def test_cli_failure_has_no_success_payload(self):
        self.git(self.source, 'commit', '--allow-empty', '-qm', 'changed source')
        result = subprocess.run([sys.executable, str(ROOT / 'scripts/check-dispatch-target-source.py'),
                                 '--source', str(self.source), '--reference', str(self.reference)],
                                capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(result.stdout, '')
        self.assertIn('Source commit differs', result.stderr)


if __name__ == '__main__':
    unittest.main()
