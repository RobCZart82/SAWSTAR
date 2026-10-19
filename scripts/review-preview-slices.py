#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Read-only review of retained schema-2 numerical evidence; never runs audio."""
import argparse
import csv
import hashlib
import itertools
import json
from pathlib import Path
import re

BASE_FIELDS = ('waveform', 'rate', 'filter_mode', 'voice_mode', 'fx',
               'output_peak_error', 'output_relative_rms',
               'prefx_peak_error', 'prefx_relative_rms')
DIGEST_FIELDS = ('output_reference_digest', 'output_candidate_digest',
                 'prefx_reference_digest', 'prefx_candidate_digest')


def require(condition, message):
    if not condition:
        raise ValueError(message)


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        require(key not in result, 'Duplicate manifest key: ' + key)
        result[key] = value
    return result


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def regular(root, name):
    path = root / name
    require(path.parent == root and not path.is_symlink() and path.is_file(),
            'Missing, linked or unsafe evidence file: ' + name)
    return path


def verify_csv(path, wave):
    diagnostic = wave == 2
    with path.open(encoding='utf-8', newline='') as stream:
        reader = csv.reader(stream)
        require(tuple(next(reader, ())) == BASE_FIELDS + (DIGEST_FIELDS if diagnostic else ()),
                'CSV header mismatch: ' + path.name)
        rows = list(reader)
    width = 13 if diagnostic else 9
    require(len(rows) == 120 and all(len(row) == width for row in rows),
            'Incomplete or malformed CSV: ' + path.name)
    expected = set(itertools.product((str(wave),),
                   ('48000', '96000', '176400', '192000', '384000'),
                   ('0', '1', '2', '3'), ('0', '1', '2'), ('0', '1')))
    require({tuple(row[:5]) for row in rows} == expected,
            'Missing, duplicate or unexpected case: ' + path.name)
    require(all(row[5:9] == ['0'] * 4 for row in rows),
            'Nonzero bit-identity error: ' + path.name)
    if not diagnostic:
        return None
    require(all(all(re.fullmatch('[0-9a-f]{16}', value) for value in row[9:]) for row in rows),
            'Malformed signal fingerprint: ' + path.name)
    require(all(row[9] == row[10] and row[11] == row[12] for row in rows),
            'Reference/Candidate fingerprint mismatch: ' + path.name)
    # SHA-256 of the declared noncryptographic FNV table, not raw audio SHA.
    table = sorted(tuple(row[:5] + row[9:]) for row in rows)
    return hashlib.sha256(json.dumps(table, separators=(',', ':')).encode()).hexdigest()


def review(directory, expected_source):
    require(re.fullmatch('[0-9a-f]{40}', expected_source) is not None,
            'Expected source must be a full lowercase commit SHA')
    root = Path(directory).resolve()
    manifest_path = regular(root, 'manifest.json')
    manifest = json.loads(manifest_path.read_text(encoding='utf-8'),
                          object_pairs_hook=unique_object)
    require(isinstance(manifest, dict), 'Manifest must be an object')
    require(type(manifest.get('schema')) is int and manifest['schema'] == 2 and
            manifest.get('state') == 'complete' and 'error' not in manifest,
            'Manifest is not a completed schema-2 collection')
    require(manifest.get('source_commit') == expected_source, 'Source commit mismatch')
    require(manifest.get('native_host_acceptance') is False and
            manifest.get('production_promotion_allowed') is False,
            'Numerical evidence cannot claim native acceptance or promotion')
    require(type(manifest.get('cold_runs_per_architecture')) is int and
            manifest['cold_runs_per_architecture'] == 4, 'Cold-process plan mismatch')
    for name, key in [('sawstar_premium_native_pair', 'executable_sha256'),
                      ('CMakeCache.txt', 'cmake_cache_sha256')]:
        require(sha(regular(root, name)) == manifest.get(key), 'SHA-256 mismatch: ' + name)
    expected = [(wave, arch, 0) for wave in range(4) for arch in ('arm64', 'x86_64')]
    expected += [(2, arch, cold) for cold in range(1, 4) for arch in ('arm64', 'x86_64')]
    results = manifest.get('results')
    require(isinstance(results, list) and len(results) == 14, 'Incomplete stream plan')
    baselines, checked, filenames = {}, [], {'manifest.json', 'sawstar_premium_native_pair', 'CMakeCache.txt'}
    for entry, (wave, arch, cold) in zip(results, expected):
        require(isinstance(entry, dict) and type(entry.get('waveform')) is int and
                type(entry.get('cold_run')) is int and
                (entry['waveform'], entry.get('architecture'), entry['cold_run']) == (wave, arch, cold),
                'Stream identity/order mismatch')
        require(entry.get('translated') is (arch == 'x86_64'), 'Translation label mismatch')
        require(type(entry.get('returncode')) is int and entry['returncode'] == 0 and
                'validation_error' not in entry, 'Failed numerical stream')
        filename = arch + '-waveform-' + str(wave) + ('-cold-' + str(cold) if cold else '') + '.txt'
        require(entry.get('log_file') == filename, 'Unexpected log filename')
        filenames.add(filename)
        path = regular(root, filename)
        require(sha(path) == entry.get('log_sha256'), 'SHA-256 mismatch: ' + filename)
        digest = verify_csv(path, wave)
        if wave == 2:
            require(digest == entry.get('fingerprint_table_sha256'),
                    'Fingerprint table SHA-256 mismatch: ' + filename)
            if cold == 0:
                baselines[arch] = digest
            require(digest == baselines[arch], 'Cold-process signal mismatch: ' + filename)
        checked.append(dict(entry, cases=120))
    require({path.name for path in root.iterdir()} == filenames, 'Unexpected evidence files')
    return dict(schema=1, state='verified', source_commit=expected_source,
                manifest_sha256=sha(manifest_path), executable_sha256=manifest['executable_sha256'],
                cmake_cache_sha256=manifest['cmake_cache_sha256'], streams=14,
                case_observations=1680, triangle_processes_per_architecture=4,
                triangle_fingerprint_tables=baselines, results=checked,
                purpose='retained numerical evidence review; no execution or CPU measurement',
                native_host_acceptance=False, production_promotion_allowed=False,
                original_numerical_failure_resolved=False)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('directory', type=Path)
    parser.add_argument('--expected-source', required=True)
    args = parser.parse_args()
    try:
        result = review(args.directory, args.expected_source)
    except (ValueError, OSError, TypeError, KeyError, csv.Error) as error:
        print(json.dumps(dict(state='failed', error=str(error),
                              native_host_acceptance=False, production_promotion_allowed=False)))
        return 1
    print(json.dumps(result, indent=2))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
