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

def qualification(summaries):
    """Describe isolated observations, never promote a candidate from timing alone."""
    slower = []
    all_pairs_slower = []
    for row in summaries:
        cell = {key: row[key] for key in ('rate', 'waveform', 'modulated')}
        if row['median_paired_seconds_ratio'] > 1:
            slower.append(cell)
        if all(ratio > 1 for ratio in row['paired_ratios']):
            all_pairs_slower.append(cell)
    return {
        'cpu_acceptance': 'not-established',
        'production_promotion_allowed': False,
        'median_slowdown_cells': slower,
        'all_pairs_slowdown_cells': all_pairs_slower,
        'interpretation': 'Observed ratios, not statistical significance or a native realtime gate.',
        'remaining_gates': ['controlled-target-repeat', 'full-engine-modulated-deadline',
                            'native-host-realtime'],
    }

def report(folder, source_sha, emit_ci=False, variant="held-saw-tuning-v2",
           campaign="short-v1", comparison="candidate"):
    if variant not in ("held-saw-tuning-v2", "shared-frequency-v1"):
        raise ValueError("Unknown oscillator candidate variant")
    if campaign not in ("short-v1", "extended-v1") or comparison not in ("candidate", "reference-repeat"):
        raise ValueError("Unknown campaign or comparison")
    extended = campaign == "extended-v1"
    if (extended and variant != "shared-frequency-v1") or (comparison == "reference-repeat" and not extended):
        raise ValueError("Incompatible campaign or comparison")
    pairs, frames = (8, 131072) if extended else (4, 8192)
    folder = Path(folder)
    if not re.fullmatch(r'[0-9a-f]{40}', source_sha):
        raise ValueError('Exact source SHA required')
    with (folder / 'oscillators.csv').open(newline='') as f:
        reader = csv.DictReader(f)
        columns = ['path', 'rate', 'waveform', 'modulated', 'pair', 'order', 'oscillators',
                   'frames', 'seconds', 'energy', 'variant']
        if extended:
            columns += ['campaign', 'comparison', 'warmup_frames']
        if reader.fieldnames != columns:
            raise ValueError('Incorrect campaign columns')
        rows = list(reader)
    expected = {(path, rate, wave, moving, pair)
                for path in ('reference', 'study') for rate in (48000, 96000, 192000)
                for wave in range(4) for moving in range(2) for pair in range(pairs)}
    samples = {}
    for row in rows:
        if row.get('variant') != variant:
            raise ValueError('Incorrect compiled candidate variant')
        key = row['path'], int(row['rate']), int(row['waveform']), int(row['modulated']), int(row['pair'])
        if key not in expected or key in samples:
            raise ValueError('Unexpected or duplicate oscillator pair')
        if int(row['oscillators']) != 32 or int(row['frames']) != frames:
            raise ValueError('Incorrect bank or frame count')
        if extended and (row['campaign'] != campaign or row['comparison'] != comparison
                         or int(row['warmup_frames']) != key[1] // 4):
            raise ValueError('Incorrect campaign, comparison or warmup')
        if row['order'] != ('study-first' if key[-1] % 2 else 'reference-first'):
            raise ValueError('Incorrect alternating order')
        seconds, energy = float(row['seconds']), float(row['energy'])
        if not all(math.isfinite(x) and x > 0 for x in (seconds, energy)):
            raise ValueError('Nonfinite or nonpositive measurement')
        samples[key] = seconds, energy
    if len(rows) != 48 * pairs or set(samples) != expected:
        raise ValueError('Incomplete oscillator grid')
    summaries = []
    for rate in (48000, 96000, 192000):
        for wave in range(4):
            for moving in range(2):
                ratios = []
                for pair in range(pairs):
                    a = samples['reference', rate, wave, moving, pair]
                    b = samples['study', rate, wave, moving, pair]
                    if a[1] != b[1]:
                        raise ValueError('Paired energy differs')
                    ratios.append(b[0] / a[0])
                summaries.append({'rate': rate, 'waveform': wave, 'modulated': bool(moving),
                                  'paired_observations': pairs, 'median_paired_seconds_ratio': statistics.median(ratios),
                                  'paired_ratios': ratios})
                if extended:
                    # Keep every observation; a wide range does not license dropping a pair.
                    summaries[-1].update(min_paired_seconds_ratio=min(ratios),
                                         max_paired_seconds_ratio=max(ratios))
    root = Path(__file__).resolve().parents[1]
    candidate = 'SharedFrequencySevenSaw' if variant == 'shared-frequency-v1' else 'CachedSevenSaw'
    sources = ('src/dsp/SevenSaw.h', 'src/dsp/SevenSaw.cpp', f'experiments/oscillator/{candidate}.h',
               f'experiments/oscillator/{candidate}.cpp', 'experiments/oscillator/benchmark_frequency_cache.cpp',
               'tests/oscillator_frequency_cache.cpp', 'third_party/DaisySP/Source/Synthesis/oscillator.h',
               'third_party/DaisySP/Source/Synthesis/oscillator.cpp', 'third_party/DaisySP/Source/Utility/dsp.h')
    metadata = {'source_sha': source_sha, 'platform': platform.platform(), 'machine': platform.machine(),
                'runner_os': os.environ.get('RUNNER_OS'), 'configuration': 'Release',
                'raw_sha256': hashlib.sha256((folder / 'oscillators.csv').read_bytes()).hexdigest(),
                'source_file_sha256': {name: hashlib.sha256((root / name).read_bytes()).hexdigest() for name in sources},
                'compiler_files_sha256': {str(p.relative_to(folder)): hashlib.sha256(p.read_bytes()).hexdigest()
                                          for p in sorted((folder / 'compiler').glob('*')) if p.is_file()},
                'oscillators': 32, 'voices_represented': 16, 'frames_per_path_and_pair': frames,
                'warmup_seconds': .25, 'native_host_acceptance': False,
                'scope': 'isolated-oscillator-bank', 'candidate_variant': variant,
                'ratio_direction': 'study/reference',
                'timed_work': 'bank processing, pitch setters/sine when modulated, energy accumulation',
                'production_activation': False}
    if extended:
        metadata.update(campaign=campaign, comparison=comparison, pairs_per_cell=pairs,
                        timed_study_type='SevenSaw' if comparison == 'reference-repeat' else 'SharedFrequencySevenSaw',
                        elapsed_seconds_range=[min(x[0] for x in samples.values()), max(x[0] for x in samples.values())],
                        noise_correction_applied=False, background_load_controlled=False)
    decision = qualification(summaries)
    payload = {'schema_version': 3 if extended else 2, 'metadata': metadata, 'summary': summaries,
               'qualification': decision}
    (folder / 'report.json').write_text(json.dumps(payload, indent=2, allow_nan=False) + '\n', encoding='utf-8')
    title = 'frequency sharing' if variant == 'shared-frequency-v1' else 'frequency cache'
    if comparison == 'reference-repeat':
        title = 'unchanged reference repeat'
    lines = [f'# Isolated oscillator {title} study', '', f'Source: {source_sha}',
             '32 oscillator banks represent OSC1 + OSC2 for 16 voices; no engine, filter or FX timing.',
             f'{pairs} alternating pairs of {frames} frames. Ratios are study/reference; smaller is favorable.',
             'Static and audio-rate pitch modulation are separate controls. No native realtime acceptance.', '',
             'CPU acceptance: not established. Production promotion: not allowed by this isolated study.',
             f"Median slowdown cells: {len(decision['median_slowdown_cells'])}/24; "
             f"all {pairs} pairs slower: {len(decision['all_pairs_slowdown_cells'])}/24.",
             'These are observations, not statistical significance; green CI validates the study only.', '',
             '| Rate | Waveform | Pitch modulation | Median paired time ratio |', '| --- | --- | --- | --- |']
    for row in summaries:
        lines.append(f"| {row['rate']} | {row['waveform']} | {row['modulated']} | {row['median_paired_seconds_ratio']:.6f} |")
    if extended:
        lines += ['', f'Campaign: {campaign}; comparison: {comparison}.',
                  'Reference-repeat runs the identical SevenSaw function on both paths; it is a timing control, not a candidate gain.',
                  'All raw observations and ratio ranges are retained. No baseline subtraction, outlier exclusion or production acceptance.',
                  'Longer measurements reduce short-timer sensitivity; runner scheduling and background load remain uncontrolled.']
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
    parser.add_argument('--variant', choices=('held-saw-tuning-v2', 'shared-frequency-v1'),
                        default='held-saw-tuning-v2')
    parser.add_argument('--campaign', choices=('short-v1', 'extended-v1'), default='short-v1')
    parser.add_argument('--comparison', choices=('candidate', 'reference-repeat'), default='candidate')
    args = parser.parse_args()
    report(args.folder, args.source_sha, args.emit_ci, args.variant, args.campaign, args.comparison)
