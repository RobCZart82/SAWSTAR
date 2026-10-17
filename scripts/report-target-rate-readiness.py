#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Summarize the frozen M1/Windows evidence; never measure or grant acceptance."""
import argparse
import hashlib
import itertools
import json
import math
from pathlib import Path

SOURCE = '2f401939db95c4da51e80d041e8911d0258f3b39'
DAISY = '599511b740f8f3a9b8db72a0642aa45b8a23c3a3'
ROOT = Path(__file__).resolve().parents[1]
INPUTS = {
    'M1': ROOT / 'docs/measurements/mac-target-arm64-2026-10-16.json',
    'Windows': ROOT / 'docs/measurements/windows-target-x64-2026-10-16.json',
}
EXPECTED = set(itertools.product((1, 2), ('reference-repeat', 'candidate'),
    ('stationary-v1', 'modulated-v1'), range(4), (48000, 96000, 192000), (32, 64, 128)))
KEYS = ('round', 'comparison', 'workload', 'waveform', 'rate', 'buffer')


def require(ok, message):
    if not ok:
        raise ValueError(message)


def load(path):
    def pairs(items):
        result = {}
        for key, value in items:
            require(key not in result, 'Duplicate JSON key: ' + key)
            result[key] = value
        return result
    raw = Path(path).read_bytes()
    data = json.loads(raw, object_pairs_hook=pairs,
        parse_constant=lambda value: (_ for _ in ()).throw(ValueError('Nonfinite JSON: ' + value)))
    return data, hashlib.sha256(raw).hexdigest()


def validate(data):
    require(data['source_sha'] == SOURCE, 'Unexpected frozen source')
    require(data.get('DaisySP_sha', data.get('daisysp_sha')) == DAISY, 'Unexpected DaisySP')
    for name in ('DaisySP_sha', 'daisysp_sha'):
        if name in data:
            require(data[name] == DAISY, 'Conflicting DaisySP identity')
    require(type(data['schema_version']) is int and data['schema_version'] == 1
            and data['state'] == 'complete', 'Incomplete or unknown evidence')
    for name, expected in (('grids', 32), ('cells', 288), ('total_raw_blocks', 2359296), ('verifier_exit_code', 0)):
        require(type(data[name]) is int and data[name] == expected, 'Unexpected ' + name)
    require(data['raw_measurements_verified'] is True, 'Missing original raw review')
    require(data['native_host_acceptance'] is False and data['production_promotion_allowed'] is False,
            'Frozen evidence must not claim native or production acceptance')
    hashes = data['source_file_sha256']
    require(isinstance(hashes, dict) and len(hashes) == 20, 'Expected 20 source hashes')
    for path, digest in hashes.items():
        require(isinstance(path, str) and isinstance(digest, str) and len(digest) == 64
                and all(c in '0123456789abcdef' for c in digest), 'Invalid source digest')
    rows = data.get('paired_results', data.get('rows'))
    require(isinstance(rows, list) and len(rows) == len(EXPECTED), 'Expected all 288 cells')
    seen = set()
    for row in rows:
        for name in ('round', 'waveform', 'rate', 'buffer', 'paired_observations', 'blocks_per_path',
                     'reference_over_budget_blocks', 'study_over_budget_blocks'):
            require(type(row[name]) is int, 'Expected integer ' + name)
        key = tuple(row[name] for name in KEYS)
        require(key in EXPECTED and key not in seen, 'Unknown or duplicate cell: ' + str(key))
        seen.add(key)
        require(row['paired_observations'] == 16 and row['blocks_per_path'] == 4096,
                'Unexpected paired observation/block count')
        for name in ('reference_over_budget_blocks', 'study_over_budget_blocks'):
            require(0 <= row[name] <= 4096, 'Out-of-range ' + name)
        for name in ('median_paired_p50_ratio', 'median_paired_p99_ratio'):
            value = row[name]
            require(type(value) in (int, float) and math.isfinite(value) and value > 0,
                    'Invalid timing ratio: ' + name)
    require(seen == EXPECTED, 'Missing cells')
    return rows


def group(rows):
    rows = sorted(rows, key=lambda row: (row['workload'], row['waveform']))
    p50 = [r['median_paired_p50_ratio'] for r in rows]
    p99 = [r['median_paired_p99_ratio'] for r in rows]
    worst = max(p99)
    return dict(cells=len(rows), blocks_per_path_total=sum(r['blocks_per_path'] for r in rows),
        p50_ratio_min=min(p50), p50_ratio_max=max(p50),
        p99_ratio_min=min(p99), p99_ratio_max=worst,
        reference_over_budget_blocks=sum(r['reference_over_budget_blocks'] for r in rows),
        study_over_budget_blocks=sum(r['study_over_budget_blocks'] for r in rows),
        study_all_blocks_over_budget_cells=sum(r['study_over_budget_blocks'] == r['blocks_per_path'] for r in rows),
        study_p99_ratio_gt_one_cells=sum(value > 1 for value in p99),
        worst_p99_cells=[dict(workload=r['workload'], waveform=r['waveform'],
            p99_ratio=r['median_paired_p99_ratio'],
            reference_over_budget_blocks=r['reference_over_budget_blocks'],
            study_over_budget_blocks=r['study_over_budget_blocks'])
            for r in rows if r['median_paired_p99_ratio'] == worst])


def report(inputs):
    require(set(inputs) == {'M1', 'Windows'}, 'Both target platforms are required')
    rows_by_platform = {name: validate(data) for name, data in inputs.items()}
    require(isinstance(inputs['M1']['machine'], dict)
            and inputs['M1']['machine']['hardware']['chip_type'] == 'Apple M1', 'Expected M1 machine identity')
    require(inputs['Windows']['machine'] == 'AMD64'
            and inputs['Windows']['platform'].startswith('Windows-'), 'Expected Windows x64 identity')
    require(inputs['M1']['source_file_sha256'] == inputs['Windows']['source_file_sha256'],
            'The 20 source hashes differ between targets')
    result = []
    for platform in ('M1', 'Windows'):
        rows = rows_by_platform[platform]
        for round_, rate, buffer in itertools.product((1, 2), (48000, 96000, 192000), (32, 64, 128)):
            selected = [r for r in rows if (r['round'], r['rate'], r['buffer']) == (round_, rate, buffer)]
            comparisons = {name: group([r for r in selected if r['comparison'] == name])
                           for name in ('reference-repeat', 'candidate')}
            result.append(dict(platform=platform, round=round_, rate=rate, buffer=buffer,
                comparisons=comparisons, native_status='not_accepted', support_policy='undecided'))
    return dict(schema_version=1, source_sha=SOURCE, daisysp_sha=DAISY,
        input_cells=576, rows=result, raw_reverification_performed=False,
        cpu_acceptance=False, native_host_acceptance=False, production_promotion_allowed=False,
        limitations=['Korábban nyersen ellenőrzött összefoglalókat olvas; a nyers blokkokat nem ellenőrzi újra.',
            'Az arány study/reference. A referenciaismétlés külön marad, nincs zajlevonás.',
            'Mindkét kör, minden hullámforma és statikus/modulált legrosszabb eset megmarad.',
            'Az offline időkeret-túllépés nem mért REAPER-dropout és nem REAPER CPU%.',
            'Nincs ráta-/pufferkorlátozás, minőségváltás vagy kiadási elfogadás.'])


def markdown(data):
    lines = ['# Célgépes ráta-/pufferkészenlét — 2026-10-16-i lezárt adatok', '',
        'Forrás: `' + data['source_sha'] + '`. M1 és Windows, minden kör külön.',
        'Minden sor 8 jelölt- és 8 referenciaismétlés-cellát őriz (4 hullámforma × 2 terhelés).',
        'Az időkeret-túllépés számlálója blokkok összege, nem natív dropout; útvonalanként 32 768 blokk/sor.',
        '**Minden sor natív elfogadása nyitott, a támogatási policy nincs eldöntve.**', '',
        '| Gép | Kör | kHz / puffer | Jelölt p50 arány min–max | Jelölt p99 legrosszabb | Jelölt ref / study túllépés | Ref-ismétlés p99 min–max |',
        '| --- | ---: | --- | --- | ---: | ---: | --- |']
    for row in data['rows']:
        c, r = (row['comparisons'][name] for name in ('candidate', 'reference-repeat'))
        lines.append(f"| {row['platform']} | {row['round']} | {row['rate'] // 1000} / {row['buffer']} | "
            f"{c['p50_ratio_min']:.6f}–{c['p50_ratio_max']:.6f} | {c['p99_ratio_max']:.6f} | "
            f"{c['reference_over_budget_blocks']} / {c['study_over_budget_blocks']} | "
            f"{r['p99_ratio_min']:.6f}–{r['p99_ratio_max']:.6f} |")
    lines += ['', '## Korlátok', ''] + ['- ' + item for item in data['limitations']]
    lines += ['', 'A JSON-kimenet külön megőrzi a legrosszabb p99-cellák terhelését, hullámformáját és mindkét túllépésszámlálóját.',
        'Az eredeti 576 cella a bemeneti bizonyítékokban változatlanul megmarad.', '', '## Bemenetek SHA-256', '']
    lines += ['- ' + name + ': `' + value + '`' for name, value in data.get('input_sha256', {}).items()]
    return '\n'.join(lines) + '\n'


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mac', type=Path, default=INPUTS['M1'])
    parser.add_argument('--windows', type=Path, default=INPUTS['Windows'])
    parser.add_argument('--format', choices=('json', 'markdown'), default='json')
    args = parser.parse_args()
    try:
        loaded = {name: load(path) for name, path in (('M1', args.mac), ('Windows', args.windows))}
        result = report({name: value[0] for name, value in loaded.items()})
        result['input_sha256'] = {name: value[1] for name, value in loaded.items()}
        print(markdown(result) if args.format == 'markdown' else json.dumps(result, indent=2, allow_nan=False), end='\n' if args.format == 'json' else '')
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.exit(1, 'Target readiness report rejected: ' + str(error) + '\n')
