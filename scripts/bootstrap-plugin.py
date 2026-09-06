#!/usr/bin/env python3
"""Fetch only the pinned iPlug2 and MIT VST3 SDK components needed by SAWSTAR."""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SDK_COMMIT = "3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96"
SDK = ROOT / "third_party/iPlug2/Dependencies/IPlug/VST3_SDK"

def git(*args, cwd=ROOT):
    return subprocess.check_output(["git", *args], cwd=cwd, text=True).strip()

git("submodule", "update", "--init", "third_party/iPlug2", "third_party/DaisySP")
if not (SDK / ".git").exists():
    # iPlug2 ships an instruction-only placeholder here. Replace only that
    # exact tracked file; refuse to overwrite a user-supplied SDK directory.
    if SDK.exists() and list(SDK.iterdir()):
        expected = git("show", "HEAD:Dependencies/IPlug/VST3_SDK/README.md", cwd=ROOT / "third_party/iPlug2")
        if sorted(p.name for p in SDK.iterdir()) != ["README.md"] or (SDK / "README.md").read_text().strip() != expected:
            raise SystemExit(f"Refusing to modify an unmanaged SDK directory: {SDK}")
        (SDK / "README.md").unlink()
    SDK.mkdir(parents=True, exist_ok=True)
    git("init", cwd=SDK)
    git("remote", "add", "origin", "https://github.com/steinbergmedia/vst3sdk.git", cwd=SDK)
if git("status", "--porcelain", cwd=SDK):
    raise SystemExit("SDK has local changes; preserve them before updating.")
git("fetch", "--depth", "1", "origin", SDK_COMMIT, cwd=SDK)
git("checkout", "--detach", SDK_COMMIT, cwd=SDK)
# Each child revision is pinned by the SDK commit; no VSTGUI or optional SDKs.
git("submodule", "update", "--init", "--depth", "1", "base", "pluginterfaces", "public.sdk", "cmake", cwd=SDK)
print(f"VST3 SDK ready at {git('rev-parse', 'HEAD', cwd=SDK)}")
