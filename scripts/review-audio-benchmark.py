#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Read-only audit of a completed production benchmark archive; never runs audio."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import statistics

spec = importlib.util.spec_from_file_location('production_benchmark',
    Path(__file__).resolve().with_name('benchmark-audio.py'))
benchmark = importlib.util.module_from_spec(spec)
spec.loader.exec_module(benchmark)


def require(condition, message):
    if not condition:
        raise ValueError(message)


def document(text):
    def pairs(items):
        result = {}
        for key, value in items:
            require(key not in result, 'Duplicate JSON key: ' + key)
            result[key] = value
        return result
    return json.loads(text, object_pairs_hook=pairs,
        parse_constant=lambda value: (_ for _ in ()).throw(ValueError('Nonfinite JSON: ' + value)))


def review(folder):
    folder = Path(folder).resolve(strict=True)
    raw_manifest = (folder / 'manifest.json').read_bytes()
    manifest = document(raw_manifest.decode('utf-8'))
    require(manifest['schema'] == 1, 'Unknown evidence schema')
    require(manifest['status'] in ('success', 'failed'), 'Incomplete benchmark')
    metadata = manifest['metadata']
    require(metadata['baseline_commit'] == benchmark.base, 'Unexpected baseline commit')
    require(re.fullmatch('[0-9a-f]{40}', metadata['current_commit']) is not None,
            'Invalid current commit')
    require(type(metadata['repeats']) is int and metadata['repeats'] == benchmark.REPEATS,
            'Unexpected repeat count')
    expected_thresholds = dict(cpu_relative=benchmark.CPU_RELATIVE_LIMIT,
        cpu_absolute=benchmark.CPU_ABSOLUTE_LIMIT, level_db=benchmark.LEVEL_DB_LIMIT,
        dc=benchmark.MAX_CURRENT_DC)
    require(metadata['thresholds'] == expected_thresholds, 'Thresholds differ from production gate')
    require(re.fullmatch('[0-9a-f]{64}', metadata['workload_sha256']) is not None,
            'Invalid workload digest')
    for label in ('baseline', 'current'):
        require(re.fullmatch('[0-9a-f]{64}', manifest['builds'][label]['binary_sha256']) is not None,
                'Missing recorded executable digest')
    schedule = [('baseline', 'warmup', 0), ('current', 'warmup', 0)]
    for repeat in range(benchmark.REPEATS):
        order = ('baseline', 'current') if repeat % 2 == 0 else ('current', 'baseline')
        schedule.extend((label, 'timed', repeat + 1) for label in order)
    records = manifest['records']
    require(len(records) == len(schedule), 'Expected both warmups and every timed pair')
    names = {'manifest.json'}
    reports = {'baseline': [], 'current': []}
    for index, (record, expected) in enumerate(zip(records, schedule)):
        label, phase, repeat = expected
        require((record['label'], record['phase'], record['repeat']) == expected,
                'Recorded execution order differs from balanced schedule')
        name = f'{index:03d}-{label}-{phase}-{repeat}.csv'
        require(record['file'] == name and record['returncode'] == 0,
                'Unexpected raw filename or failed executable')
        names.add(name)
        path = folder / name
        require(not path.is_symlink(), 'Raw report symlink is not allowed')
        data = path.read_bytes()
        require(hashlib.sha256(data).hexdigest() == record['sha256'], 'Raw report hash mismatch')
        report = benchmark.parse_report(data.decode('utf-8'))
        if label == 'current':
            benchmark.check_current_dc(report)
        if phase == 'timed':
            reports[label].append(report)
    require(not (folder / 'manifest.json').is_symlink(), 'Manifest symlink is not allowed')
    require({path.name for path in folder.iterdir()} == names, 'Missing or extra archive files')
    medians = {label: benchmark.median_report(value) for label, value in reports.items()}
    scenes = []
    for scene, voices in sorted(benchmark.EXPECTED_SCENARIOS):
        baseline, current = (medians[label][scene, voices] for label in ('baseline', 'current'))
        old, new = baseline['realtime_percent'], current['realtime_percent']
        limit = max(old * benchmark.CPU_RELATIVE_LIMIT, old + benchmark.CPU_ABSOLUTE_LIMIT)
        peak = current['peak_dbfs'] - baseline['peak_dbfs']
        rms = current['rms_dbfs'] - baseline['rms_dbfs']
        # Diagnostic only: keep every pair and both execution orders. The
        # production decision below still compares the two original medians.
        pairs = []
        for index in range(benchmark.REPEATS):
            old_pair, new_pair = (reports[label][index][scene, voices]['realtime_percent']
                                  for label in ('baseline', 'current'))
            pairs.append(dict(repeat=index + 1,
                first='baseline' if index % 2 == 0 else 'current',
                baseline_cpu=old_pair, current_cpu=new_pair,
                cpu_change_pct=100 * (new_pair / old_pair - 1),
                threshold_exceeded=new_pair > max(old_pair * benchmark.CPU_RELATIVE_LIMIT,
                                                  old_pair + benchmark.CPU_ABSOLUTE_LIMIT)))
        variation = {}
        for label in ('baseline', 'current'):
            values = [row[label + '_cpu'] for row in pairs]
            variation[label] = dict(min=min(values), max=max(values),
                span_pct_of_median=100 * (max(values) - min(values)) / statistics.median(values))
        scenes.append(dict(scene=scene, voices=voices, baseline_cpu=old, current_cpu=new,
            cpu_change_pct=100 * (new / old - 1), cpu_limit=limit,
            cpu_regression=new > limit, peak_delta_db=peak, rms_delta_db=rms,
            level_regression=abs(peak) > benchmark.LEVEL_DB_LIMIT or abs(rms) > benchmark.LEVEL_DB_LIMIT,
            timing_diagnostics=dict(pairs=pairs, variation=variation,
                pair_threshold_exceedances=sum(row['threshold_exceeded'] for row in pairs),
                replaces_production_gate=False)))
    regression = any(row['cpu_regression'] or row['level_regression'] for row in scenes)
    require(manifest['status'] == ('failed' if regression else 'success'),
            'Recorded status disagrees with recomputed production gate')
    return dict(schema=1, raw_reports_verified=True, records=len(records), scenes=scenes,
        recorded_status=manifest['status'], regression_detected=regression,
        manifest_sha256=hashlib.sha256(raw_manifest).hexdigest(), metadata=metadata,
        recorded_builds=manifest['builds'],
        native_host_acceptance=False, production_promotion_allowed=False,
        limitation='Checks internal consistency, not authenticity or executable bytes; exit 0 means audit succeeded, even for a failed benchmark.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('folder', type=Path)
    args = parser.parse_args()
    try:
        print(json.dumps(review(args.folder), indent=2, allow_nan=False))
    except (OSError, ValueError, KeyError, TypeError, RuntimeError) as error:
        parser.exit(1, f'Benchmark evidence audit failed: {error}\n')
