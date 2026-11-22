# SPDX-License-Identifier: MIT
"""Emit the production state adapter for the offline stream regression."""
import importlib.util
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location(
    "sawstar_iplug2_patch", ROOT / "scripts/patch-iplug2-midi-overflow.py"
)
patch = importlib.util.module_from_spec(spec)
spec.loader.exec_module(patch)

relative = "IPlug/VST3/IPlugVST3_Common.h"
[(original, replacement)] = [
    (before, after)
    for name, before, after in patch.PATCHES
    if name == relative and before.startswith(b"struct IPlugVST3State\n")
]
upstream = ROOT / "third_party/iPlug2" / relative
if upstream.is_file():
    source = upstream.read_bytes().replace(b"\r\n", b"\n")
    if source.count(original) + source.count(replacement) != 1:
        raise SystemExit("VST3 state adapter drifted from the pinned patch context.")

Path(sys.argv[1]).write_bytes(replacement.rstrip() + b"\n")
