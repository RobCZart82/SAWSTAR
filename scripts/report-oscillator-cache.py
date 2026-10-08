#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Validate an isolated paired oscillator-bank study; no realtime acceptance."""
import argparse
import csv
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import re
import statistics

def report(folder, source_sha, emit_ci=False):
    folder = Path(folder)
    if not re.fullmatch(r'[0-9a-f]{40}', source_sha):
        raise ValueError('Exact source SHA required')
    with (folder / 'oscillators.csv').open(newline='') as f:
        rows = list(csv.DictReader(f))
    expected = {(path, rate, wave, moving, pair)
                for path in ('reference', 'study') for rate in (48000, 96000, 192000)
                for wave in range(4) for moving in range(2) for pair in range(4)}
    samples = {}
    for row in rows:
        key = row['path'], int(row['rate']), int(row['waveform']), int(row['modulated']), int(row['pair'])
        if key not in expected or key in samples:
            raise ValueError('Unexpected or duplicate oscillator pair')
        if int(row['oscillators']) != 32 or int(row['frames']) != 8192:
            raise ValueError('Incorrect bank or frame count')
        if row['order'] != ('study-first' if key[-1] % 2 else 'reference-first'):
            raise ValueError('Incorrect alternating order')
        seconds, energy = float(row['seconds']), float(row['energy'])
        if not all(math.isfinite(x) and x > 0 for x in (seconds, energy)):
            raise ValueError('Nonfinite or nonpositive measurement')
        samples[key] = seconds, energy
    if len(rows) != 192 or set(samples) != expected:
        raise ValueError('Incomplete oscillator grid')
    summaries = []
    for rate in (48000, 96000, 192000):
        for wave in range(4):
            for moving in range(2):
                ratios = []
                for pair in range(4):
                    a = samples['reference', rate, wave, moving, pair]
                    b = samples['study', rate, wave, moving, pair]
                    if a[1] != b[1]:
                        raise ValueError('Paired energy differs')
                    ratios.append(b[0] / a[0])
                summaries.append({'rate': rate, 'waveform': wave, 'modulated': bool(moving),
                                  'paired_observations': 4, 'median_paired_seconds_ratio': statistics.median(ratios),
                                  'paired_ratios': ratios})
    root = Path(__file__).resolve().parents[1]
    sources = ('src/dsp/SevenSaw.h', 'src/dsp/SevenSaw.cpp', 'experiments/oscillator/CachedSevenSaw.h',
               'experiments/oscillator/CachedSevenSaw.cpp', 'experiments/oscillator/benchmark_frequency_cache.cpp',
               'tests/oscillator_frequency_cache.cpp', 'third_party/DaisySP/Source/Synthesis/oscillator.h',
               'third_party/DaisySP/Source/Synthesis/oscillator.cpp', 'third_party/DaisySP/Source/Utility/dsp.h')
    metadata = {'source_sha': source_sha, 'platform': platform.platform(), 'machine': platform.machine(),
                'runner_os': os.environ.get('RUNNER_OS'), 'configuration': 'Release',
                'raw_sha256': hashlib.sha256((folder / 'oscillators.csv').read_bytes()).hexdigest(),
                'source_file_sha256': {name: hashlib.sha256((root / name).read_bytes()).hexdigest() for name in sources},
                'compiler_files_sha256': {str(p.relative_to(folder)): hashlib.sha256(p.read_bytes()).hexdigest()
                                          for p in sorted((folder / 'compiler').glob('*')) if p.is_file()},
                'oscillators': 32, 'voices_represented': 16, 'frames_per_path_and_pair': 8192,
                'warmup_seconds': .25, 'native_host_acceptance': False,
                'scope': 'isolated-oscillator-bank', 'ratio_direction': 'study/reference',
                'timed_work': 'bank processing, pitch setters/sine when modulated, energy accumulation',
                'production_activation': False}
    payload = {'schema_version': 1, 'metadata': metadata, 'summary': summaries}
    (folder / 'report.json').write_text(json.dumps(payload, indent=2, allow_nan=False) + '\n', encoding='utf-8')
    lines = ['# Isolated frequency-cache oscillator study', '', f'Source: {source_sha}',
             '32 oscillator banks represent OSC1 + OSC2 for 16 voices; no engine, filter or FX timing.',
             'Four alternating pairs. Ratios are study/reference; smaller is favorable.',
             'Static and audio-rate pitch modulation are separate controls. No native realtime acceptance.', '',
             '| Rate | Waveform | Pitch modulation | Median paired time ratio |', '| --- | --- | --- | --- |']
    for row in summaries:
        lines.append(f"| {row['rate']} | {row['waveform']} | {row['modulated']} | {row['median_paired_seconds_ratio']:.6f} |")
    text = '\n'.join(lines) + '\n'
    (folder / 'report.md').write_text(text, encoding='utf-8')
    print(text)
    if emit_ci:
        print('OSCILLATOR_CI_REPORT=' + json.dumps(payload, separators=(',', ':'), allow_nan=False))
    return payload

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('folder', type=Path)
    parser.add_argument('--source-sha', required=True)
    parser.add_argument('--emit-ci', action='store_true')
    args = parser.parse_args()
    report(args.folder, args.source_sha, args.emit_ci)
