# SPDX-License-Identifier: MIT
"""Synthetic archive composition controls; not Windows/M1 CPU evidence."""
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import sys
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


comparison = load('target_comparison', ROOT / 'scripts/compare-dispatch-engine-targets.py')
fixture = load('archive_fixture', ROOT / 'tests/dispatch_engine_archive.py')


class TargetComparison(unittest.TestCase):
    def setUp(self):
        self.case = fixture.ArchiveReview()
        self.case.setUp()
        self.addCleanup(self.case.tearDown)
        self.win = self.case.root
        # The second archive must be a sibling, not extra evidence inside the first.
        self.mac = self.win.with_name(self.win.name + '-mac')
        shutil.copytree(self.win, self.mac)
        self.addCleanup(shutil.rmtree, self.mac)
        self.configure(self.win, 'Windows-11-test', 'AMD64')
        self.configure(self.mac, 'macOS-test', 'arm64')

    def configure(self, path, platform, machine, source=None):
        manifest = comparison.reviewer.document(path / 'manifest.json')
        manifest.update(platform=platform, machine=machine)
        if source:
            manifest['source_sha'] = source
        for run in manifest['runs']:
            grid = path / run['folder']
            payload = comparison.reviewer.document(grid / 'paired-results.json')
            payload['metadata'].update(platform=platform, machine=machine, source_sha=manifest['source_sha'])
            for name, value in [('paired-results.json', payload), ('metadata.json', payload['metadata'])]:
                (grid / name).write_text(json.dumps(value), encoding='utf-8')
            run['files_sha256'] = {p.relative_to(grid).as_posix(): comparison.reviewer.digest(p)
                                   for p in grid.rglob('*') if p.is_file()}
        (path / 'manifest.json').write_text(json.dumps(manifest), encoding='utf-8')

    def test_complete_both_targets_preserve_negative_results_and_controls(self):
        before = {str(p): comparison.reviewer.digest(p)
                  for root in (self.win, self.mac) for p in root.rglob('*') if p.is_file()}
        # Raw numeric validation is covered by dispatch_engine_archive.py; these
        # placeholders test orchestration and require every grid to reach it.
        with patch.object(comparison.reviewer, 'verify_raw_grid') as raw:
            result = comparison.compare(self.win, self.mac)
        self.assertEqual(raw.call_count, 64)
        self.assertEqual([len(t['rows']) for t in result['targets']], [288, 288])
        self.assertFalse(result['native_host_acceptance'])
        self.assertFalse(result['production_promotion_allowed'])
        text = comparison.render(result)
        self.assertIn('1.200000', text)
        self.assertIn('4096', text)
        self.assertIn('reference-repeat', text)
        self.assertIn('macOS Apple Silicon', text)
        self.assertEqual(before, {str(p): comparison.reviewer.digest(p)
                                 for root in (self.win, self.mac) for p in root.rglob('*') if p.is_file()})
        self.assertEqual(json.loads(json.dumps(result))['ratio_direction'], 'study/reference')

    def test_different_source_rejected_even_after_internal_hashes_updated(self):
        self.configure(self.mac, 'macOS-test', 'arm64', source='c' * 40)
        with patch.object(comparison.reviewer, 'verify_raw_grid'):
            with self.assertRaisesRegex(ValueError, 'source_sha'):
                comparison.compare(self.win, self.mac)

    def test_wrong_platform_and_intel_mac_rejected(self):
        for platform, machine in [('Linux-test', 'arm64'), ('macOS-test', 'x86_64')]:
            self.configure(self.mac, platform, machine)
            with patch.object(comparison.reviewer, 'verify_raw_grid'):
                with self.assertRaisesRegex(ValueError, 'Apple Silicon'):
                    comparison.compare(self.win, self.mac)

    def test_malformed_platform_rejected_as_validation_error(self):
        self.configure(self.mac, None, 'arm64')
        with patch.object(comparison.reviewer, 'verify_raw_grid'):
            with self.assertRaisesRegex(ValueError, 'Invalid target'):
                comparison.compare(self.win, self.mac)

    def test_incomplete_archive_rejected(self):
        manifest = comparison.reviewer.document(self.mac / 'manifest.json')
        manifest['state'] = 'incomplete'
        (self.mac / 'manifest.json').write_text(json.dumps(manifest), encoding='utf-8')
        with patch.object(comparison.reviewer, 'verify_raw_grid'):
            with self.assertRaisesRegex(ValueError, 'Incomplete'):
                comparison.compare(self.win, self.mac)

    def test_different_source_file_hashes_rejected(self):
        manifest = comparison.reviewer.document(self.mac / 'manifest.json')
        manifest['source_file_sha256']['src/engine/Synth.cpp'] = 'd' * 64
        for run in manifest['runs']:
            grid = self.mac / run['folder']
            payload = comparison.reviewer.document(grid / 'paired-results.json')
            payload['metadata']['source_file_sha256'] = manifest['source_file_sha256']
            for name, value in [('paired-results.json', payload), ('metadata.json', payload['metadata'])]:
                (grid / name).write_text(json.dumps(value), encoding='utf-8')
            run['files_sha256'] = {p.relative_to(grid).as_posix(): comparison.reviewer.digest(p)
                                   for p in grid.rglob('*') if p.is_file()}
        (self.mac / 'manifest.json').write_text(json.dumps(manifest), encoding='utf-8')
        with patch.object(comparison.reviewer, 'verify_raw_grid'):
            with self.assertRaisesRegex(ValueError, 'source_file_sha256'):
                comparison.compare(self.win, self.mac)

    def test_grid_target_label_must_match_manifest(self):
        manifest = comparison.reviewer.document(self.win / 'manifest.json')
        manifest['machine'] = 'x86_64'
        (self.win / 'manifest.json').write_text(json.dumps(manifest), encoding='utf-8')
        with patch.object(comparison.reviewer, 'verify_raw_grid'):
            with self.assertRaisesRegex(ValueError, 'Grid platform/machine'):
                comparison.compare(self.win, self.mac)

    def test_same_archive_rejected(self):
        with self.assertRaisesRegex(ValueError, 'distinct'):
            comparison.compare(self.win, self.win)

    def test_second_target_raw_failure_propagates_without_success(self):
        with patch.object(comparison.reviewer, 'verify_raw_grid',
                          side_effect=[None] * 32 + [ValueError('second raw grid invalid')]) as raw:
            with self.assertRaisesRegex(ValueError, 'second raw'):
                comparison.compare(self.win, self.mac)
        self.assertEqual(raw.call_count, 33)

    def test_cli_invalid_raw_has_no_partial_stdout(self):
        result = subprocess.run([sys.executable, str(ROOT / 'scripts/compare-dispatch-engine-targets.py'),
                                 str(self.win), str(self.mac), '--format', 'json'],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 1)
        self.assertEqual(result.stdout, '')
        self.assertIn('Target comparison failed', result.stderr)


if __name__ == '__main__':
    unittest.main()
