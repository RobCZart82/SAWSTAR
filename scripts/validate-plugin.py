#!/usr/bin/env python3
"""Build the pinned Steinberg validator and test the actual VST3 bundle."""
from pathlib import Path
import subprocess
import os
root = Path(__file__).resolve().parents[1]
sdk = root / "third_party/iPlug2/Dependencies/IPlug/VST3_SDK"
build = root / "build-validator"
subprocess.run(["cmake", "-S", str(sdk), "-B", str(build),
                "-DCMAKE_BUILD_TYPE=Release", "-DSMTG_ENABLE_VSTGUI_SUPPORT=OFF",
                "-DSMTG_ENABLE_VST3_PLUGIN_EXAMPLES=OFF"], check=True)
subprocess.run(["cmake", "--build", str(build), "--config", "Release",
                "--target", "validator", "--parallel", "3"], check=True)
name = "validator.exe" if os.name == "nt" else "validator"
executables = [p for p in build.rglob(name) if p.is_file() and "Debug" not in p.parts]
if len(executables) != 1:
    raise SystemExit(f"Expected one validator executable, found {executables}")
subprocess.run([str(executables[0]), str(root / "build-plugin/out/SAWSTAR.vst3")], check=True)
