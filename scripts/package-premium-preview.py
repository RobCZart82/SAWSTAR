#!/usr/bin/env python3
"""Package the separate experimental preview; never publish or install it."""
import argparse
import hashlib
import json
from pathlib import Path
import plistlib
import re
import struct
import subprocess
import zipfile

ROOT = Path(__file__).resolve().parents[1]


IDENTITIES = {
    'reference': ('SAWSTAR Premium Preview', 'SAWSTARPremiumPreview', 'SwPP'),
    'candidate': ('SAWSTAR Premium Candidate', 'SAWSTARPremiumCandidate', 'SwPC'),
}


# The pinned SDK JSON5Writer emits quoted JSON with trailing commas. Preserve
# its raw bytes as evidence; remove only structural trailing commas for parsing.
def read_module_info(data):
    text = data.decode('utf-8')
    text = re.sub(r'"(?:\\.|[^"\\])*"|,\s*(?=[}\]])',
                  lambda m: m.group(0) if m.group(0).startswith('"') else '', text)
    return json.loads(text)


def windows_identity_bytes(data, binary_relative_path, stem, product, signature,
                           module_info_data, proof, revision):
    """Validate SDK identity/provenance against bytes, without loading the DLL."""
    if len(data) < 64 or data[:2] != b'MZ':
        raise ValueError('Expected a Windows PE image')
    pe = struct.unpack_from('<I', data, 60)[0]
    if (pe + 26 > len(data) or data[pe:pe+4] != b'PE\0\0' or
            struct.unpack_from('<H', data, pe+4)[0] != 0x8664 or
            struct.unpack_from('<H', data, pe+24)[0] != 0x20b):
        raise ValueError('Expected a Windows x64 PE32+ image')
    info = read_module_info(module_info_data)
    digest = hashlib.sha256(data).hexdigest()
    if (proof.get('schema') != 1 or proof.get('source_commit') != revision or
            proof.get('binary_sha256') != digest or
            proof.get('binary_relative_path') != binary_relative_path or
            proof.get('module_info_sha256') != hashlib.sha256(module_info_data).hexdigest() or
            proof.get('sdk_commit') != '3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96' or
            proof.get('validator_passed') is not True or
            not re.fullmatch(r'[0-9a-f]{64}', proof.get('module_info_tool_sha256', '')) or
            not re.fullmatch(r'[0-9a-f]{64}', proof.get('validator_sha256', '')) or
            proof.get('native_host_acceptance') is not False or
            proof.get('production_promotion_allowed') is not False):
        raise ValueError('Windows module info provenance does not match this validated binary/source')
    # Pinned iPlug2 VST3_PROCESSOR_UID / Steinberg FUID::toString canonical order.
    cid = 'F2AEE70D00DE4F4E52433832' + signature.encode('ascii').hex().upper()
    classes = info.get('Classes', [])
    if (info.get('Name') != stem or len(classes) != 1 or
            classes[0].get('CID') != cid or classes[0].get('Name') != product or
            classes[0].get('Category') != 'Audio Module Class' or
            classes[0].get('Vendor') != 'RobCZart82'):
        raise ValueError('Windows VST3 factory identity mismatch')
    return cid


def windows_identity(bundle, stem, product, signature, module_info, revision):
    binary = bundle / ('Contents/x86_64-win/' + stem + '.vst3')
    if not binary.is_file():
        raise ValueError('Missing Windows x64 preview executable')
    if module_info is None:
        raise ValueError('Windows packaging requires fresh SDK module info and provenance')
    module_info = Path(module_info)
    provenance = Path(str(module_info) + '.provenance.json')
    cid = windows_identity_bytes(binary.read_bytes(), binary.relative_to(bundle).as_posix(),
                                 stem, product, signature, module_info.read_bytes(),
                                 json.loads(provenance.read_bytes()), revision)
    return binary, cid, {'evidence/moduleinfo.json': module_info,
                         'evidence/moduleinfo.provenance.json': provenance}


def package(bundle, output, role='reference', platform='macos', module_info=None):
    if role not in IDENTITIES:
        raise ValueError('Unknown native pair role')
    product, stem, signature = IDENTITIES[role]
    bundle, output = Path(bundle), Path(output)
    if bundle.name != stem + '.vst3' or not bundle.is_dir():
        raise ValueError('Expected the separate ' + stem + '.vst3 bundle')
    revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip()
    evidence = {}
    cid = None
    bundle_identifier = None
    if platform == 'windows-x64':
        binary, cid, evidence = windows_identity(bundle, stem, product, signature, module_info, revision)
    elif platform == 'macos':
        info = plistlib.loads((bundle / 'Contents/Info.plist').read_bytes())
        if (info.get('CFBundleIdentifier'), info.get('CFBundleSignature'),
                info.get('CFBundleExecutable'), info.get('CFBundleName')) != (
                'io.github.robczart82.' + stem.lower(), signature, stem, stem):
            raise ValueError('Preview identity mismatch; refusing to mislabel the native pair')
        bundle_identifier = info['CFBundleIdentifier']
        binary = bundle / ('Contents/MacOS/' + stem)
        if not binary.is_file():
            raise ValueError('Missing preview executable')
    else:
        raise ValueError('Unsupported preview package platform')
    files = {}
    for path in sorted(bundle.rglob('*')):
        if path.is_symlink():
            raise ValueError('Unexpected bundle symlink')
        if path.is_file(): files[bundle.name + '/' + path.relative_to(bundle).as_posix()] = path
    files.update(evidence)
    for path in sorted((ROOT / 'third_party/licenses').glob('*')):
        if path.is_file(): files['licenses/' + path.name] = path
    files['LICENSE'] = ROOT / 'LICENSE'
    files['README-PREMIUM-PREVIEW.md'] = ROOT / ('docs/PREMIUM_PREVIEW_WINDOWS.md' if platform == 'windows-x64' else 'docs/PREMIUM_PREVIEW.md')
    files['NATIVE_PREVIEW_PAIR_QUALIFICATION.md'] = ROOT / 'docs/NATIVE_PREVIEW_PAIR_QUALIFICATION.md'
    files['PREMIUM_PREVIEW_LEVELS.md'] = ROOT / 'docs/PREMIUM_PREVIEW_LEVELS.md'
    files['measurements/premium-preview-levels-2026-10-15-local-arm64.csv'] = ROOT / 'docs/measurements/premium-preview-levels-2026-10-15-local-arm64.csv'
    files['licenses/SAWSTAR-BRANDING-LICENSE.txt'] = ROOT / 'assets/branding/LICENSE.txt'
    notice = (ROOT / 'THIRD_PARTY_NOTICES.md').read_text(encoding='utf-8').replace('third_party/licenses/', 'licenses/').encode('utf-8')
    hashes = {name: hashlib.sha256(path.read_bytes()).hexdigest() for name, path in files.items()}
    hashes['THIRD_PARTY_NOTICES.md'] = hashlib.sha256(notice).hexdigest()
    manifest = dict(product=product, pair_role=role, experimental=True, source_commit=revision,
                    platform=platform, bundle_identifier=bundle_identifier,
                    vst3_class_id=cid, plugin_unique_id=signature,
                    oscillator='SawDispatchSevenSaw' if role == 'candidate' else 'SevenSaw',
                    binary_sha256=hashlib.sha256(binary.read_bytes()).hexdigest(),
                    native_host_acceptance=False,
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
    parser.add_argument('--role', choices=IDENTITIES, default='reference')
    parser.add_argument('--platform', choices=['macos', 'windows-x64'], default='macos')
    parser.add_argument('--module-info', type=Path)
    args = parser.parse_args()
    print(package(args.bundle, args.output, role=args.role, platform=args.platform, module_info=args.module_info))

