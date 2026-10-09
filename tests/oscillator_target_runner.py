# SPDX-License-Identifier: MIT
import csv
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('target_runner', ROOT / 'scripts/run-oscillator-target.py')
runner = importlib.util.module_from_spec(spec)
spec.loader.exec_module(runner)

class TargetRunnerTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.folder = Path(self.temp.name)
        self.build = self.folder / 'build'
        self.output = self.folder / 'results'
        self.measured = []
        self.fail_contract = False
        self.invalid_csv = False
        self.mutate_executable = False

    def tearDown(self):
        self.temp.cleanup()

    def fake_run(self, args, **kwargs):
        if '--extended' in args:
            comparison = 'reference-repeat' if '--reference-repeat' in args else 'candidate'
            self.measured.append(comparison)
            stream = io.StringIO(newline='')
            writer = csv.writer(stream)
            writer.writerow(['path', 'rate', 'waveform', 'modulated', 'pair', 'order', 'oscillators',
                             'frames', 'seconds', 'energy', 'variant', 'campaign', 'comparison', 'warmup_frames'])
            for rate in (48000, 96000, 192000):
                for wave in range(4):
                    for moving in range(2):
                        for pair in range(8):
                            for path in ('reference', 'study'):
                                writer.writerow([path, rate, wave, moving, pair,
                                                 'study-first' if pair % 2 else 'reference-first',
                                                 32, 1 if self.invalid_csv else 131072,
                                                 1, 10, 'shared-frequency-v1', 'extended-v1', comparison, rate // 4])
            kwargs['stdout'].write(stream.getvalue().encode('utf-8'))
            if self.mutate_executable:
                Path(args[0]).write_bytes(b'changed binary')
            return
        if '--no-tests=error' in args and self.fail_contract:
            raise subprocess.CalledProcessError(1, args)
        if '-S' in args:
            compiler = self.build / 'CMakeFiles/1/CMakeCXXCompiler.cmake'
            compiler.parent.mkdir(parents=True)
            compiler.write_bytes(b'compiler evidence\r\n')
        if '--build' in args:
            name = 'sawstar_oscillator_shared_benchmark' + ('.exe' if sys.platform == 'win32' else '')
            (self.build / name).write_bytes(b'fixed executable')
        kwargs['stdout'].write(b'qualification/build log\n')

    def collect(self):
        with patch.object(runner, 'source_identity', return_value='a' * 40), \
             patch.object(runner.platform, 'platform', return_value='test-platform'), \
             patch.object(runner.platform, 'machine', return_value='test-machine'), \
             patch.object(runner.platform, 'processor', return_value='test-processor'), \
             patch.object(runner.subprocess, 'run', side_effect=self.fake_run):
            return runner.collect(self.build, self.output)

    def test_complete_campaign_order_bytes_and_no_acceptance(self):
        result = self.collect()
        self.assertEqual(self.measured, ['reference-repeat', 'candidate', 'candidate', 'reference-repeat'])
        self.assertEqual(result['state'], 'complete')
        self.assertEqual(len(result['runs']), 4)
        self.assertFalse(result['production_promotion_allowed'])
        self.assertFalse(result['background_load_controlled'])
        self.assertFalse(result['native_host_acceptance'])
        for run in result['runs']:
            folder = self.output / run['folder']
            for name, sha in run['files_sha256'].items():
                self.assertEqual(hashlib.sha256((folder / name).read_bytes()).hexdigest(), sha)
            report = json.loads((folder / 'report.json').read_bytes())
            self.assertEqual(len(report['summary']), 24)
            self.assertEqual(report['metadata']['comparison'], run['comparison'])
            self.assertEqual(report['metadata']['source_sha'], result['source_sha'])
            self.assertEqual(report['metadata']['raw_sha256'], run['files_sha256']['oscillators.csv'])
            self.assertFalse(report['qualification']['production_promotion_allowed'])
        self.assertEqual(json.loads((self.output / 'manifest.json').read_bytes()), result)

    def test_failed_contract_prevents_any_timing(self):
        self.fail_contract = True
        with self.assertRaises(subprocess.CalledProcessError):
            self.collect()
        self.assertEqual(self.measured, [])
        self.assertEqual(json.loads((self.output / 'manifest.json').read_bytes())['state'], 'failed')

    def test_incomplete_grid_is_preserved_and_rejected(self):
        self.invalid_csv = True
        with self.assertRaisesRegex(ValueError, 'frame count'):
            self.collect()
        manifest = json.loads((self.output / 'manifest.json').read_bytes())
        self.assertEqual(manifest['state'], 'failed')
        self.assertEqual(manifest['runs'], [])
        self.assertTrue((self.output / 'round-01/reference-repeat/oscillators.csv').is_file())

    def test_executable_change_stops_collection(self):
        self.mutate_executable = True
        with self.assertRaisesRegex(ValueError, 'executable changed'):
            self.collect()
        self.assertEqual(len(self.measured), 1)
        self.assertEqual(json.loads((self.output / 'manifest.json').read_bytes())['state'], 'failed')

    def test_existing_result_and_unsafe_paths_are_rejected(self):
        self.output.mkdir()
        (self.output / 'keep.txt').write_bytes(b'prior result')
        with self.assertRaisesRegex(ValueError, 'fresh'):
            self.collect()
        self.assertEqual((self.output / 'keep.txt').read_bytes(), b'prior result')
        for build, output, message in [(ROOT / 'build', self.folder / 'other', 'outside'),
                                       (self.folder / 'x', self.folder / 'x/y', 'separate')]:
            with self.assertRaisesRegex(ValueError, message):
                runner.collect(build, output)
        with self.assertRaisesRegex(ValueError, '2 to 10'):
            runner.collect(self.build, self.folder / 'other', rounds=1)

    def test_source_requires_clean_pinned_daisy(self):
        with patch.object(runner.subprocess, 'check_output', side_effect=[
            'a' * 40, '', '160000 commit ' + 'b' * 40 + '\tthird_party/DaisySP', 'b' * 40, '']):
            self.assertEqual(runner.source_identity(), 'a' * 40)
        for values in [['a' * 40, ' M src/dsp/SevenSaw.cpp'],
                       ['a' * 40, '', '160000 commit ' + 'b' * 40 + '\tthird_party/DaisySP', 'c' * 40],
                       ['a' * 40, '', '160000 commit ' + 'b' * 40 + '\tthird_party/DaisySP', 'b' * 40, ' M Source/file']]:
            with patch.object(runner.subprocess, 'check_output', side_effect=values):
                with self.assertRaises(ValueError):
                    runner.source_identity()

if __name__ == '__main__':
    unittest.main()
