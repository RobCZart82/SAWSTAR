#!/usr/bin/env python3
"""Validate descriptive component timings; never infer additive CPU shares."""
import argparse
import base64
import copy
import csv
import hashlib
import itertools
import json
import math
from pathlib import Path
import re
import statistics

STAGES = ('oscillators', 'modulation', 'fx', 'drive', 'filter', 'adapter',
          'engine-no-fx', 'engine-fx', 'production-no-fx', 'production-fx')
FIELDS = ('engine', 'stage', 'rate', 'voices', 'filter_mode', 'drive_db',
          'frames', 'repeat', 'seconds', 'realtime_percent', 'energy')
GRID = set(itertools.product(STAGES, (48000, 96000, 192000), range(4), range(3)))

def validate(rows):
    seen = set()
    for row in rows:
        if set(row) != set(FIELDS):
            raise ValueError('unexpected CSV fields')
        if row['engine'] != 'rate-lookup':
            raise ValueError('wrong engine label')
        integers = {key: int(row[key]) for key in
                    ('rate', 'voices', 'filter_mode', 'drive_db', 'frames', 'repeat')}
        if (integers['voices'], integers['drive_db'], integers['frames']) != (16, 20, 16384):
            raise ValueError('wrong workload')
        key = (row['stage'], integers['rate'], integers['filter_mode'], integers['repeat'])
        if key not in GRID or key in seen:
            raise ValueError('unknown or duplicate case')
        seen.add(key)
        seconds, percent, energy = (float(row[k]) for k in ('seconds', 'realtime_percent', 'energy'))
        if not all(math.isfinite(x) and x > 0 for x in (seconds, percent, energy)):
            raise ValueError('nonfinite or nonpositive measurement')
        expected = 100 * seconds * integers['rate'] / integers['frames']
        if not math.isclose(percent, expected, rel_tol=2e-5, abs_tol=1e-8):
            raise ValueError('inconsistent realtime percent')
    if seen != GRID:
        raise ValueError('incomplete component grid')
    return rows

def backend(value):
    match = re.fullmatch(r'fir_backend=(SSE2|NEON|scalar-fallback);factor_192000=2;study=rate-lookup\s*', value)
    if not match:
        raise ValueError('unknown backend or routing metadata')
    return match[1]

def summarize(rows):
    result = []
    for stage, rate, mode in itertools.product(STAGES, (48000, 96000, 192000), range(4)):
        values = [float(r['realtime_percent']) for r in rows
                  if (r['stage'], int(r['rate']), int(r['filter_mode'])) == (stage, rate, mode)]
        result.append(dict(stage=stage, rate=rate, filter_mode=mode,
                           median_realtime_percent=statistics.median(values),
                           minimum_realtime_percent=min(values), maximum_realtime_percent=max(values)))
    return result

def self_test():
    rows = [dict(zip(FIELDS, ('rate-lookup', stage, str(rate), '16', str(mode), '20',
                             '16384', str(repeat), '.01', str(100 * .01 * rate / 16384), '1')))
            for stage, rate, mode, repeat in sorted(GRID)]
    validate(rows)
    assert len(summarize(rows)) == 120
    rejects = [rows[:-1], rows + [rows[0]]]
    for field, value in [('engine', 'premium'), ('stage', 'unknown'), ('rate', '44100'),
                         ('voices', '8'), ('filter_mode', '4'), ('drive_db', '24'),
                         ('frames', '4096'), ('repeat', '3'), ('seconds', 'nan'),
                         ('seconds', '0'), ('energy', 'inf'), ('energy', '-1'),
                         ('realtime_percent', '0'), ('realtime_percent', '99')]:
        bad = copy.deepcopy(rows); bad[0][field] = value; rejects.append(bad)
    bad = copy.deepcopy(rows); bad[0]['extra'] = '1'; rejects.append(bad)
    for bad in rejects:
        try:
            validate(bad)
        except ValueError:
            pass
        else:
            raise AssertionError('invalid measurement accepted')
    for name in ('SSE2', 'NEON', 'scalar-fallback'):
        assert backend(f'fir_backend={name};factor_192000=2;study=rate-lookup\n') == name
    for bad in ('', 'fir_backend=NEON;factor_192000=4;study=rate-lookup',
                'fir_backend=unknown;factor_192000=2;study=rate-lookup'):
        try:
            backend(bad)
        except ValueError:
            pass
        else:
            raise AssertionError('invalid backend accepted')
    print(f'360 component cases, 120 summaries, {len(rejects) + 3} rejection controls PASS')

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def report(directory, source_sha, source_root):
    if not re.fullmatch(r'[0-9a-f]{40}', source_sha):
        raise ValueError('source SHA must be a full commit identifier')
    with (directory / 'components.csv').open(newline='', encoding='utf-8') as stream:
        reader = csv.DictReader(stream)
        if tuple(reader.fieldnames or ()) != FIELDS:
            raise ValueError('unexpected or duplicate CSV columns')
        rows = validate(list(reader))
    compiled_backend = backend((directory / 'backend.txt').read_text(encoding='utf-8'))
    inputs = [directory / 'components.csv', directory / 'backend.txt', directory / 'contracts.txt']
    compilers = sorted((directory / 'compiler').glob('**/CMakeCXXCompiler.cmake'))
    if not compilers or any(not p.is_file() or p.stat().st_size == 0 for p in inputs + compilers):
        raise ValueError('missing raw, contract or compiler provenance')
    source_paths = ['CMakeLists.txt', '.github/workflows/premium-components.yml',
                    'experiments/premium_filter/benchmark_engine.cpp',
                    'experiments/premium_filter/EnginePremiumFilter.h',
                    'experiments/premium_filter/PremiumDrive.h',
                    'experiments/premium_filter/PremiumLowPass.h',
                    'experiments/premium_filter/FirLanes4.h',
                    'experiments/premium_filter/LookupTanh.h',
                    'experiments/premium_filter/RateScaledPremiumDrive.h',
                    'experiments/premium_filter/ResearchTanh.h',
                    'third_party/DaisySP/Source/Synthesis/oscillator.h',
                    'third_party/DaisySP/Source/Synthesis/oscillator.cpp',
                    'third_party/DaisySP/Source/Control/adsr.h',
                    'third_party/DaisySP/Source/Control/adsr.cpp',
                    'third_party/DaisySP/Source/Utility/dsp.h',
                    'scripts/report-premium-components.py']
    source_paths += [p.relative_to(source_root).as_posix() for folder in ('src/dsp', 'src/engine')
                     for p in sorted((source_root / folder).rglob('*')) if p.is_file()]
    metadata = dict(source_commit=source_sha, compiled_backend=compiled_backend,
                    workload='components-v1', raw_rows=len(rows),
                    warmup_seconds=.25, measured_frames=16384, repeats=3,
                    timed_instrumentation='loop/index and energy accumulation included',
                    global_stages=['modulation', 'fx'], bank_instances=16,
                    cpu_shares_additive=False, native_acceptance=False,
                    measurement_sha256={p.relative_to(directory).as_posix(): digest(p) for p in inputs + compilers},
                    source_sha256={p: digest(source_root / p) for p in source_paths})
    summaries = summarize(rows)
    with (directory / 'summary.csv').open('w', newline='', encoding='utf-8') as stream:
        writer = csv.DictWriter(stream, fieldnames=list(summaries[0])); writer.writeheader(); writer.writerows(summaries)
    metadata['summary_sha256'] = digest(directory / 'summary.csv')
    (directory / 'metadata.json').write_text(json.dumps(metadata, indent=2) + '\n', encoding='utf-8')
    lines = ['# Component timing observations', '',
             'Independent synthetic workloads; percentages are audio-duration ratios, not additive CPU shares.',
             'Full engines are measured separately. Three repeats on a shared runner do not establish realtime acceptance.', '',
             '| Rate | Mode | Stage | Median % | Min % | Max % |',
             '| --- | --- | --- | --- | --- | --- |']
    for r in summaries:
        lines.append(f"| {r['rate']} | {r['filter_mode']} | {r['stage']} | {r['median_realtime_percent']:.3f} | {r['minimum_realtime_percent']:.3f} | {r['maximum_realtime_percent']:.3f} |")
    (directory / 'report.md').write_text('\n'.join(lines) + '\n', encoding='utf-8')
    print(f'Validated {len(rows)} rows; descriptive reports written to {directory}')
    return dict(metadata=metadata, summaries=summaries,
                raw_csv_base64=base64.b64encode((directory / 'components.csv').read_bytes()).decode('ascii'))

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('directory', nargs='?', type=Path)
    parser.add_argument('--source-sha')
    parser.add_argument('--source-root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--self-test', action='store_true')
    parser.add_argument('--emit-ci', action='store_true',
                        help='print validated results and exact raw CSV bytes for CI log retrieval')
    args = parser.parse_args()
    try:
        if args.self_test:
            self_test()
        elif args.directory and args.source_sha:
            result = report(args.directory, args.source_sha, args.source_root)
            if args.emit_ci:
                print('COMPONENT_CI_REPORT=' + json.dumps(result, separators=(',', ':')))
        else:
            parser.error('provide directory and --source-sha, or --self-test')
    except (ValueError, OSError, KeyError, TypeError) as error:
        parser.exit(1, f'Invalid component measurements: {error}\n')

