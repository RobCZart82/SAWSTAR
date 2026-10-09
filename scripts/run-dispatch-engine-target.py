#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Collect source-identical full-engine deadline repeats; no CPU acceptance."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('oscillator_target', ROOT / 'scripts/run-oscillator-target.py')
common = importlib.util.module_from_spec(spec)
spec.loader.exec_module(common)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def collect(build, output, rounds=2, cmake='cmake', ctest='ctest'):
    build, output = Path(build).resolve(), Path(output).resolve()
    if not isinstance(rounds, int) or not 2 <= rounds <= 10:
        raise ValueError('Use 2 to 10 complete rounds')
    if any(path == ROOT or ROOT in path.parents for path in (build, output)):
        raise ValueError('Build and output directories must be outside the checkout')
    if build == output or build in output.parents or output in build.parents:
        raise ValueError('Build and output directories must be separate')
    if build.exists() or output.exists():
        raise ValueError('Use fresh build and output directories; existing results are never overwritten')
    sha = common.source_identity()
    build.mkdir(parents=True)
    output.mkdir(parents=True)
    manifest = {
        'schema_version': 1, 'source_sha': sha, 'state': 'incomplete',
        'started_utc': datetime.now(timezone.utc).isoformat(),
        'platform': platform.platform(), 'machine': platform.machine(),
        'processor': platform.processor(), 'python': platform.python_version(),
        'rounds_requested': rounds, 'configuration': 'Release',
        'study_kind': 'rate-saw-dispatch', 'candidate_variant': 'saw-dispatch-v1',
        'background_load_controlled': False, 'affinity_applied': False,
        'native_host_acceptance': False, 'production_promotion_allowed': False,
        'commands': [], 'runs': [],
    }

    def save():
        (output / 'manifest.json').write_text(json.dumps(manifest, indent=2, allow_nan=False) + '\n', encoding='utf-8')

    def command(args, log, env=None):
        manifest['commands'].append([str(x) for x in args])
        save()
        with log.open('wb') as stream:
            subprocess.run([str(x) for x in args], cwd=ROOT, env=env,
                           stdout=stream, stderr=subprocess.STDOUT, check=True)

    save()
    try:
        command([cmake, '-S', ROOT, '-B', build, '-DCMAKE_BUILD_TYPE=Release',
                 '-DBUILD_TESTING=ON', '-DSAWSTAR_CHECK_DAISYSP=ON'], output / 'configure.txt')
        targets = ('sawstar_dispatch_engine_tests', 'sawstar_dispatch_engine_deadline',
                   'sawstar_oscillator_dispatch_tests', 'sawstar_premium_lookup_complex_tests')
        command([cmake, '--build', build, '--config', 'Release', '--parallel', '2',
                 '--target', *targets], output / 'build.txt')
        command([ctest, '--test-dir', build, '-C', 'Release', '--verbose', '--timeout', '600',
                 '--no-tests=error', '-R', '^(oscillator_dispatch_(bit_identity|engine_identity|engine_deadline_fixture|engine_reference_contract)|premium_deadline_report|dispatch_engine_target_runner)$'],
                output / 'contracts.txt')

        def executable(name):
            name += '.exe' if sys.platform == 'win32' else ''
            paths = [p for p in (build / 'Release' / name, build / name) if p.is_file()]
            if len(paths) != 1:
                raise ValueError('Expected exactly one freshly built Release executable: ' + name)
            return paths[0]

        probe = executable('sawstar_dispatch_engine_deadline')
        complex_probe = executable('sawstar_premium_lookup_complex_tests')
        compiler_files = sorted((build / 'CMakeFiles').rglob('CMakeCXXCompiler.cmake'))
        if len(compiler_files) != 1:
            raise ValueError('Expected one compiler evidence file')
        compiler = compiler_files[0]
        stable_files = [probe, complex_probe, compiler, Path(__file__),
                        ROOT / 'scripts/run-oscillator-target.py', ROOT / 'scripts/report-premium-deadline.py']
        stable_hashes = {str(p): digest(p) for p in stable_files}
        manifest['executable_sha256'] = {p.name: digest(p) for p in (probe, complex_probe)}
        manifest['tool_sha256'] = {p.name: digest(p) for p in stable_files[3:]}
        manifest['compiler_sha256'] = digest(compiler)

        def unchanged():
            if common.source_identity() != sha or any(digest(p) != stable_hashes[str(p)] for p in stable_files):
                raise ValueError('Source, executable, compiler or tool changed during collection')

        unchanged()
        command([complex_probe], output / 'complex-controls.txt')
        # Qualify every waveform timeline before any timed campaign, even when
        # the first campaign uses the unchanged reference on both paths.
        for wave in range(4):
            command([probe, '--modulation-contract', '--waveform', wave], output / f'waveform-{wave}-contracts.txt')
        qualification = ['configure.txt', 'build.txt', 'contracts.txt', 'complex-controls.txt',
                         *[f'waveform-{wave}-contracts.txt' for wave in range(4)]]
        manifest['build_evidence_sha256'] = {name: digest(output / name) for name in qualification}
        evidence = None
        scenes = [(workload, wave) for workload in ('stationary-v1', 'modulated-v1') for wave in range(4)]
        for round_index in range(rounds):
            comparisons = ['reference-repeat', 'candidate']
            order = scenes
            if round_index % 2:
                comparisons = list(reversed(comparisons))
                order = list(reversed(scenes))
            for comparison in comparisons:
                for workload, wave in order:
                    unchanged()
                    folder = output / f'round-{round_index + 1:02d}' / comparison / workload / f'waveform-{wave}'
                    (folder / 'compiler').mkdir(parents=True)
                    shutil.copyfile(compiler, folder / 'compiler/CMakeCXXCompiler.cmake')
                    args = [probe, '--deadline-modulated' if workload == 'modulated-v1' else '--deadline',
                            folder, '--waveform', wave]
                    if comparison == 'reference-repeat':
                        args.append('--reference-repeat')
                    print(f'Round {round_index + 1}/{rounds}: {comparison}, {workload}, waveform {wave}', flush=True)
                    command(args, folder / 'measurement.txt')
                    unchanged()
                    if (folder / 'comparison.txt').read_text(encoding='utf-8').strip() != comparison:
                        raise ValueError('Compiled comparison disagrees with target campaign')
                    env = dict(os.environ, SAWSTAR_PREMIUM_STUDY='rate-saw-dispatch',
                               SAWSTAR_PREMIUM_WORKLOAD=workload, SAWSTAR_OSC_WAVEFORM=str(wave),
                               SAWSTAR_PREMIUM_COMPARISON=comparison)
                    command([sys.executable, ROOT / 'scripts/report-premium-deadline.py', folder],
                            folder / 'reporter.txt', env=env)
                    result = json.loads((folder / 'paired-results.json').read_text(encoding='utf-8'))
                    meta = result['metadata']
                    if (meta['source_sha'], meta['comparison'], meta['workload'], meta['oscillator_waveform']) != (sha, comparison, workload, wave):
                        raise ValueError('Reporter identity disagrees with target campaign')
                    if result['native_host_acceptance'] or meta['production_activation']:
                        raise ValueError('Reporter cannot grant native or production acceptance')
                    if evidence is not None and meta['source_file_sha256'] != evidence:
                        raise ValueError('Measured source bytes changed between campaigns')
                    evidence = meta['source_file_sha256']
                    manifest['runs'].append({
                        'round': round_index + 1, 'comparison': comparison, 'workload': workload,
                        'waveform': wave, 'folder': folder.relative_to(output).as_posix(),
                        'files_sha256': {p.relative_to(folder).as_posix(): digest(p)
                                         for p in sorted(folder.rglob('*')) if p.is_file()},
                    })
                    save()
        unchanged()
        # Do not certify a complete collection if earlier evidence was modified.
        for run in manifest['runs']:
            for name, expected in run['files_sha256'].items():
                if digest(output / run['folder'] / name) != expected:
                    raise ValueError('Previously collected evidence changed')
        for name, expected in manifest['build_evidence_sha256'].items():
            if digest(output / name) != expected:
                raise ValueError('Qualification evidence changed')
        manifest['state'] = 'complete'
        manifest['source_file_sha256'] = evidence
        manifest['completed_utc'] = datetime.now(timezone.utc).isoformat()
        save()
        return manifest
    except Exception as error:
        manifest['state'] = 'failed'
        manifest['error'] = str(error)
        save()
        raise


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir', required=True, type=Path)
    parser.add_argument('--output-dir', required=True, type=Path)
    parser.add_argument('--rounds', type=int, default=2)
    parser.add_argument('--cmake', default='cmake')
    parser.add_argument('--ctest', default='ctest')
    args = parser.parse_args()
    try:
        collect(args.build_dir, args.output_dir, args.rounds, args.cmake, args.ctest)
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        parser.exit(1, f'Collection failed: {error}\n')
