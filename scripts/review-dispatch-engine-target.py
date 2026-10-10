#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Read-only integrity review of collected dispatch grids; no CPU acceptance."""
import argparse
import hashlib
import json
import math
from pathlib import Path, PurePosixPath
import re


def digest(path):
    with path.open('rb') as stream:
        value = hashlib.sha256()
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            value.update(chunk)
    return value.hexdigest()


def document(path):
    def pairs(items):
        result = {}
        for key, value in items:
            if key in result:
                raise ValueError('Duplicate JSON key: ' + key)
            result[key] = value
        return result
    return json.loads(path.read_text(encoding='utf-8'), object_pairs_hook=pairs,
                      parse_constant=lambda value: (_ for _ in ()).throw(ValueError('Non-finite JSON: ' + value)))


def require(condition, message):
    if not condition:
        raise ValueError(message)


def relative(root, name):
    require(isinstance(name, str) and bool(name) and '\\' not in name, 'Invalid archive path')
    part = PurePosixPath(name)
    require(not part.is_absolute() and '..' not in part.parts and part.as_posix() == name,
            'Unsafe archive path: ' + name)
    path = root / name
    require(root.resolve() in path.resolve().parents, 'Archive path escapes root: ' + name)
    return path


def hashes(root, recorded):
    require(isinstance(recorded, dict) and bool(recorded), 'Missing hash evidence')
    for name, expected in recorded.items():
        require(isinstance(expected, str) and re.fullmatch('[0-9a-f]{64}', expected), 'Invalid SHA-256')
        path = relative(root, name)
        require(path.is_file() and digest(path) == expected, 'Missing or changed evidence: ' + name)


def review(folder):
    folder = Path(folder).resolve()
    require(folder.is_dir(), 'Missing archive directory')
    require(not any(p.is_symlink() for p in folder.rglob('*')), 'Archive contains symlinks')
    manifest = document(folder / 'manifest.json')
    require(manifest['schema_version'] == 1 and manifest['state'] == 'complete', 'Incomplete collection')
    require(re.fullmatch('[0-9a-f]{40}', manifest['source_sha']), 'Invalid source SHA')
    rounds = manifest['rounds_requested']
    require(type(rounds) is int and 2 <= rounds <= 10, 'Invalid round count')
    require(manifest['study_kind'] == 'rate-saw-dispatch' and manifest['candidate_variant'] == 'saw-dispatch-v1'
            and manifest['configuration'] == 'Release', 'Unexpected collection identity')
    for flag in ('native_host_acceptance', 'production_promotion_allowed', 'background_load_controlled', 'affinity_applied'):
        require(manifest[flag] is False, 'Unexpected acceptance/control claim')
    qualification = {'configure.txt', 'build.txt', 'contracts.txt', 'complex-controls.txt',
                     *[f'waveform-{wave}-contracts.txt' for wave in range(4)]}
    require(set(manifest['build_evidence_sha256']) == qualification, 'Incomplete qualification evidence')
    hashes(folder, manifest['build_evidence_sha256'])
    recorded_files = {'manifest.json', *qualification}
    source_hashes = manifest['source_file_sha256']
    require(isinstance(source_hashes, dict) and bool(source_hashes), 'Missing source evidence')
    for name, value in source_hashes.items():
        relative(folder, name)
        require(isinstance(value, str) and re.fullmatch('[0-9a-f]{64}', value), 'Invalid source hash')
    scenes = [(workload, wave) for workload in ('stationary-v1', 'modulated-v1') for wave in range(4)]
    expected = []
    for index in range(rounds):
        comparisons = ('reference-repeat', 'candidate') if index % 2 == 0 else ('candidate', 'reference-repeat')
        for comparison in comparisons:
            for workload, wave in (scenes if index % 2 == 0 else reversed(scenes)):
                expected.append((index + 1, comparison, workload, wave))
    require(len(manifest['runs']) == len(expected), 'Missing or extra campaigns')
    rows = []
    for run, identity in zip(manifest['runs'], expected):
        require(tuple(run[k] for k in ('round', 'comparison', 'workload', 'waveform')) == identity,
                'Campaign identity/order mismatch')
        round_number, comparison, workload, wave = identity
        name = f'round-{round_number:02d}/{comparison}/{workload}/waveform-{wave}'
        require(run['folder'] == name, 'Campaign folder mismatch')
        grid = relative(folder, name)
        files = run['files_sha256']
        required = {'summary.csv', 'blocks.csv', 'factors.csv', 'backend.txt', 'workload.txt', 'study.txt',
                    'waveform.txt', 'comparison.txt', 'metadata.json', 'paired-results.json', 'report.md',
                    'measurement.txt', 'reporter.txt', 'compiler/CMakeCXXCompiler.cmake'}
        require(required <= set(files), 'Incomplete grid evidence')
        require(set(files) == {p.relative_to(grid).as_posix() for p in grid.rglob('*') if p.is_file()},
                'Unrecorded or missing grid file')
        hashes(grid, files)
        recorded_files.update(name + '/' + entry for entry in files)
        require(files['compiler/CMakeCXXCompiler.cmake'] == manifest['compiler_sha256'], 'Compiler mismatch')
        payload = document(grid / 'paired-results.json')
        meta = payload['metadata']
        require(document(grid / 'metadata.json') == meta, 'Metadata copies disagree')
        require(payload['schema_version'] == 1 and payload['ratio_direction'] == 'study/reference'
                and payload['native_host_acceptance'] is False, 'Unexpected report contract')
        require((meta['source_sha'], meta['comparison'], meta['workload'], meta['oscillator_waveform']) ==
                (manifest['source_sha'], comparison, workload, wave), 'Report identity mismatch')
        require(meta['source_file_sha256'] == source_hashes and meta['configuration'] == 'Release'
                and meta['study_kind'] == 'rate-saw-dispatch' and meta['production_activation'] is False
                and meta['native_host_acceptance'] is False, 'Report provenance mismatch')
        variant = 'rate-lookup-seven-saw' if comparison == 'reference-repeat' else 'rate-lookup-saw-dispatch-v1'
        require(meta['reference_variant'] == 'rate-lookup-seven-saw' and meta['study_variant'] == variant,
                'Report variant mismatch')
        for marker, value in [('comparison.txt', comparison), ('workload.txt', workload),
                              ('waveform.txt', str(wave)), ('study.txt', 'saw-dispatch-v1')]:
            require((grid / marker).read_text(encoding='utf-8').strip() == value, 'Compiled marker mismatch')
        for evidence in ('measurement_sha256', 'compiler_files_sha256'):
            require(bool(meta[evidence]) and all(files.get(k) == v for k, v in meta[evidence].items()),
                    'Report hashes disagree')
        cells = payload['paired_results']
        require(len(cells) == 9 and {(c['rate'], c['buffer']) for c in cells} ==
                {(r, b) for r in (48000, 96000, 192000) for b in (32, 64, 128)}, 'Incomplete report grid')
        for cell in cells:
            require(cell['paired_observations'] == 16 and cell['blocks_per_path'] == 4096, 'Invalid observation count')
            for key in ('median_paired_p50_ratio', 'median_paired_p99_ratio'):
                value = cell[key]
                require(type(value) in (int, float) and math.isfinite(value) and value > 0, 'Invalid timing ratio')
            for key in ('reference_over_budget_blocks', 'study_over_budget_blocks'):
                require(type(cell[key]) is int and 0 <= cell[key] <= 4096, 'Invalid deadline count')
            rows.append(dict(round=round_number, comparison=comparison, workload=workload, waveform=wave, **cell))
    require(recorded_files == {p.relative_to(folder).as_posix() for p in folder.rglob('*') if p.is_file()},
            'Unrecorded archive evidence')
    return {'source_sha': manifest['source_sha'], 'manifest_sha256': digest(folder / 'manifest.json'),
            'platform': manifest['platform'], 'machine': manifest['machine'], 'rows': rows,
            'native_host_acceptance': False, 'production_promotion_allowed': False}


def render(result):
    lines = ['# Dispatch target archive review', '', f"Source: {result['source_sha']}",
             f"Platform: {result['platform']}; machine: {result['machine']}",
             f"Manifest SHA-256: {result['manifest_sha256']}", '',
             'Checksums and recorded report contracts verified. Ratios are study/reference.',
             'Raw CSV percentiles are not recomputed here. Hashes establish consistency, not authenticity.',
             'No CPU/native-host acceptance; controls are not subtracted; every round remains separate.', '',
             '| Round | Comparison | Workload | Waveform | Rate | Buffer | p50 ratio | p99 ratio | Reference misses | Study misses |',
             '| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |']
    for row in result['rows']:
        values = [row[k] for k in ('round', 'comparison', 'workload', 'waveform', 'rate', 'buffer')]
        values += [f"{row[k]:.6f}" for k in ('median_paired_p50_ratio', 'median_paired_p99_ratio')]
        values += [row[k] for k in ('reference_over_budget_blocks', 'study_over_budget_blocks')]
        lines.append('| ' + ' | '.join(map(str, values)) + ' |')
    return '\n'.join(lines) + '\n'


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archive', type=Path)
    args = parser.parse_args()
    try:
        print(render(review(args.archive)), end='')
    except (ValueError, OSError, KeyError, TypeError) as error:
        parser.exit(1, f'Archive review failed: {error}\n')
