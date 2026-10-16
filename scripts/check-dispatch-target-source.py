#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Read-only source preflight against a reviewed target summary; never starts timing."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import subprocess


def require(condition, message):
    if not condition:
        raise ValueError(message)


def git(source, *args):
    return subprocess.check_output(['git', '-C', str(source), *args],
                                   text=True, stderr=subprocess.PIPE).strip()


def check(source, reference):
    source = Path(source).resolve()
    reference = Path(reference).resolve()
    # Duplicate keys must not silently replace the recorded source identity.
    def unique(items):
        result = {}
        for key, value in items:
            require(key not in result, 'Duplicate reference key: ' + key)
            result[key] = value
        return result
    reference_bytes = reference.read_bytes()
    evidence = json.loads(reference_bytes.decode('utf-8'), object_pairs_hook=unique)
    require(isinstance(evidence, dict), 'Reference must be a JSON object')
    require(evidence.get('state') == 'complete' and evidence.get('raw_measurements_verified') is True,
            'Reference must be a complete, raw-reviewed target summary')
    require(evidence.get('native_host_acceptance') is False and
            evidence.get('production_promotion_allowed') is False,
            'Unexpected reference acceptance claim')
    for key in ('source_sha', 'DaisySP_sha'):
        require(isinstance(evidence.get(key), str) and
                re.fullmatch('[0-9a-f]{40}', evidence[key]), 'Invalid reference ' + key)
    require(Path(git(source, 'rev-parse', '--show-toplevel')).resolve() == source,
            'Source must be the checkout root')
    sha = git(source, 'rev-parse', 'HEAD')
    require(sha == evidence['source_sha'], 'Source commit differs from the measured reference')
    pinned = git(source, 'ls-tree', 'HEAD', 'third_party/DaisySP').split()
    require(len(pinned) == 4 and pinned[:2] == ['160000', 'commit'] and
            pinned[2] == evidence['DaisySP_sha'], 'DaisySP gitlink differs from reference')
    daisy = source / 'third_party/DaisySP'
    require(Path(git(daisy, 'rev-parse', '--show-toplevel')).resolve() == daisy,
            'Initialize the pinned DaisySP submodule')
    require(git(daisy, 'rev-parse', 'HEAD') == evidence['DaisySP_sha'],
            'DaisySP checkout differs from the pinned commit')
    for folder in (source, daisy):
        require(not git(folder, 'status', '--porcelain', '--untracked-files=all'),
                'Use clean source and DaisySP checkouts, including untracked files')
    expected = evidence.get('source_file_sha256')
    require(isinstance(expected, dict) and bool(expected), 'Missing reference source hashes')
    for name, digest in expected.items():
        part = PurePosixPath(name)
        require(bool(name) and '\\' not in name and not part.is_absolute() and
                '..' not in part.parts and part.as_posix() == name, 'Unsafe source path: ' + name)
        path = source / name
        require(source in path.resolve().parents and path.is_file(), 'Missing/escaping source: ' + name)
        require(isinstance(digest, str) and re.fullmatch('[0-9a-f]{64}', digest),
                'Invalid source digest: ' + name)
        # Hash the actual checkout bytes. Never normalize CRLF or repair a mismatch here.
        require(hashlib.sha256(path.read_bytes()).hexdigest() == digest,
                'Source byte mismatch: ' + name)
    return dict(source_preflight_passed=True, source_sha=sha, DaisySP_sha=evidence['DaisySP_sha'],
                source_files_checked=len(expected), reference_sha256=hashlib.sha256(reference_bytes).hexdigest(),
                measurement_started=False, native_host_acceptance=False, production_promotion_allowed=False,
                limitation='Source consistency only; no hardware, compiler, background-load or CPU acceptance.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--reference', type=Path, required=True)
    args = parser.parse_args()
    try:
        result = check(args.source, args.reference)
    except (ValueError, OSError, KeyError, TypeError, subprocess.CalledProcessError) as error:
        parser.exit(1, f'Source preflight failed: {error}\n')
    print(json.dumps(result, indent=2, allow_nan=False))


if __name__ == '__main__':
    main()
