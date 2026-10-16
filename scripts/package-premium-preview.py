#!/usr/bin/env python3
"""Package the separate experimental preview; never publish or install it."""
import argparse
import hashlib
import json
from pathlib import Path
import plistlib
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parents[1]


def package(bundle, output):
    bundle, output = Path(bundle), Path(output)
    if bundle.name != 'SAWSTARPremiumPreview.vst3' or not bundle.is_dir():
        raise ValueError('Expected the separate SAWSTARPremiumPreview.vst3 bundle')
    info = plistlib.loads((bundle / 'Contents/Info.plist').read_bytes())
    if (info['CFBundleIdentifier'], info['CFBundleSignature']) != (
            'io.github.robczart82.sawstarpremiumpreview', 'SwPP'):
        raise ValueError('Preview identity mismatch; refusing to package release identity')
    revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip()
    files = {}
    for path in sorted(bundle.rglob('*')):
        if path.is_symlink():
            raise ValueError('Unexpected bundle symlink')
        if path.is_file(): files[bundle.name + '/' + path.relative_to(bundle).as_posix()] = path
    if not (bundle / 'Contents/MacOS/SAWSTARPremiumPreview').is_file():
        raise ValueError('Missing preview executable')
    for path in sorted((ROOT / 'third_party/licenses').glob('*')):
        if path.is_file(): files['licenses/' + path.name] = path
    files['LICENSE'] = ROOT / 'LICENSE'
    files['README-PREMIUM-PREVIEW.md'] = ROOT / 'docs/PREMIUM_PREVIEW.md'
    files['PREMIUM_PREVIEW_LEVELS.md'] = ROOT / 'docs/PREMIUM_PREVIEW_LEVELS.md'
    files['measurements/premium-preview-levels-2026-10-15-local-arm64.csv'] = ROOT / 'docs/measurements/premium-preview-levels-2026-10-15-local-arm64.csv'
    files['licenses/SAWSTAR-BRANDING-LICENSE.txt'] = ROOT / 'assets/branding/LICENSE.txt'
    notice = (ROOT / 'THIRD_PARTY_NOTICES.md').read_text(encoding='utf-8').replace('third_party/licenses/', 'licenses/').encode('utf-8')
    hashes = {name: hashlib.sha256(path.read_bytes()).hexdigest() for name, path in files.items()}
    hashes['THIRD_PARTY_NOTICES.md'] = hashlib.sha256(notice).hexdigest()
    manifest = dict(product='SAWSTAR Premium Preview', experimental=True, source_commit=revision,
                    engine='RateScaledLookupEnginePremiumFilter', latency_samples=32,
                    level_policy='wet-output-3db-factory-trim-v1',
                    production_promotion_allowed=False, files_sha256=hashes)
    output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED) as archive:
        for name, path in files.items(): archive.write(path, name)
        archive.writestr('THIRD_PARTY_NOTICES.md', notice)
        archive.writestr('PACKAGE-MANIFEST.json', json.dumps(manifest, indent=2) + '\n')
    digest = hashlib.sha256(output.read_bytes()).hexdigest()
    output.with_suffix(output.suffix + '.sha256').write_text(f'{digest}  {output.name}\n')
    return output


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('bundle', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    print(package(args.bundle, args.output))
