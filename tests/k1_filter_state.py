#!/usr/bin/env python3
"""Bounded deep-filter campaign must not accept a shallow or altered baseline."""
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('deep', ROOT / 'scripts/investigate-k1-filter-state.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class DeepEvidence(unittest.TestCase):
    def fixture(self, marker=True, code=0, bad_baseline=False, drift=False, translation="1"):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            builds = [root / 'baseline', root / 'trace']
            for i, build in enumerate(builds):
                build.mkdir()
                mode = 'ON' if i or bad_baseline else 'OFF'
                (build / 'CMakeCache.txt').write_text(
                    'SAWSTAR_BUILD_PLUGIN:BOOL=OFF\n'
                    f'SAWSTAR_NUMERICAL_TRACE:BOOL={mode}\nSAWSTAR_FILTER_STATE_TRACE:BOOL={mode}\n')

            def run(baseline, trace, output, architectures):
                self.assertEqual(architectures, ['x86_64'])
                output.mkdir()
                (output / 'manifest.json').write_text(json.dumps({
                    'state': 'negative' if code else 'complete', 'source_files_sha256': {},
                    'results': [{'returncode': code}], 'files_sha256': {},
                }))
                (output / 'x86_64-trace-self-test.csv').write_text(
                    'trace=on filter_state=on history=6' if marker else 'trace=on')
                return code

            original_digest=module.sine.digest
            calls={}
            def digest(path):
                calls[path]=calls.get(path,0)+1
                if drift and path.name=='FirLanes4.h' and calls[path]>1:
                    return 'changed-source'
                return original_digest(path)

            with patch.object(module.sine, 'digest', side_effect=digest), patch.object(module.sine, 'investigate', side_effect=run) as control, patch.object(
                    module.platform, 'machine', return_value='arm64'), patch.object(
                    module.subprocess, 'check_output', return_value=translation+'\n'):
                output = root / 'result'
                if bad_baseline or translation != '1':
                    with self.assertRaises(ValueError):
                        module.investigate(*builds, output)
                    control.assert_not_called()
                    return
                actual = module.investigate(*builds, output)
            manifest = json.loads((output / 'manifest.json').read_text())
            self.assertEqual(actual, code if marker and not drift else 1)
            self.assertEqual(manifest['state'], 'negative' if code or not marker or drift else 'complete')
            self.assertEqual(manifest['history']['preceding_samples'], 5)
            self.assertEqual(manifest['rosetta_translation_probe'], '1')
            self.assertIn('experiments/premium_filter/FirLanes4.h', manifest['source_files_sha256'])
            self.assertEqual(manifest['results'][0]['returncode'], code)

    def test_complete_deep_control(self):
        self.fixture()

    def test_shallow_trace_is_not_deep_pass(self):
        self.fixture(marker=False)

    def test_negative_remains_negative(self):
        self.fixture(code=1)

    def test_source_drift_cannot_be_accepted_as_verified_control(self):
        self.fixture(drift=True)

    def test_unconfirmed_rosetta_rejected_before_execution(self):
        self.fixture(translation='0')

    def test_instrumented_baseline_rejected_before_execution(self):
        self.fixture(bad_baseline=True)


if __name__ == '__main__':
    unittest.main()
