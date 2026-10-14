#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Read-only Windows/Apple Silicon target comparison; never grants CPU acceptance."""
import argparse
import importlib.util
import json
from pathlib import Path

spec = importlib.util.spec_from_file_location('target_archive_review',
    Path(__file__).resolve().parent / 'review-dispatch-engine-target.py')
reviewer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reviewer)


def compare(windows, macos):
    paths = [Path(windows).resolve(), Path(macos).resolve()]
    reviewer.require(paths[0] != paths[1], 'Two distinct target archives are required')
    # Nothing is returned or rendered until BOTH complete archives pass raw validation.
    results = [reviewer.review(path, verify_raw=True) for path in paths]
    manifests = [reviewer.document(path / 'manifest.json') for path in paths]
    win, mac = manifests
    for manifest in manifests:
        reviewer.require(all(isinstance(manifest[key], str) and manifest[key]
                             for key in ('platform', 'machine')), 'Invalid target platform/machine')
    reviewer.require(win['platform'].startswith('Windows-') and win['machine'].lower() in ('amd64', 'x86_64'),
                     'First archive must be Windows x64')
    reviewer.require(mac['platform'].lower().startswith(('macos-', 'darwin-')) and
                     mac['machine'].lower() in ('arm64', 'aarch64'),
                     'Second archive must be Apple Silicon macOS')
    for key in ('source_sha', 'source_file_sha256', 'rounds_requested', 'study_kind',
                'candidate_variant', 'configuration'):
        reviewer.require(win[key] == mac[key], 'Target archives disagree: ' + key)
    for path, manifest in zip(paths, manifests):
        for run in manifest['runs']:
            meta = reviewer.document(path / run['folder'] / 'metadata.json')
            reviewer.require((meta['platform'], meta['machine']) ==
                             (manifest['platform'], manifest['machine']),
                             'Grid platform/machine disagrees with target manifest')
    keys = ('round', 'comparison', 'workload', 'waveform', 'rate', 'buffer')
    reviewer.require([tuple(row[k] for k in keys) for row in results[0]['rows']] ==
                     [tuple(row[k] for k in keys) for row in results[1]['rows']],
                     'Target scene sequences disagree')
    return dict(source_sha=win['source_sha'], ratio_direction='study/reference',
                raw_measurements_verified=True, native_host_acceptance=False,
                production_promotion_allowed=False,
                targets=[dict(label=label, **result)
                         for label, result in zip(('Windows x64', 'macOS Apple Silicon'), results)])


def render(result):
    lines = ['# Windows / Apple Silicon dispatch target comparison', '',
             'Source: ' + result['source_sha'], '',
             'Both complete archives passed checksum and raw CSV validation.',
             'Ratios are study/reference within each target; they do not compare absolute hardware speed.',
             'Reference-repeat controls and candidate results remain separate, including every round.',
             'No pooling, control subtraction, outlier removal, CPU acceptance or production promotion.',
             'Platform labels describe archived evidence, not independently authenticated hardware.', '']
    for target in result['targets']:
        lines.extend(['## ' + target['label'], '', reviewer.render(target)])
    return '\n'.join(lines) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('windows', type=Path)
    parser.add_argument('macos', type=Path)
    parser.add_argument('--format', choices=('markdown', 'json'), default='markdown')
    args = parser.parse_args()
    try:
        result = compare(args.windows, args.macos)
        print(json.dumps(result, indent=2, allow_nan=False) if args.format == 'json' else render(result),
              end='\n' if args.format == 'json' else '')
    except (ValueError, OSError, KeyError, TypeError) as error:
        parser.exit(1, f'Target comparison failed: {error}\n')


if __name__ == '__main__':
    main()
