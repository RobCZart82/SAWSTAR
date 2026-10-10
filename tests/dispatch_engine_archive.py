# SPDX-License-Identifier: MIT
"""Archive corruption and semantic negative controls; no target CPU claim."""
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('archive_review', ROOT / 'scripts/review-dispatch-engine-target.py')
reviewer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reviewer)


class ArchiveReview(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.manifest = dict(schema_version=1, state='complete', source_sha='a' * 40,
                             rounds_requested=2, study_kind='rate-saw-dispatch', candidate_variant='saw-dispatch-v1',
                             configuration='Release', native_host_acceptance=False, production_promotion_allowed=False,
                             background_load_controlled=False, affinity_applied=False, platform='synthetic', machine='test',
                             source_file_sha256={'src/engine/Synth.cpp': 'b' * 64}, runs=[])
        qualification = ['configure.txt', 'build.txt', 'contracts.txt', 'complex-controls.txt',
                         *[f'waveform-{wave}-contracts.txt' for wave in range(4)]]
        for name in qualification:
            (self.root / name).write_bytes(b'qualification\r\n')
        self.manifest['build_evidence_sha256'] = {name: reviewer.digest(self.root / name) for name in qualification}
        compiler = b'compiler evidence\r\n'
        self.manifest['compiler_sha256'] = hashlib.sha256(compiler).hexdigest()
        scenes = [(w, v) for w in ('stationary-v1', 'modulated-v1') for v in range(4)]
        for index in range(2):
            comparisons = ('reference-repeat', 'candidate') if index == 0 else ('candidate', 'reference-repeat')
            for comparison in comparisons:
                for workload, wave in (scenes if index == 0 else reversed(scenes)):
                    folder = f'round-{index + 1:02d}/{comparison}/{workload}/waveform-{wave}'
                    grid = self.root / folder
                    (grid / 'compiler').mkdir(parents=True)
                    (grid / 'compiler/CMakeCXXCompiler.cmake').write_bytes(compiler)
                    for name in ('summary.csv', 'blocks.csv', 'factors.csv', 'backend.txt', 'report.md', 'measurement.txt', 'reporter.txt'):
                        (grid / name).write_bytes(b'synthetic evidence\r\n')
                    for name, value in [('comparison.txt', comparison), ('workload.txt', workload), ('waveform.txt', str(wave)), ('study.txt', 'saw-dispatch-v1')]:
                        (grid / name).write_text(value + '\n', encoding='utf-8')
                    meta = dict(source_sha=self.manifest['source_sha'], comparison=comparison, workload=workload,
                                oscillator_waveform=wave, source_file_sha256=self.manifest['source_file_sha256'],
                                configuration='Release', study_kind='rate-saw-dispatch', production_activation=False,
                                native_host_acceptance=False, reference_variant='rate-lookup-seven-saw',
                                study_variant='rate-lookup-seven-saw' if comparison == 'reference-repeat' else 'rate-lookup-saw-dispatch-v1',
                                measurement_sha256={name: reviewer.digest(grid / name) for name in ('summary.csv', 'blocks.csv', 'factors.csv', 'backend.txt', 'workload.txt', 'study.txt', 'waveform.txt', 'comparison.txt')},
                                compiler_files_sha256={'compiler/CMakeCXXCompiler.cmake': self.manifest['compiler_sha256']})
                    cells = [dict(rate=rate, buffer=buffer, paired_observations=16, blocks_per_path=4096,
                                  median_paired_p50_ratio=1.2 if comparison == 'candidate' else .99,
                                  median_paired_p99_ratio=1.3, reference_over_budget_blocks=0, study_over_budget_blocks=4096)
                             for rate in (48000, 96000, 192000) for buffer in (32, 64, 128)]
                    self.write(grid / 'metadata.json', meta)
                    self.write(grid / 'paired-results.json', dict(schema_version=1, metadata=meta, ratio_direction='study/reference', native_host_acceptance=False, paired_results=cells))
                    run = dict(round=index + 1, comparison=comparison, workload=workload, waveform=wave, folder=folder)
                    self.rehash(run)
                    self.manifest['runs'].append(run)
        self.save()

    def tearDown(self):
        self.temp.cleanup()

    def write(self, path, value):
        path.write_text(json.dumps(value) + '\n', encoding='utf-8')

    def save(self):
        self.write(self.root / 'manifest.json', self.manifest)

    def rehash(self, run):
        grid = self.root / run['folder']
        run['files_sha256'] = {p.relative_to(grid).as_posix(): reviewer.digest(p) for p in grid.rglob('*') if p.is_file()}

    def mutate_report(self, mutate):
        run = self.manifest['runs'][0]
        grid = self.root / run['folder']
        payload = reviewer.document(grid / 'paired-results.json')
        mutate(payload)
        self.write(grid / 'paired-results.json', payload)
        self.write(grid / 'metadata.json', payload['metadata'])
        self.rehash(run)
        self.save()

    def test_all_rows_preserved_no_acceptance_or_archive_writes(self):
        before = {str(p): reviewer.digest(p) for p in self.root.rglob('*') if p.is_file()}
        result = reviewer.review(self.root)
        self.assertEqual(len(result['rows']), 288)
        self.assertFalse(result['production_promotion_allowed'])
        text = reviewer.render(result)
        self.assertIn('1.200000', text)
        self.assertIn('4096', text)
        self.assertIn('reference-repeat', text)
        self.assertEqual(before, {str(p): reviewer.digest(p) for p in self.root.rglob('*') if p.is_file()})

    def test_modified_raw_data_rejected(self):
        (self.root / self.manifest['runs'][0]['folder'] / 'blocks.csv').write_bytes(b'tampered')
        with self.assertRaisesRegex(ValueError, 'changed evidence'): reviewer.review(self.root)

    def test_cli_success_and_failure_without_partial_report(self):
        args = [sys.executable, str(ROOT / 'scripts/review-dispatch-engine-target.py'), str(self.root)]
        good = subprocess.run(args, capture_output=True, text=True)
        self.assertEqual(good.returncode, 0, good.stderr)
        self.assertIn('Dispatch target archive review', good.stdout)
        (self.root / 'contracts.txt').write_bytes(b'changed qualification')
        bad = subprocess.run(args, capture_output=True, text=True)
        self.assertEqual(bad.returncode, 1)
        self.assertEqual(bad.stdout, '')
        self.assertIn('changed evidence', bad.stderr)

    def test_cli_raw_switch_rejects_non_csv_fixture_without_partial_report(self):
        args = [sys.executable, str(ROOT / 'scripts/review-dispatch-engine-target.py'), str(self.root), '--verify-raw']
        result = subprocess.run(args, capture_output=True, text=True)
        self.assertEqual(result.returncode, 1)
        self.assertEqual(result.stdout, '')
        self.assertIn('Archive review failed', result.stderr)

    def test_unrecorded_root_file_rejected(self):
        (self.root / 'unexpected.json').write_text('{}')
        with self.assertRaisesRegex(ValueError, 'Unrecorded archive'): reviewer.review(self.root)

    def test_incomplete_collection_rejected(self):
        self.manifest['state'] = 'failed'
        self.save()
        with self.assertRaisesRegex(ValueError, 'Incomplete collection'): reviewer.review(self.root)

    def test_missing_and_duplicate_campaigns_rejected(self):
        removed = self.manifest['runs'].pop()
        self.save()
        with self.assertRaisesRegex(ValueError, 'campaigns'): reviewer.review(self.root)
        self.manifest['runs'].append(self.manifest['runs'][0])
        self.save()
        with self.assertRaisesRegex(ValueError, 'identity/order'): reviewer.review(self.root)
        self.manifest['runs'][-1] = removed

    def test_source_and_control_identity_rejected_even_after_rehash(self):
        self.mutate_report(lambda p: p['metadata'].update(source_sha='c' * 40))
        with self.assertRaisesRegex(ValueError, 'identity'): reviewer.review(self.root)
        self.mutate_report(lambda p: p['metadata'].update(source_sha='a' * 40, study_variant='rate-lookup-saw-dispatch-v1'))
        with self.assertRaisesRegex(ValueError, 'variant'): reviewer.review(self.root)

    def test_invalid_and_incomplete_cells_rejected_even_after_rehash(self):
        self.mutate_report(lambda p: p['paired_results'][0].update(study_over_budget_blocks=4097))
        with self.assertRaisesRegex(ValueError, 'deadline count'): reviewer.review(self.root)
        self.mutate_report(lambda p: p['paired_results'][0].update(study_over_budget_blocks=0, median_paired_p99_ratio=float('nan')))
        with self.assertRaisesRegex(ValueError, 'Non-finite'): reviewer.review(self.root)
        self.mutate_report_cleanup()

    def mutate_report_cleanup(self):
        # Restore a valid JSON document before testing duplicate cells.
        run = self.manifest['runs'][0]
        path = self.root / run['folder'] / 'paired-results.json'
        payload = json.loads(path.read_text())
        payload['paired_results'][0]['median_paired_p99_ratio'] = 1.0
        payload['paired_results'][1] = payload['paired_results'][0]
        self.write(path, payload)
        self.rehash(run)
        self.save()
        with self.assertRaisesRegex(ValueError, 'report grid'): reviewer.review(self.root)

    def test_paths_unrecorded_files_and_symlinks_rejected(self):
        run = self.manifest['runs'][0]
        run['files_sha256']['../outside'] = 'b' * 64
        self.save()
        with self.assertRaises(ValueError): reviewer.review(self.root)
        del run['files_sha256']['../outside']
        self.save()
        extra = self.root / run['folder'] / 'extra.csv'
        extra.write_bytes(b'unrecorded')
        with self.assertRaisesRegex(ValueError, 'Unrecorded'): reviewer.review(self.root)
        extra.unlink()
        try:
            extra.symlink_to(self.root / 'contracts.txt')
        except (OSError, NotImplementedError):
            return  # Other negative controls still run on Windows without symlink privilege.
        with self.assertRaisesRegex(ValueError, 'symlinks'): reviewer.review(self.root)

    def test_duplicate_json_keys_rejected(self):
        (self.root / 'manifest.json').write_text('{"state":"complete","state":"failed"}')
        with self.assertRaisesRegex(ValueError, 'Duplicate JSON'): reviewer.review(self.root)

    def test_existing_csv_reporter_output_is_accepted(self):
        spec = importlib.util.spec_from_file_location('csv_fixture', ROOT / 'tests/premium_deadline_report.py')
        fixture = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(fixture)
        fixture.DeadlineReport.setUpClass()
        try:
            case = fixture.DeadlineReport()
            case.setUp()
            (case.root / 'comparison.txt').write_text('candidate')
            meta = case.run_report('rate-saw-dispatch', 2)
            cells = reviewer.document(case.root / 'paired-results.json')['paired_results']
            before = {str(p): reviewer.digest(p) for p in case.root.rglob('*') if p.is_file()}
            # Validation must not inspect the current checkout, run binaries or
            # rewrite evidence. Conflicting ambient labels do not override the archive.
            with patch.dict('os.environ', SAWSTAR_PREMIUM_STUDY='wrong'), \
                 patch('subprocess.check_output', side_effect=AssertionError('unexpected subprocess')):
                reviewer.verify_raw_grid(case.root, ('candidate', 'stationary-v1', 0), cells)
            self.assertEqual(before, {str(p): reviewer.digest(p) for p in case.root.rglob('*') if p.is_file()})
            cells[0]['median_paired_p99_ratio'] += .25
            with self.assertRaisesRegex(ValueError, 'paired results disagree'):
                reviewer.verify_raw_grid(case.root, ('candidate', 'stationary-v1', 0), cells)
            cells[0]['median_paired_p99_ratio'] -= .25
            raw = case.root / 'blocks.csv'
            original = raw.read_bytes()
            lines = original.splitlines(keepends=True)
            lines[2] = lines[1]
            raw.write_bytes(b''.join(lines))
            with self.assertRaisesRegex(ValueError, 'Duplicate raw block'):
                reviewer.verify_raw_grid(case.root, ('candidate', 'stationary-v1', 0), cells)
            raw.write_bytes(original)
            self.manifest['source_sha'] = meta['source_sha']
            self.manifest['source_file_sha256'] = meta['source_file_sha256']
            compiler = case.root / 'compiler/CMakeCXXCompiler.cmake'
            self.manifest['compiler_sha256'] = reviewer.digest(compiler)
            for run in self.manifest['runs']:
                grid = self.root / run['folder']
                payload = reviewer.document(grid / 'paired-results.json')
                payload['metadata'].update(source_sha=meta['source_sha'], source_file_sha256=meta['source_file_sha256'],
                                           compiler_files_sha256=meta['compiler_files_sha256'])
                shutil.copyfile(compiler, grid / 'compiler/CMakeCXXCompiler.cmake')
                # Full synthetic collection: every scene shares this fixture's
                # CSV bytes. This tests archive orchestration, not real timing.
                for name in ('summary.csv', 'blocks.csv', 'factors.csv', 'backend.txt'):
                    shutil.copyfile(case.root / name, grid / name)
                payload['paired_results'] = cells
                payload['metadata']['measurement_sha256'] = {
                    name: reviewer.digest(grid / name) for name in payload['metadata']['measurement_sha256']}
                self.write(grid / 'paired-results.json', payload)
                self.write(grid / 'metadata.json', payload['metadata'])
                if (run['round'], run['comparison'], run['workload'], run['waveform']) == (1, 'candidate', 'stationary-v1', 0):
                    for name in ('summary.csv', 'blocks.csv', 'factors.csv', 'backend.txt', 'workload.txt', 'study.txt',
                                 'waveform.txt', 'comparison.txt', 'metadata.json', 'paired-results.json', 'report.md'):
                        shutil.copyfile(case.root / name, grid / name)
                self.rehash(run)
            self.save()
            result = reviewer.review(self.root, verify_raw=True)
            self.assertTrue(result['raw_measurements_verified'])
            self.assertIn('percentiles, strict deadline counts', reviewer.render(result))
            rows = result['rows']
            cell = next(r for r in rows if (r['round'], r['comparison'], r['workload'], r['waveform'], r['rate'], r['buffer']) ==
                        (1, 'candidate', 'stationary-v1', 0, 192000, 32))
            self.assertEqual(cell['median_paired_p50_ratio'], 3.5)
            self.assertEqual(cell['study_over_budget_blocks'], 1024)
        finally:
            fixture.DeadlineReport.tearDownClass()


if __name__ == '__main__':
    unittest.main()
