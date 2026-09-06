#!/usr/bin/env python3
"""Fetch only the pinned iPlug2 and MIT VST3 SDK components needed by SAWSTAR."""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SDK_COMMIT = "3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96"
SDK = ROOT / "third_party/iPlug2/Dependencies/IPlug/VST3_SDK"

def git(*args, cwd=ROOT):
    return subprocess.check_output(["git", *args], cwd=cwd, text=True).strip()

git("submodule", "update", "--init", "third_party/iPlug2")
if not SDK.exists():
    SDK.mkdir(parents=True)
    git("init", cwd=SDK)
    git("remote", "add", "origin", "https://github.com/steinbergmedia/vst3sdk.git", cwd=SDK)
if not (SDK / ".git").exists():
    raise SystemExit(f"Refusing to modify an unmanaged SDK directory: {SDK}")
if git("status", "--porcelain", cwd=SDK):
    raise SystemExit("SDK has local changes; preserve them before updating.")
git("fetch", "--depth", "1", "origin", SDK_COMMIT, cwd=SDK)
git("checkout", "--detach", SDK_COMMIT, cwd=SDK)
# Each child revision is pinned by the SDK commit; no VSTGUI or optional SDKs.
git("submodule", "update", "--init", "--depth", "1", "base", "pluginterfaces", "public.sdk", "cmake", cwd=SDK)
print(f"VST3 SDK ready at {git('rev-parse', 'HEAD', cwd=SDK)}")
