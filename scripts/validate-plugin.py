#!/usr/bin/env python3
"""Build the pinned Steinberg validator and test the actual VST3 bundle."""
from pathlib import Path
import subprocess
import os
import argparse
root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--bundle', type=Path, default=root / 'build-plugin/out/SAWSTAR.vst3')
args = parser.parse_args()
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
subprocess.run([str(executables[0]), str(bundle)], check=True)

# Exercise both slices of a Universal validator/plugin on Apple Silicon runners.
# This is translated x86_64 validation, not a substitute for native Intel host QA.
if os.name != 'nt' and 'x86_64' in os.environ.get('SAWSTAR_MAC_ARCHS',''):
    import platform
    if platform.machine() == 'arm64':
        subprocess.run(['arch','-x86_64',str(executables[0]),str(bundle)],check=True)
