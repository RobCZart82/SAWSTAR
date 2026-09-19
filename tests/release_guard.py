# SPDX-License-Identifier: MIT
"""The release guard must skip candidates without creating or querying releases."""
import io
import hashlib
import zipfile
import warnings
import os
import runpy
from unittest.mock import patch
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from release_validation import matching_runs, workflows_ready, validate_package

SCRIPT = Path(__file__).resolve().parents[1] / 'scripts/publish-release.py'

class ReleaseGuard(unittest.TestCase):
    def run_guard(self, candidate):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder)
            (path / 'release.json').write_text(json.dumps({
                'version': '1.0.3', 'candidate': candidate, 'release_date': ''}))
            # No credentials, RELEASE_SHA or gh executable: candidate must stop first.
            result = subprocess.run([sys.executable, str(SCRIPT)], cwd=folder,
                                    env={}, capture_output=True, text=True)
            self.assertEqual(sorted(p.name for p in path.iterdir()), ['release.json'])
            return result

    def test_candidate_is_successful_skip(self):
        result = self.run_guard('rc1')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('skipped', result.stdout)

    def test_final_missing_release_context_still_fails(self):
        self.assertNotEqual(self.run_guard('').returncode, 0)

class FinalReleaseValidation(unittest.TestCase):
    def runs(self):
        return [dict(id=i, path='.github/workflows/' + name, head_sha='expected',
                     event='push', status='completed', conclusion='success')
                for i, name in enumerate(('build-macos.yml', 'build-windows.yml', 'quality.yml'), 1)]

    def test_all_matching_checks_required(self):
        runs = self.runs()
        self.assertTrue(workflows_ready(matching_runs(runs, 'expected')))
        self.assertFalse(workflows_ready(matching_runs(runs[:2], 'expected')))
        for field, value in [('head_sha', 'other'), ('event', 'pull_request'), ('status', 'in_progress')]:
            runs = self.runs()
            runs[-1][field] = value
            self.assertFalse(workflows_ready(matching_runs(runs, 'expected')))

    def test_failed_checks_cannot_publish(self):
        for conclusion in ('failure', 'cancelled', 'skipped', 'timed_out', 'action_required'):
            for index in range(3):
                runs = self.runs()
                runs[index]['conclusion'] = conclusion
                with self.assertRaises(ValueError):
                    workflows_ready(matching_runs(runs, 'expected'))

    def test_newest_run_supersedes_old_success(self):
        runs = self.runs()
        runs.append(dict(runs[-1], id=99, conclusion='failure'))
        with self.assertRaises(ValueError):
            workflows_ready(matching_runs(runs, 'expected'))

    def test_publisher_blocks_before_artifacts_or_release(self):
        for kind in ('missing', 'other_sha', 'failed'):
            runs = self.runs()
            if kind == 'missing':
                runs.pop()
            elif kind == 'other_sha':
                runs[-1]['head_sha'] = 'other'
            else:
                runs[-1]['conclusion'] = 'failure'
            def fake_gh(args, **kwargs):
                self.assertEqual(args[:2], ['gh', 'api'])
                self.assertIn('/actions/runs?head_sha=expected&event=push', args[2])
                return json.dumps({'workflow_runs': runs})
            with tempfile.TemporaryDirectory() as folder:
                root = Path(folder)
                (root / 'release.json').write_text(json.dumps(dict(
                    version='1.0.3', candidate='', release_date='2026-09-19')))
                (root / 'docs').mkdir()
                (root / 'docs/RELEASE_NOTES_1.0.3.md').write_text('notes')
                with patch('pathlib.Path.cwd', return_value=root), patch.dict(
                        os.environ, {'RELEASE_SHA': 'expected', 'GH_REPO': 'owner/repo'}), patch(
                        'subprocess.check_output', side_effect=fake_gh), patch(
                        'time.monotonic', side_effect=[0, 2101]):
                    with self.assertRaises(ValueError if kind == 'failed' else RuntimeError):
                        runpy.run_path(str(SCRIPT), run_name='__main__')
                self.assertFalse((root / 'release-assets').exists())

    def archive(self, mutation=None):
        files = {'SAWSTAR.vst3/Contents/plugin': b'binary',
                 'docs/manuals/SAWSTAR-User-Manual-EN.pdf': b'english',
                 'docs/manuals/SAWSTAR-User-Manual-HU.pdf': b'hungarian'}
        manifest = dict(source_commit='expected', version='1.0.3',
                        files_sha256={k: hashlib.sha256(v).hexdigest() for k, v in files.items()})
        entries = list(files.items())
        if mutation:
            mutation(entries, manifest)
        data = io.BytesIO()
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', UserWarning)
            with zipfile.ZipFile(data, 'w') as z:
                for name, content in entries:
                    z.writestr(name, content)
                z.writestr('PACKAGE-MANIFEST.json', json.dumps(manifest))
        data.seek(0)
        return zipfile.ZipFile(data)

    def test_exact_valid_archive(self):
        with self.archive() as z:
            validate_package(z, 'expected', '1.0.3')

    def test_rejects_extra_missing_duplicate_tampered_and_wrong_identity(self):
        cases = [lambda e, m: e.append(('stale.txt', b'extra')),
                 lambda e, m: e.pop(),
                 lambda e, m: e.append(e[0]),
                 lambda e, m: e.__setitem__(0, (e[0][0], b'tampered')),
                 lambda e, m: m.update(source_commit='other'),
                 lambda e, m: m.update(version='1.0.2'),
                 lambda e, m: e.append(('PACKAGE-MANIFEST.json', b'{}'))]
        for case in cases:
            with self.subTest(case=case), self.archive(case) as z:
                with self.assertRaises(ValueError):
                    validate_package(z, 'expected', '1.0.3')

    def test_rejects_unsafe_even_when_hashed(self):
        for name in ('../escape', '/absolute', 'a/../escape', 'a//b', 'folder/', 'C:/drive', 'a\\b'):
            def add(entries, manifest):
                entries.append((name, b'x'))
                manifest['files_sha256'][name] = hashlib.sha256(b'x').hexdigest()
            with self.subTest(name=name), self.archive(add) as z:
                with self.assertRaises(ValueError):
                    validate_package(z, 'expected', '1.0.3')

    def test_missing_manual_not_excused_by_matching_manifest(self):
        def remove(entries, manifest):
            name, _ = entries.pop()
            del manifest['files_sha256'][name]
        with self.archive(remove) as z:
            with self.assertRaises(ValueError):
                validate_package(z, 'expected', '1.0.3')

if __name__ == '__main__':
    unittest.main()
