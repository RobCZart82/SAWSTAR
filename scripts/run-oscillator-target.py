#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build and collect repeated local oscillator campaigns; no CPU acceptance."""
import argparse
import contextlib
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('oscillator_report', ROOT / 'scripts/report-oscillator-cache.py')
reporter = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reporter)

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def source_identity():
    def git(*args):
        return subprocess.check_output(['git', *args], cwd=ROOT, text=True).strip()
    sha = git('rev-parse', 'HEAD')
    if not re.fullmatch('[0-9a-f]{40}', sha):
        raise ValueError('Exact source commit required')
    if git('status', '--porcelain', '--untracked-files=all'):
        raise ValueError('Use a clean checkout, including untracked files')
    pinned = git('ls-tree', 'HEAD', 'third_party/DaisySP').split()
    if len(pinned) < 3 or pinned[0] != '160000':
        raise ValueError('Pinned DaisySP gitlink required')
    daisy = git('-C', 'third_party/DaisySP', 'rev-parse', 'HEAD')
    if daisy != pinned[2] or git('-C', 'third_party/DaisySP', 'status', '--porcelain', '--untracked-files=all'):
        raise ValueError('DaisySP must match the clean pinned commit')
    return sha

def checked(command, log):
    with log.open('wb') as stream:
        subprocess.run([str(x) for x in command], cwd=ROOT, stdout=stream,
                       stderr=subprocess.STDOUT, check=True)

def collect(build, output, rounds=2, cmake='cmake', ctest='ctest'):
    build, output = Path(build).resolve(), Path(output).resolve()
    if not 2 <= rounds <= 10:
        raise ValueError('Use 2 to 10 complete rounds')
    # Keep generated files outside the checkout so source cleanliness is meaningful.
    if any(path == ROOT or ROOT in path.parents for path in (build, output)):
        raise ValueError('Build and output directories must be outside the checkout')
    if build == output or build in output.parents or output in build.parents:
        raise ValueError('Build and output directories must be separate')
    if build.exists() or output.exists():
        raise ValueError('Use fresh build and output directories; existing results are never overwritten')
    sha = source_identity()
    build.mkdir(parents=True)
    output.mkdir(parents=True)
    manifest = {
        'schema_version': 1, 'source_sha': sha, 'state': 'incomplete',
        'started_utc': datetime.now(timezone.utc).isoformat(),
        'platform': platform.platform(), 'machine': platform.machine(),
        'processor': platform.processor(), 'python': platform.python_version(),
        'rounds_requested': rounds, 'configuration': 'Release', 'campaign': 'extended-v1',
        'background_load_controlled': False, 'affinity_applied': False,
        'native_host_acceptance': False, 'production_promotion_allowed': False,
        'commands': [], 'runs': [],
    }
    def save():
        (output / 'manifest.json').write_text(json.dumps(manifest, indent=2, allow_nan=False) + '\n', encoding='utf-8')
    def command(args, log):
        manifest['commands'].append([str(x) for x in args])
        save()
        checked(args, output / log)
    save()
    try:
        command([cmake, '-S', ROOT, '-B', build, '-DCMAKE_BUILD_TYPE=Release',
                 '-DBUILD_TESTING=ON', '-DSAWSTAR_CHECK_DAISYSP=ON'], 'configure.txt')
        command([cmake, '--build', build, '--config', 'Release', '--parallel', '2', '--target',
                 'sawstar_oscillator_shared_tests', 'sawstar_oscillator_shared_benchmark',
                 'sawstar_sevensaw_tests'], 'build.txt')
        command([ctest, '--test-dir', build, '-C', 'Release', '--verbose', '--timeout', '600',
                 '--no-tests=error', '-R', '^(seven_saw|oscillator_shared_bit_identity|oscillator_cache_report|oscillator_target_runner)$'],
                'contracts.txt')
        name = 'sawstar_oscillator_shared_benchmark' + ('.exe' if sys.platform == 'win32' else '')
        executables = [path for path in (build / 'Release' / name, build / name) if path.is_file()]
        if len(executables) != 1:
            raise ValueError('Expected exactly one freshly built Release benchmark')
        executable = executables[0]
        executable_hash = digest(executable)
        compilers = sorted((build / 'CMakeFiles').rglob('CMakeCXXCompiler.cmake'))
        if len(compilers) != 1:
            raise ValueError('Expected one compiler evidence file')
        manifest['executable_sha256'] = executable_hash
        manifest['runner_sha256'] = digest(Path(__file__))
        manifest['reporter_sha256'] = digest(ROOT / 'scripts/report-oscillator-cache.py')
        evidence = None
        for round_index in range(rounds):
            comparisons = ['reference-repeat', 'candidate']
            if round_index % 2:
                comparisons.reverse()
            for comparison in comparisons:
                if source_identity() != sha or digest(executable) != executable_hash:
                    raise ValueError('Source or executable changed during collection')
                folder = output / f'round-{round_index + 1:02d}' / comparison
                (folder / 'compiler').mkdir(parents=True)
                shutil.copyfile(compilers[0], folder / 'compiler/CMakeCXXCompiler.cmake')
                shutil.copyfile(output / 'contracts.txt', folder / 'contracts.txt')
                args = [str(executable), '--extended']
                if comparison == 'reference-repeat':
                    args.append('--reference-repeat')
                manifest['commands'].append(args)
                save()
                print(f'Round {round_index + 1}/{rounds}: {comparison}', flush=True)
                with (folder / 'oscillators.csv').open('wb') as stream, (folder / 'stderr.txt').open('wb') as errors:
                    subprocess.run(args, cwd=ROOT, stdout=stream, stderr=errors, check=True)
                buffer = io.StringIO()
                with contextlib.redirect_stdout(buffer):
                    result = reporter.report(folder, sha, variant='shared-frequency-v1',
                                             campaign='extended-v1', comparison=comparison)
                (folder / 'reporter.txt').write_text(buffer.getvalue(), encoding='utf-8')
                source_hashes = result['metadata']['source_file_sha256']
                if evidence is not None and source_hashes != evidence:
                    raise ValueError('Measured source bytes changed between campaigns')
                evidence = source_hashes
                manifest['runs'].append({
                    'round': round_index + 1, 'comparison': comparison,
                    'folder': folder.relative_to(output).as_posix(),
                    'files_sha256': {p.relative_to(folder).as_posix(): digest(p)
                                     for p in sorted(folder.rglob('*')) if p.is_file()},
                })
                save()
        if source_identity() != sha or digest(executable) != executable_hash:
            raise ValueError('Source or executable changed before completion')
        manifest['state'] = 'complete'
        manifest['source_file_sha256'] = evidence
        manifest['completed_utc'] = datetime.now(timezone.utc).isoformat()
        manifest['build_evidence_sha256'] = {name: digest(output / name)
                                             for name in ('configure.txt', 'build.txt', 'contracts.txt')}
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
