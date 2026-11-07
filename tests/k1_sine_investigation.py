#!/usr/bin/env python3
"""Check that investigation failures keep all later controls and exact evidence."""
import hashlib
import importlib.util
import json
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('investigation', ROOT / 'scripts/investigate-k1-sine.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class InvestigationEvidence(unittest.TestCase):
    def run_case(self, timeout=False):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            builds = [root / 'baseline', root / 'trace']
            for build in builds:
                build.mkdir()
                for name in ('sawstar_premium_native_pair', 'CMakeCache.txt', 'compile_commands.json'):
                    (build / name).write_text(build.name + '-' + name)
                (build / 'premium_preview_engine').mkdir()
                (build / 'premium_preview_engine' / 'Synth.cpp').write_text('exact generated source')
            commands = []

            def run(command, stdout, stderr, timeout):
                commands.append(command)
                if len(commands) == 3:
                    stderr.write('Numerical mismatch retained\n')
                    if timeout_test:
                        raise subprocess.TimeoutExpired(command, timeout)
                    return subprocess.CompletedProcess(command, 1)
                if '--diagnostic-self-test' in command:
                    stdout.write('Numerical failure diagnostics self-test passed trace=on\n')
                else:
                    stdout.write('header\n')
                    for rate in (48000, 96000, 176400, 192000, 384000):
                        for mode in range(4):
                            for voice in range(3):
                                for fx in range(2):
                                    stdout.write(f'3,{rate},{mode},{voice},{fx},0,0,0,0\n')
                    stderr.write('state=complete cases=120\n')
                return subprocess.CompletedProcess(command, 0)

            output = root / 'evidence'
            timeout_test = timeout
            with patch.object(module.subprocess, 'check_output', return_value='fixture\n'), patch.object(
                    module.subprocess, 'run', side_effect=run):
                code = module.investigate(*builds, output, ['x86_64'])
            self.assertEqual(code, 1)
            manifest = json.loads((output / 'manifest.json').read_text())
            self.assertEqual(manifest['state'], 'negative')
            self.assertEqual(len(commands), 5)
            self.assertEqual([r['returncode'] for r in manifest['results']],
                             [0, 0, 124 if timeout else 1, 0, 0])
            for name, digest in manifest['files_sha256'].items():
                self.assertEqual(hashlib.sha256((output / name).read_bytes()).hexdigest(), digest)
            self.assertIn('baseline/sawstar_premium_native_pair', manifest['files_sha256'])
            self.assertIn('trace/sawstar_premium_native_pair', manifest['files_sha256'])
            self.assertIn('Numerical mismatch retained',
                          (output / 'x86_64-original-sine-grid.txt').read_text())
            with self.assertRaises(FileExistsError):
                module.investigate(*builds, output, ['x86_64'])

    def test_negative_keeps_following_controls_and_binary_hashes(self):
        self.run_case()

    def test_timeout_keeps_following_controls_and_timeout_record(self):
        self.run_case(timeout=True)


if __name__ == '__main__':
    unittest.main()
