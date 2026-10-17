#!/usr/bin/env python3
"""Build the pinned Steinberg validator and test the actual VST3 bundle."""
from pathlib import Path
import subprocess
import os
import argparse
import hashlib
import json
root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--bundle', type=Path, default=root / 'build-plugin/out/SAWSTAR.vst3')
parser.add_argument('--module-info-output', type=Path, help='Export Windows x64 factory identity and binary-bound provenance after validation')
args = parser.parse_args()
if args.module_info_output and (os.name != 'nt' or os.environ.get('SAWSTAR_WINDOWS_ARCH') != 'x64'):
    parser.error('--module-info-output currently requires a Windows x64 validator')
bundle = args.bundle.resolve()
if not bundle.is_dir():
    raise SystemExit(f'Missing plugin bundle: {bundle}')
sdk = root / "third_party/iPlug2/Dependencies/IPlug/VST3_SDK"
build = root / "build-validator"
extra=[]
if os.name=="nt" and os.environ.get("SAWSTAR_WINDOWS_ARCH"):
    extra += ["-A",os.environ["SAWSTAR_WINDOWS_ARCH"]]
if os.environ.get("SAWSTAR_MAC_ARCHS"):
    extra += ["-DCMAKE_OSX_ARCHITECTURES="+os.environ["SAWSTAR_MAC_ARCHS"],"-DCMAKE_OSX_DEPLOYMENT_TARGET=11.0"]
subprocess.run(["cmake", "-S", str(sdk), "-B", str(build),
                "-DCMAKE_BUILD_TYPE=Release", "-DSMTG_ENABLE_VSTGUI_SUPPORT=OFF",
                "-DSMTG_ENABLE_VST3_PLUGIN_EXAMPLES=OFF"]+extra, check=True)
subprocess.run(["cmake", "--build", str(build), "--config", "Release",
                "--target", "validator", "--parallel", "3"], check=True)
name = "validator.exe" if os.name == "nt" else "validator"
executables = [p for p in build.rglob(name) if p.is_file() and "Debug" not in p.parts]
if len(executables) != 1:
    raise SystemExit(f"Expected one validator executable, found {executables}")
if args.module_info_output:
    binaries = list((bundle / 'Contents/x86_64-win').glob('*.vst3'))
    if len(binaries) != 1:
        raise SystemExit('Expected one Windows x64 VST3 executable')
    binary = binaries[0]
    before = hashlib.sha256(binary.read_bytes()).hexdigest()
subprocess.run([str(executables[0]), str(bundle)], check=True)

# Exercise both slices of a Universal validator/plugin on Apple Silicon runners.
# This is translated x86_64 validation, not a substitute for native Intel host QA.
if os.name != 'nt' and 'x86_64' in os.environ.get('SAWSTAR_MAC_ARCHS',''):
    import platform
    if platform.machine() == 'arm64':
        subprocess.run(['arch','-x86_64',str(executables[0]),str(bundle)],check=True)

# The SDK loader reads the actual factory; a directory rename is not identity proof.
if args.module_info_output:
    subprocess.run(['cmake', '--build', str(build), '--config', 'Release',
                    '--target', 'moduleinfotool', '--parallel', '3'], check=True)
    tools = [p for p in build.rglob('moduleinfotool.exe') if p.is_file() and 'Debug' not in p.parts]
    if len(tools) != 1:
        raise SystemExit('Expected one pinned SDK moduleinfotool')
    output = args.module_info_output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    if output.exists() or Path(str(output) + '.provenance.json').exists():
        raise SystemExit('Refusing to overwrite previous identity evidence')
    subprocess.run([str(tools[0]), '-create', '-version',
                    json.loads((root / 'release.json').read_text())['version'],
                    # The pinned Win32 Module::create derives name using '/' only.
                    # Preserve strict package identity checks; pass a portable SDK path.
                    '-path', bundle.as_posix(), '-output', str(output)], check=True)
    if hashlib.sha256(binary.read_bytes()).hexdigest() != before:
        raise SystemExit('Plugin changed during identity inspection')
    def revision(path):
        return subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=path, text=True).strip()
    proof = dict(schema=1, source_commit=revision(root), sdk_commit=revision(sdk),
                 binary_relative_path=binary.relative_to(bundle).as_posix(), binary_sha256=before,
                 module_info_sha256=hashlib.sha256(output.read_bytes()).hexdigest(),
                 module_info_tool_sha256=hashlib.sha256(tools[0].read_bytes()).hexdigest(),
                 validator_sha256=hashlib.sha256(executables[0].read_bytes()).hexdigest(),
                 validator_passed=True, native_host_acceptance=False, production_promotion_allowed=False)
    Path(str(output) + '.provenance.json').write_text(json.dumps(proof, indent=2)+'\n', encoding='utf-8')
