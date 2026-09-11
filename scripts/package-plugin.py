#!/usr/bin/env python3
"""Package a VST3 bundle with user documentation, notices and integrity data."""
from pathlib import Path
import hashlib
import json
import re
import subprocess
import sys
import zipfile

root = Path(__file__).resolve().parents[1]

def package(bundle_parent, output):
    bundle = Path(bundle_parent) / 'SAWSTAR.vst3'
    if not bundle.is_dir():
        raise ValueError(f'Missing plugin bundle: {bundle}')
    files = {}
    for p in sorted(bundle.rglob('*')):
        if p.is_symlink():
            raise ValueError(f'Bundle symlink requires explicit packaging support: {p}')
        if p.is_file():
            files['SAWSTAR.vst3/' + p.relative_to(bundle).as_posix()] = p
    if not files:
        raise ValueError('Empty plugin bundle')
    for p in sorted((root / 'third_party/licenses').rglob('*')):
        if p.is_file():
            files['licenses/' + p.relative_to(root / 'third_party/licenses').as_posix()] = p
    for name in ['LICENSE', 'THIRD_PARTY_NOTICES.md', 'docs/INSTALLATION.md',
                 'docs/RELEASE_NOTES_1.0.0.md', 'docs/manuals/README.md',
                 'docs/manuals/SAWSTAR-User-Manual-EN.pdf',
                 'docs/manuals/SAWSTAR-User-Manual-HU.pdf']:
        p = root / name
        if not p.is_file():
            raise ValueError(f'Missing distribution document: {name}')
        files[name if name.startswith('docs/manuals/') else p.name] = p
    version = re.search(r'project\(SAWSTAR VERSION ([0-9.]+)',
                        (root / 'CMakeLists.txt').read_text()).group(1)
    try:
        revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root,
                                           stderr=subprocess.DEVNULL, text=True).strip()
    except (OSError, subprocess.CalledProcessError):
        revision = 'unavailable'
    hashes = {name: hashlib.sha256(p.read_bytes()).hexdigest() for name, p in files.items()}
    manifest = {'version': version, 'source_commit': revision,
                'note': 'Source version; verify binary About metadata before publication.',
                'files_sha256': hashes}
    output = Path(output)
    with zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED) as archive:
        for name, p in files.items():
            archive.write(p, name)
        archive.writestr('PACKAGE-MANIFEST.json', json.dumps(manifest, indent=2) + '\n')
    digest = hashlib.sha256(output.read_bytes()).hexdigest()
    output.with_suffix(output.suffix + '.sha256').write_text(f'{digest}  {output.name}\n')
    return output

if __name__ == '__main__':
    if len(sys.argv) != 3:
        raise SystemExit('Usage: package-plugin.py BUNDLE_PARENT OUTPUT.zip')
    try:
        print(package(sys.argv[1], sys.argv[2]))
    except ValueError as error:
        raise SystemExit(str(error))
