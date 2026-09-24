# SPDX-License-Identifier: MIT
"""Check that the pinned iPlug2 source patch applies reproducibly and idempotently."""
from pathlib import Path
import importlib.util
import tempfile


ROOT = Path(__file__).resolve().parents[1]
PATCH_PATH = ROOT / "scripts/patch-iplug2-midi-overflow.py"
spec = importlib.util.spec_from_file_location("sawstar_iplug2_patch", PATCH_PATH)
patch = importlib.util.module_from_spec(spec)
spec.loader.exec_module(patch)

with tempfile.TemporaryDirectory() as directory:
    fixture = Path(directory)
    for relative, original, _ in patch.PATCHES:
        path = fixture / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open("ab") as stream:
            stream.write(original)

    patch.apply(fixture)
    first = {relative: (fixture / relative).read_bytes()
             for relative, _, _ in patch.PATCHES}
    patch.apply(fixture)
    second = {relative: (fixture / relative).read_bytes()
              for relative, _, _ in patch.PATCHES}
    if first != second:
        raise SystemExit("Applying the iPlug2 patch twice changed the result.")
    for relative, _, replacement in patch.PATCHES:
        if replacement not in second[relative]:
            raise SystemExit(f"Patch result missing expected change: {relative}")

print("iPlug2 MIDI overflow patch applies cleanly and is idempotent.")
