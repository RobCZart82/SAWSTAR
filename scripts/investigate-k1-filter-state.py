#!/usr/bin/env python3
"""One fixed Rosetta Sine control with named FIR/filter-state history."""
import argparse
import importlib.util
import json
import platform
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('k1_sine', ROOT / 'scripts/investigate-k1-sine.py')
sine = importlib.util.module_from_spec(spec)
spec.loader.exec_module(sine)


def cache_value(build, key):
    lines = (build / 'CMakeCache.txt').read_text().splitlines()
    matches = [line.split('=', 1)[1] for line in lines if line.startswith(key + ':')]
    if len(matches) != 1:
        raise ValueError('Missing/ambiguous cache setting: ' + key)
    return matches[0]


def investigate(baseline, trace, output):
    for build in (baseline, trace):
        if cache_value(build, 'SAWSTAR_BUILD_PLUGIN') != 'OFF':
            raise ValueError('Investigation must be offline-only')
    for key, expected in (('SAWSTAR_NUMERICAL_TRACE', 'OFF'), ('SAWSTAR_FILTER_STATE_TRACE', 'OFF')):
        if cache_value(baseline, key) != expected:
            raise ValueError('Baseline must remain uninstrumented: ' + key)
    for key in ('SAWSTAR_NUMERICAL_TRACE', 'SAWSTAR_FILTER_STATE_TRACE'):
        if cache_value(trace, key) != 'ON':
            raise ValueError('Named filter-state instrumentation is required: ' + key)
    if platform.machine() != 'arm64':
        raise ValueError('This bounded control requires an Apple Silicon host')
    translation = subprocess.check_output(
        ['arch', '-x86_64', '/usr/sbin/sysctl', '-n', 'sysctl.proc_translated'], text=True).strip()
    if translation != '1':
        raise ValueError('Rosetta translation probe did not confirm translation')
    extra_sources = (
        'src/engine/FilterStateTrace.h', 'src/engine/NumericalTrace.h',
        'tests/FilterStateComparison.h', 'tests/oscillator_dispatch_engine.cpp',
        'experiments/premium_filter/EnginePremiumFilter.h',
        'experiments/premium_filter/PremiumDrive.h', 'experiments/premium_filter/FirLanes4.h',
        'experiments/premium_filter/RateScaledPremiumDrive.h',
        'experiments/premium_filter/PremiumLowPass.h',
        'scripts/investigate-k1-filter-state.py', 'CMakeLists.txt', 'cmake/PremiumPreview.cmake',
    )
    source_hashes = {name: sine.digest(ROOT / name) for name in extra_sources}
    source_status = subprocess.check_output(['git', 'status', '--porcelain'], cwd=ROOT, text=True)
    # Reuse the established 120-case inventory, timeout, negative retention and
    # no-overwrite rules. Exactly five processes, all Rosetta x86_64, no retry.
    code = sine.investigate(baseline, trace, output, ['x86_64'])
    path = output / 'manifest.json'
    manifest = json.loads(path.read_text())
    manifest['purpose'] = 'Named filter/FIR first-divergence localization, not root-cause proof'
    manifest['history'] = {'frames': 6, 'preceding_samples': 5, 'raw_object_memory': False}
    manifest['execution_scope'] = 'x86_64 control on Apple Silicon; normal/passive/deep Sine grids once each'
    manifest['rosetta_translation_probe'] = translation
    manifest['source_dirty'] = bool(source_status.strip())
    manifest['source_status'] = source_status
    manifest['source_files_sha256'].update(source_hashes)
    if any(sine.digest(ROOT / name) != digest for name, digest in source_hashes.items()):
        manifest['source_validation_error'] = 'Source files changed during the fixed control'
        manifest['state'] = 'negative'
        code = 1
    marker = 'filter_state=on history=6'
    if marker not in (output / 'x86_64-trace-self-test.csv').read_text():
        manifest['filter_state_validation_error'] = 'Missing deep-state and history self-test confirmation'
        manifest['state'] = 'negative'
        code = 1
    path.write_text(json.dumps(manifest, indent=2) + '\n')
    return code


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline-build', type=Path, required=True)
    parser.add_argument('--trace-build', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    raise SystemExit(investigate(args.baseline_build, args.trace_build, args.output))
