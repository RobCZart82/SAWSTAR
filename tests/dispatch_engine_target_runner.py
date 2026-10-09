# SPDX-License-Identifier: MIT
"""Orchestration/failure tests; CSV validation is exercised by premium_deadline_report."""
import contextlib
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import Mock, patch

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('dispatch_target', ROOT / 'scripts/run-dispatch-engine-target.py')
runner = importlib.util.module_from_spec(spec)
spec.loader.exec_module(runner)


class DispatchTargetTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.folder = Path(self.temp.name)
        self.build, self.output = self.folder / 'build', self.folder / 'results'
        self.sha = 'a' * 40
        self.measured = []
        self.commands = []
        self.failure = None
        self.multi_config = False

    def tearDown(self):
        self.temp.cleanup()

    def fake_run(self, args, **kwargs):
        self.commands.append(args)
        if '-S' in args:
            compiler = self.build / 'CMakeFiles/1/CMakeCXXCompiler.cmake'
            compiler.parent.mkdir(parents=True)
            compiler.write_bytes(b'compiler evidence\r\n')
        elif '--build' in args:
            binary_folder = self.build / 'Release' if self.multi_config else self.build
            binary_folder.mkdir(exist_ok=True)
            for name in ('sawstar_dispatch_engine_deadline', 'sawstar_premium_lookup_complex_tests'):
                (binary_folder / (name + ('.exe' if sys.platform == 'win32' else ''))).write_bytes(b'fixed executable')
        elif '--no-tests=error' in args or '--modulation-contract' in args or Path(args[0]).stem == 'sawstar_premium_lookup_complex_tests':
            if (self.failure == 'qualification' or
                self.failure == 'complex' and Path(args[0]).stem == 'sawstar_premium_lookup_complex_tests' or
                self.failure == 'wave-contract' and '--modulation-contract' in args):
                raise subprocess.CalledProcessError(1, args)
        elif '--deadline' in args or '--deadline-modulated' in args:
            folder = Path(args[2])
            comparison = 'reference-repeat' if args[-1] == '--reference-repeat' else 'candidate'
            workload = 'modulated-v1' if '--deadline-modulated' in args else 'stationary-v1'
            wave = int(args[args.index('--waveform') + 1])
            self.measured.append((comparison, workload, wave))
            (folder / 'comparison.txt').write_text('candidate' if self.failure == 'comparison' else comparison)
            (folder / 'summary.csv').write_bytes(b'synthetic measurement\r\n')
            if self.failure == 'timing':
                raise subprocess.CalledProcessError(1, args)
            if self.failure == 'binary':
                Path(args[0]).write_bytes(b'changed executable')
            if self.failure == 'compiler':
                (self.build / 'CMakeFiles/1/CMakeCXXCompiler.cmake').write_bytes(b'changed compiler')
        elif 'report-premium-deadline.py' in args[1]:
            if self.failure == 'reporter':
                raise subprocess.CalledProcessError(1, args)
            folder, env = Path(args[2]), kwargs['env']
            self.assertEqual(env['SAWSTAR_PREMIUM_STUDY'], 'rate-saw-dispatch')
            meta = dict(source_sha='b' * 40 if self.failure == 'identity' else self.sha,
                        comparison=env['SAWSTAR_PREMIUM_COMPARISON'],
                        workload=env['SAWSTAR_PREMIUM_WORKLOAD'],
                        oscillator_waveform=int(env['SAWSTAR_OSC_WAVEFORM']),
                        production_activation=False, source_file_sha256={'source.cpp': 'c' * 64})
            if self.failure == 'source-bytes' and len(self.measured) > 1:
                meta['source_file_sha256']['source.cpp'] = 'd' * 64
            (folder / 'paired-results.json').write_text(json.dumps(dict(metadata=meta, native_host_acceptance=False)))
            if self.failure == 'prior-evidence' and len(self.measured) > 1:
                first = self.output / 'round-01/reference-repeat/stationary-v1/waveform-0/summary.csv'
                first.write_bytes(b'altered previous evidence')
        kwargs['stdout'].write(b'command log\n')

    def collect(self):
        with patch.object(runner.common, 'source_identity', return_value=self.sha), \
             patch.object(runner, 'subprocess', SimpleNamespace(run=Mock(side_effect=self.fake_run), STDOUT=subprocess.STDOUT)), \
             contextlib.redirect_stdout(io.StringIO()):
            return runner.collect(self.build, self.output)

    def test_two_rounds_reverse_comparison_and_scene_order(self):
        result = self.collect()
        scenes = [(workload, wave) for workload in ('stationary-v1', 'modulated-v1') for wave in range(4)]
        expected = [(comparison, workload, wave) for comparison in ('reference-repeat', 'candidate') for workload, wave in scenes]
        expected += [(comparison, workload, wave) for comparison in ('candidate', 'reference-repeat') for workload, wave in reversed(scenes)]
        self.assertEqual(self.measured, expected)
        self.assertEqual(len(result['runs']), 32)
        self.assertEqual(result['state'], 'complete')
        self.assertFalse(result['native_host_acceptance'])
        self.assertFalse(result['production_promotion_allowed'])
        self.assertFalse(result['background_load_controlled'])
        self.assertFalse(result['affinity_applied'])
        first_timing = next(i for i, c in enumerate(self.commands) if '--deadline' in c)
        before = self.commands[:first_timing]
        self.assertEqual(sum('--modulation-contract' in c for c in before), 4)
        self.assertTrue(any('--no-tests=error' in c for c in before))
        for run in result['runs']:
            for name, sha in run['files_sha256'].items():
                self.assertEqual(hashlib.sha256((self.output / run['folder'] / name).read_bytes()).hexdigest(), sha)
        self.assertEqual(json.loads((self.output / 'manifest.json').read_text()), result)

    def test_failed_qualification_prevents_all_timing(self):
        for phase in ('qualification', 'complex', 'wave-contract'):
            with self.subTest(phase=phase):
                self.build, self.output = self.folder / ('build-' + phase), self.folder / ('results-' + phase)
                self.failure = phase
                with self.assertRaises(subprocess.CalledProcessError): self.collect()
                self.assertEqual(self.measured, [])
                self.assertEqual(json.loads((self.output / 'manifest.json').read_text())['state'], 'failed')

    def test_multi_config_release_executable_location(self):
        self.multi_config = True
        self.assertEqual(self.collect()['state'], 'complete')
        timing = next(c for c in self.commands if '--deadline' in c)
        self.assertEqual(Path(timing[0]).parent, self.build / 'Release')

    def test_failures_preserve_raw_data_and_never_complete(self):
        for failure, error in [('comparison', ValueError), ('timing', subprocess.CalledProcessError),
                               ('binary', ValueError), ('compiler', ValueError), ('reporter', subprocess.CalledProcessError),
                               ('identity', ValueError), ('source-bytes', ValueError), ('prior-evidence', ValueError)]:
            with self.subTest(failure=failure):
                self.build, self.output = self.folder / ('build-' + failure), self.folder / ('results-' + failure)
                self.failure, self.measured = failure, []
                with self.assertRaises(error): self.collect()
                self.assertEqual(json.loads((self.output / 'manifest.json').read_text())['state'], 'failed')
                self.assertTrue(list(self.output.rglob('summary.csv')))

    def test_rejects_unsafe_existing_paths_and_invalid_rounds(self):
        self.output.mkdir()
        (self.output / 'keep.txt').write_bytes(b'previous result')
        with self.assertRaisesRegex(ValueError, 'fresh'): self.collect()
        self.assertEqual((self.output / 'keep.txt').read_bytes(), b'previous result')
        for build, output, rounds, message in [(ROOT / 'build', self.folder / 'x', 2, 'outside'),
                                               (self.folder / 'x', self.folder / 'x/y', 2, 'separate'),
                                               (self.build, self.folder / 'x', 1, '2 to 10'),
                                               (self.build, self.folder / 'x', 2.5, '2 to 10')]:
            with self.assertRaisesRegex(ValueError, message): runner.collect(build, output, rounds)

    def test_dirty_source_does_not_create_output(self):
        with patch.object(runner.common, 'source_identity', side_effect=ValueError('dirty source')):
            with self.assertRaisesRegex(ValueError, 'dirty'): runner.collect(self.build, self.output)
        self.assertFalse(self.build.exists())
        self.assertFalse(self.output.exists())


if __name__ == '__main__':
    unittest.main()
