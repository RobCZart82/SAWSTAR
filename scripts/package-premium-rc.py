#!/usr/bin/env python3
"""Validate and package an explicit Premium integration RC, never publish it."""
import argparse
import hashlib
import importlib.util
import json
import posixpath
from pathlib import Path
import re
import subprocess
import sys
import tempfile
from urllib.parse import unquote, urlsplit
import zipfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'scripts'))
from rc_provenance import checkout_provenance

MARKDOWN_LINK = re.compile(r'!?\[[^\]]*\]\(\s*(?:<([^>]+)>|([^\s)]+))(?:\s+[^)]*)?\)')

def validate_markdown_payload_links(payloads):
    """Reject relative Markdown links whose targets are absent from the RC ZIP."""
    names = set(payloads)
    missing = []
    for name, data in payloads.items():
        if not name.lower().endswith('.md'):
            continue
        text = data.decode('utf-8')
        for match in MARKDOWN_LINK.finditer(text):
            target = match.group(1) or match.group(2)
            parsed = urlsplit(target)
            if parsed.scheme or parsed.netloc or not parsed.path:
                continue
            path = unquote(parsed.path)
            resolved = posixpath.normpath(posixpath.join(posixpath.dirname(name), path))
            if resolved == '..' or resolved.startswith('../') or (
                    resolved not in names and
                    not any(member.startswith(resolved.rstrip('/') + '/') for member in names)):
                missing.append(f'{name} -> {target}')
    if missing:
        raise ValueError('Broken relative Markdown link(s) in RC package: ' + '; '.join(missing))

def integration_cache(path):
    values = {}
    for line in Path(path).read_text().splitlines():
        if '=' in line and ':' in line.split('=', 1)[0]:
            key, value = line.split('=', 1)
            values[key.split(':', 1)[0]] = value
    if values.get('SAWSTAR_PREMIUM_PRODUCTION') != 'ON':
        raise ValueError('Expected an explicit Premium production integration build')
    if values.get('SAWSTAR_BUILD_PLUGIN') != 'ON':
        raise ValueError('Expected a real VST3 plugin build')
    for key in ('SAWSTAR_PREMIUM_PREVIEW', 'SAWSTAR_PREMIUM_NATIVE_CANDIDATE'):
        if values.get(key, 'OFF') != 'OFF':
            raise ValueError('RC must retain the production SAWSTAR identity')
    if Path(values.get('CMAKE_HOME_DIRECTORY', '')).resolve() != ROOT:
        raise ValueError('Build source directory does not match this checkout')
    return values

def finalize_rc_archive(output, manifest):
    # Rebuild once so there are no duplicate manifest members and every
    # payload (including this RC supplement) is covered by the hash inventory.
    with zipfile.ZipFile(output) as archive:
        payloads = {name: archive.read(name) for name in archive.namelist()}
    package_manifest = json.loads(payloads.pop('PACKAGE-MANIFEST.json'))
    payloads['PREMIUM-RC-MANIFEST.json'] = (json.dumps(manifest, indent=2) + '\n').encode()
    payloads['docs/PREMIUM_PRODUCTION_RC.md'] = (ROOT / 'docs/PREMIUM_PRODUCTION_RC.md').read_bytes()
    validate_markdown_payload_links(payloads)
    package_manifest['files_sha256'] = {name: hashlib.sha256(data).hexdigest()
                                        for name, data in payloads.items()}
    temporary = output.with_suffix(output.suffix + '.tmp')
    with zipfile.ZipFile(temporary, 'w', zipfile.ZIP_DEFLATED) as archive:
        for name, data in payloads.items():
            archive.writestr(name, data)
        archive.writestr('PACKAGE-MANIFEST.json', json.dumps(package_manifest, indent=2) + '\n')
    temporary.replace(output)
    digest = hashlib.sha256(output.read_bytes()).hexdigest()
    output.with_suffix(output.suffix + '.sha256').write_text(f'{digest}  {output.name}\n')

def package_rc_atomically(package, bundle_parent, output, manifest):
    """Expose final archive names only after the complete RC validates."""
    output.parent.mkdir(parents=True, exist_ok=True)
    checksum = output.with_suffix(output.suffix + '.sha256')
    if output.exists() or checksum.exists():
        raise ValueError('Refusing to overwrite previous RC evidence')
    with tempfile.TemporaryDirectory(prefix='.sawstar-rc-stage-', dir=output.parent) as folder:
        staged = Path(folder) / output.name
        package(bundle_parent, staged)
        finalize_rc_archive(staged, manifest)
        staged_checksum = staged.with_suffix(staged.suffix + '.sha256')
        # Publish the ZIP last; upload steps must never find a rejected or
        # partially finalized archive under its deliverable filename.
        try:
            staged_checksum.replace(checksum)
            staged.replace(output)
        except OSError:
            checksum.unlink(missing_ok=True)
            output.unlink(missing_ok=True)
            raise


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('build_dir', type=Path)
    p.add_argument('output', type=Path)
    args = p.parse_args()
    build = args.build_dir.resolve()
    integration_cache(build / 'CMakeCache.txt')
    meta = json.loads((ROOT / 'release.json').read_text())
    if not meta['candidate'].startswith('rc') or meta.get('release_date'):
        raise ValueError('RC packaging requires candidate metadata and no release date')
    provenance = checkout_provenance(ROOT)
    bundle = build / 'out/SAWSTAR.vst3'
    subprocess.run([sys.executable, str(ROOT / 'scripts/validate-plugin.py'),
                    '--bundle', str(bundle)], check=True)
    spec = importlib.util.spec_from_file_location('rc_package', ROOT / 'scripts/package-plugin.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    output = args.output.resolve()
    revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip()
    dirty = bool(subprocess.check_output(['git', 'status', '--porcelain', '--untracked-files=no'], cwd=ROOT, text=True).strip())
    manifest = dict(schema=1, version=meta['version'], candidate=meta['candidate'],
                    source_commit=revision, source_dirty=dirty, source_provenance=provenance,
                    engine='calibrated-premium-lookup-reference', latency_samples=32,
                    plugin_name='SAWSTAR', replaces_classic_identity=True,
                    validator_passed=True, native_host_acceptance=False,
                    factory_listening_acceptance=False, publication_allowed=False,
                    build_cache_sha256=hashlib.sha256((build / 'CMakeCache.txt').read_bytes()).hexdigest())
    package_rc_atomically(module.package, bundle.parent, output, manifest)
    print(output)

if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError) as error:
        raise SystemExit(str(error))
