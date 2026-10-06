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
from release_draft import write_verified_draft

SCRIPT = Path(__file__).resolve().parents[1] / 'scripts/publish-release.py'
ROOT = Path(__file__).resolve().parents[1]

class DraftRefresh(unittest.TestCase):
    def exercise(self, release=None, refs=None, tags=None, fault=None):
        self.calls = []
        self.current = release
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            assets = root / 'assets'
            assets.mkdir()
            (assets / 'package.zip').write_bytes(b'new package')
            (assets / 'SHA256SUMS.txt').write_bytes(b'checksums')
            notes = root / 'notes.md'
            notes.write_text('final notes')
            def gh(*args):
                self.calls.append(args)
                if args[0] == 'api':
                    endpoint = args[1]
                    if '/releases?' in endpoint:
                        self.assertEqual(args[2:], ('--paginate', '--slurp'))
                        # The matching draft can be beyond the first page.
                        return json.dumps([[], [self.current] if self.current else []])
                    if '/git/matching-refs/' in endpoint:
                        return json.dumps(refs or [])
                    if '/git/tags/' in endpoint:
                        return json.dumps({'object': tags[endpoint.rsplit('/', 1)[1]]})
                    self.fail('Unexpected API: ' + endpoint)
                self.assertEqual(args[0], 'release')
                self.assertNotIn('delete', args)
                if args[1] in ('create', 'edit'):
                    self.assertIn('--draft', args)
                    self.assertEqual(args[args.index('--target') + 1], 'expected')
                    if self.current is None:
                        self.current = dict(id=9, draft=True, tag_name='v1.0.4', assets=[])
                    self.current['target_commitish'] = 'expected'
                    self.current['name'] = args[args.index('--title') + 1]
                elif args[1] == 'upload':
                    if fault == 'upload':
                        raise RuntimeError('upload failed')
                    self.current['assets'] = [dict(name=p.name, size=p.stat().st_size,
                        digest='sha256:' + hashlib.sha256(p.read_bytes()).hexdigest())
                        for p in sorted(assets.iterdir())]
                    if fault == 'missing':
                        self.current['assets'].pop()
                    if fault == 'checksum':
                        self.current['assets'][0]['digest'] = 'sha256:wrong'
                    if fault == 'target':
                        self.current['target_commitish'] = 'other'
                    if fault == 'published':
                        self.current['draft'] = False
                else:
                    self.fail('Unexpected mutation: ' + repr(args))
                return ''
            write_verified_draft(gh, 'owner/repo', 'v1.0.4', 'expected', notes, assets)

    def draft(self, **overrides):
        return dict(dict(id=9, draft=True, tag_name='v1.0.4', target_commitish='old',
                         assets=[dict(name='package.zip', size=3)]), **overrides)

    def test_create_and_refresh_are_private_and_finish_only_after_verification(self):
        for existing in (None, self.draft()):
            with self.subTest(existing=existing):
                self.exercise(existing)
                mutations = [c for c in self.calls if c[0] == 'release']
                self.assertEqual(mutations[0][1], 'edit' if existing else 'create')
                self.assertIn('in progress', mutations[0][mutations[0].index('--title') + 1])
                self.assertEqual('--clobber' in mutations[1], bool(existing))
                self.assertIn('--notes-file', mutations[-1])
                self.assertEqual(self.current['name'], 'SAWSTAR 1.0.4')

    def test_public_release_and_unknown_assets_are_untouched(self):
        for release in (self.draft(draft=False), self.draft(assets=[dict(name='unknown.exe', size=1)])):
            with self.subTest(release=release), self.assertRaises(ValueError):
                self.exercise(release)
            self.assertFalse(any(c[0] == 'release' for c in self.calls))

    def test_stale_lightweight_and_annotated_tags_are_untouched(self):
        for obj, tags in ((dict(type='commit', sha='old'), None),
                          (dict(type='tag', sha='annotation'), {'annotation': dict(type='commit', sha='old')})):
            with self.subTest(obj=obj), self.assertRaises(ValueError):
                self.exercise(self.draft(), [dict(ref='refs/tags/v1.0.4', object=obj)], tags)
            self.assertFalse(any(c[0] == 'release' for c in self.calls))

    def test_matching_annotated_tag_and_prefix_neighbor_are_allowed(self):
        self.exercise(self.draft(), [dict(ref='refs/tags/v1.0.4-rc1', object=dict(type='commit', sha='other')),
            dict(ref='refs/tags/v1.0.4', object=dict(type='tag', sha='annotation'))],
            {'annotation': dict(type='commit', sha='expected')})

    def test_upload_failure_and_bad_remote_verification_cannot_finish(self):
        for fault in ('upload', 'missing', 'checksum', 'target', 'published'):
            with self.subTest(fault=fault), self.assertRaises((RuntimeError, ValueError)):
                self.exercise(self.draft(), fault=fault)
            self.assertFalse(any('--notes-file' in c for c in self.calls))
            self.assertIn('in progress', self.current['name'])

class InstallerPackageLayout(unittest.TestCase):
    def test_windows_installer_reads_document_paths_from_package_layout(self):
        installer = (ROOT / 'packaging/windows/SAWSTAR.iss').read_text(encoding='utf-8')
        package_script = (ROOT / 'scripts/package-plugin.py').read_text(encoding='utf-8')
        self.assertIn('"{#Stage}\\docs\\INSTALLATION.md"', installer)
        self.assertIn('"{#Stage}\\docs\\SYSTEM_REQUIREMENTS.md"', installer)
        self.assertIn("'docs/INSTALLATION.md'", package_script)
        self.assertIn("'docs/SYSTEM_REQUIREMENTS.md'", package_script)

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
        for candidate in ('dev', 'rc1'):
            with self.subTest(candidate=candidate):
                result = self.run_guard(candidate)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertIn('skipped', result.stdout)

    def test_already_published_skips_before_builds_and_mutations(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'release.json').write_text(json.dumps(dict(
                version='1.0.4', candidate='', release_date='2026-10-01')))
            (root / 'docs').mkdir()
            (root / 'docs/RELEASE_NOTES_1.0.4.md').write_text('notes')
            def fake_gh(args, **kwargs):
                self.assertEqual(args, ['gh', 'api',
                    'repos/owner/repo/releases?per_page=100', '--paginate', '--slurp'])
                return json.dumps([[], [dict(tag_name='v1.0.4', draft=False)]])
            with patch('pathlib.Path.cwd', return_value=root), patch.dict(
                    os.environ, {'RELEASE_SHA': 'new-documentation', 'GH_REPO': 'owner/repo'}), patch(
                    'subprocess.check_output', side_effect=fake_gh) as api, patch(
                    'time.sleep', side_effect=AssertionError('must not wait')), patch(
                    'sys.stdout', new_callable=io.StringIO) as out:
                with self.assertRaises(SystemExit) as stopped:
                    runpy.run_path(str(SCRIPT), run_name='__main__')
                self.assertEqual(stopped.exception.code, 0)
                self.assertIn('already published', out.getvalue())
                self.assertEqual(api.call_count, 1)
            self.assertFalse((root / 'release-assets').exists())
            self.assertFalse((root / 'downloaded').exists())

    def test_release_preflight_api_error_is_not_successful_skip(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'release.json').write_text(json.dumps(dict(
                version='1.0.4', candidate='', release_date='2026-10-01')))
            (root / 'docs').mkdir()
            (root / 'docs/RELEASE_NOTES_1.0.4.md').write_text('notes')
            with patch('pathlib.Path.cwd', return_value=root), patch.dict(
                    os.environ, {'RELEASE_SHA': 'expected', 'GH_REPO': 'owner/repo'}), patch(
                    'subprocess.check_output', side_effect=subprocess.CalledProcessError(1, 'gh')):
                with self.assertRaises(subprocess.CalledProcessError):
                    runpy.run_path(str(SCRIPT), run_name='__main__')
            self.assertFalse((root / 'release-assets').exists())

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
                if '/releases?per_page=100' in args[2]:
                    return json.dumps([[]])
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
                 'LICENSE': b'mit', 'THIRD_PARTY_NOTICES.md': b'notices',
                 'docs/THIRD_PARTY.md': b'dependency review',
                 'docs/INSTALLATION.md': b'install',
                 'docs/RELEASE_NOTES_1.0.3.md': b'release notes',
                 'docs/SYSTEM_REQUIREMENTS.md': b'requirements',
                 'docs/manuals/README.md': b'manual index',
                 'docs/manuals/SAWSTAR-User-Manual-EN.pdf': b'%PDF-1.4\nEnglish',
                 'docs/manuals/SAWSTAR-User-Manual-HU.pdf': b'%PDF-1.4\nHungarian'}
        licenses = Path(__file__).resolve().parents[1] / 'third_party/licenses'
        for item in licenses.rglob('*'):
            if item.is_file():
                files['licenses/' + item.relative_to(licenses).as_posix()] = b'notice'
        files['licenses/SAWSTAR-BRANDING-LICENSE.txt'] = b'branding license'
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
            name = 'docs/manuals/SAWSTAR-User-Manual-HU.pdf'
            entries[:] = [item for item in entries if item[0] != name]
            del manifest['files_sha256'][name]
        with self.archive(remove) as z:
            with self.assertRaises(ValueError):
                validate_package(z, 'expected', '1.0.3')

    def test_rejects_missing_plugin_or_license(self):
        for missing in ('SAWSTAR.vst3/Contents/plugin', 'licenses/DaisySP-LICENSE.txt'):
            def remove(entries, manifest):
                index = next(i for i, pair in enumerate(entries) if pair[0] == missing)
                name, _ = entries.pop(index)
                del manifest['files_sha256'][name]
            with self.subTest(missing=missing), self.archive(remove) as z:
                with self.assertRaises(ValueError):
                    validate_package(z, 'expected', '1.0.3')


class PublisherRuntimeGuards(unittest.TestCase):
    # Execute the real publisher in a separate interpreter: -O must never
    # disable release validation. Mock only external GitHub/package I/O.
    RUNNER = r"""
import io, json, os, runpy, sys, zipfile
from pathlib import Path
from unittest.mock import patch
script, scenario = sys.argv[1:]
sys.path.insert(0, str(Path(script).parent))
root = Path.cwd()
os.environ.update(RELEASE_SHA='expected', GH_REPO='owner/repo')
metadata = dict(version='1.0.4', candidate='', release_date='2026-10-06')
if scenario == 'date':
    metadata['release_date'] = ''
(root / 'release.json').write_text(json.dumps(metadata))
(root / 'docs').mkdir()
if scenario != 'notes':
    (root / 'docs/RELEASE_NOTES_1.0.4.md').write_text('notes')
names = ['SAWSTAR-macos-universal-candidate',
         'SAWSTAR-windows-x64-candidate', 'SAWSTAR-windows-ARM64-candidate']
runs = [dict(id=i, path='.github/workflows/' + name, head_sha='expected',
             event='push', status='completed', conclusion='success')
        for i, name in enumerate(('build-macos.yml', 'build-windows.yml', 'quality.yml'), 1)]
def gh(args, **kwargs):
    if args[1] == 'api':
        endpoint = args[2]
        if '/releases?' in endpoint:
            return '[[]]'
        if '/actions/runs?' in endpoint:
            return json.dumps(dict(workflow_runs=runs))
        if endpoint.endswith('/artifacts'):
            run_id = int(endpoint.split('/')[-2])
            platform_names = names[:1] if run_id == 1 else names[1:] if run_id == 2 else []
            artifacts = [] if scenario == 'coverage' else [
                dict(name=name, expired=scenario == 'expired') for name in platform_names]
            return json.dumps(dict(artifacts=artifacts))
        raise RuntimeError('Unexpected API: ' + endpoint)
    if args[1:3] == ['run', 'download']:
        dest = Path(args[args.index('--dir') + 1])
        dest.mkdir(parents=True)
        mac = 'macos' in args[args.index('--name') + 1]
        with zipfile.ZipFile(dest / ('SAWSTAR-macos.zip' if mac else 'SAWSTAR-windows.zip'), 'w'):
            pass
        suffixes = ['.dmg', '.pkg'] if mac else ['.exe']
        if mac and scenario == 'duplicate_type':
            suffixes = ['.dmg', '.dmg']
        for index, suffix in enumerate(suffixes):
            version = '1.0.3' if scenario == 'version' else '1.0.4'
            prefix = 'foreign-' if scenario == 'prefix' else ''
            platform = dest.name.removesuffix('-candidate').removeprefix('SAWSTAR-')
            (dest / (prefix + 'SAWSTAR-' + version + '-' + platform + '-' + str(index) + suffix)).write_bytes(b'installer')
        return ''
    raise RuntimeError('Unexpected command: ' + repr(args))
def write_draft(*args):
    (root / 'draft-written').write_text('verified')
with patch('subprocess.check_output', side_effect=gh), \
     patch('release_validation.validate_package'), \
     patch('release_draft.write_verified_draft', side_effect=write_draft):
    runpy.run_path(script, run_name='__main__')
"""

    def exercise(self, scenario, optimized):
        with tempfile.TemporaryDirectory() as folder:
            args = [sys.executable] + (['-O'] if optimized else [])
            result = subprocess.run(args + ['-c', self.RUNNER, str(SCRIPT), scenario],
                                    cwd=folder, capture_output=True, text=True)
            return result, (Path(folder) / 'draft-written').exists()

    def test_invalid_final_release_blocked_with_and_without_optimization(self):
        expected = dict(date='Final release date required',
                        notes='Version-specific release notes required',
                        expired='Expired artifact',
                        coverage='Incomplete platform coverage',
                        duplicate_type='Missing or extra installer type',
                        version='Installer version mismatch',
                        prefix='Installer version mismatch')
        for optimized in (False, True):
            for scenario, message in expected.items():
                with self.subTest(optimized=optimized, scenario=scenario):
                    result, wrote = self.exercise(scenario, optimized)
                    self.assertNotEqual(result.returncode, 0, result.stdout)
                    self.assertIn(message, result.stderr)
                    self.assertFalse(wrote)

    def test_valid_final_release_reaches_draft_in_both_modes(self):
        for optimized in (False, True):
            with self.subTest(optimized=optimized):
                result, wrote = self.exercise('valid', optimized)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertTrue(wrote)

if __name__ == '__main__':
    unittest.main()
