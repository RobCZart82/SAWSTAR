#!/usr/bin/env python3
"""One fixed Sine investigation, preserving both binaries and every outcome."""
import argparse
import csv
import hashlib
import json
import platform
import shutil
import subprocess
from pathlib import Path


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def investigate(baseline_build, trace_build, output, architectures):
    output.mkdir(parents=True, exist_ok=False)
    manifest = {
        'schema': 1, 'state': 'incomplete',
        'purpose': 'First Sine divergence localization',
        'source_commit': subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True).strip(),
        'host_architecture': platform.machine(),
        'os': subprocess.check_output(['sw_vers'], text=True),
        'compiler': subprocess.check_output(['c++', '--version'], text=True),
        'source_files_sha256': {}, 'files_sha256': {}, 'results': [],
        'policy': 'One process per mode/architecture; retain failures; no FP override or numerical tolerance',
    }
    for name in ('tests/oscillator_dispatch_engine.cpp', 'tests/DispatchEngineSetup.h',
                 'tests/NumericalComparison.h', 'tests/FpObservation.h',
                 'src/engine/Synth.cpp', 'src/engine/Synth.h',
                 'cmake/PremiumPreview.cmake', 'scripts/investigate-k1-sine.py'):
        manifest['source_files_sha256'][name] = digest(Path(name))

    def save():
        (output / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')

    save()
    try:
        for role, build in (('baseline', baseline_build), ('trace', trace_build)):
            destination = output / role
            destination.mkdir()
            for name in ('sawstar_premium_native_pair', 'CMakeCache.txt', 'compile_commands.json'):
                source = build / name
                shutil.copy2(source, destination / name)
            shutil.copytree(build / 'premium_preview_engine', destination / 'generated-engine')
            # Includes the exact generated sources compiled for each engine.
            for path in destination.rglob('*'):
                if path.is_file():
                    manifest['files_sha256'][path.relative_to(output).as_posix()] = digest(path)
        save()
        for architecture in architectures:
            cases = (
                ('baseline-self-test', 'baseline', ['--diagnostic-self-test']),
                ('trace-self-test', 'trace', ['--diagnostic-self-test']),
                ('original-sine-grid', 'baseline', ['--waveform', '3']),
                ('passive-sine-grid', 'baseline', ['--waveform', '3', '--fp-observe']),
                ('internal-sine-grid', 'trace', ['--waveform', '3', '--fp-observe']),
            )
            for name, role, arguments in cases:
                executable = (output / role / 'sawstar_premium_native_pair').resolve()
                command = ['arch', '-' + architecture, str(executable), *arguments]
                stdout = output / f'{architecture}-{name}.csv'
                stderr = output / f'{architecture}-{name}.txt'
                with stdout.open('w') as out, stderr.open('w') as err:
                    try:
                        result = subprocess.run(command, stdout=out, stderr=err, timeout=180)
                        code = result.returncode
                    except subprocess.TimeoutExpired:
                        err.write('\nInvestigation process timeout\n')
                        code = 124
                entry = dict(architecture=architecture, name=name, command=command,
                             returncode=code, stdout=stdout.name, stderr=stderr.name)
                if name == 'trace-self-test' and 'trace=on' not in stdout.read_text():
                    entry['validation_error'] = 'Instrumented diagnostic self-test did not confirm trace=on'
                    entry['returncode'] = code or 1
                if name.endswith('sine-grid') and code == 0:
                    try:
                        with stdout.open() as stream:
                            rows = list(csv.reader(stream))
                        expected = {(3, rate, mode, voice, fx)
                                    for rate in (48000, 96000, 176400, 192000, 384000)
                                    for mode in range(4) for voice in range(3) for fx in range(2)}
                        observed = {tuple(int(float(value)) for value in row[:5]) for row in rows[1:]}
                        if len(rows) != 121 or observed != expected or any(
                                len(row) != 9 or any(float(value) != 0 for value in row[5:])
                                for row in rows[1:]):
                            raise ValueError('Expected 120 unique, bit-identical Sine cases')
                        if role == 'trace' or name == 'passive-sine-grid':
                            if 'state=complete cases=120' not in stderr.read_text():
                                raise ValueError('Passive FP observation did not complete')
                    except (ValueError, IndexError) as error:
                        entry['validation_error'] = str(error)
                        entry['returncode'] = 1
                manifest['files_sha256'][stdout.name] = digest(stdout)
                manifest['files_sha256'][stderr.name] = digest(stderr)
                manifest['results'].append(entry)
                save()
        manifest['state'] = 'negative' if any(r['returncode'] for r in manifest['results']) else 'complete'
        save()
    except Exception as error:
        manifest['error'] = str(error)
        save()
        raise
    return 1 if manifest['state'] == 'negative' else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline-build', type=Path, required=True)
    parser.add_argument('--trace-build', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--architectures', nargs='+', choices=('arm64', 'x86_64'), required=True)
    args = parser.parse_args()
    raise SystemExit(investigate(args.baseline_build, args.trace_build, args.output, args.architectures))


if __name__ == '__main__':
    main()
