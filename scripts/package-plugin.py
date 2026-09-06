#!/usr/bin/env python3
"""Archive the bundle without losing Unix file modes; include distribution notices."""
from pathlib import Path
import sys
import zipfile
root = Path(__file__).resolve().parents[1]
bundle = Path(sys.argv[1]) / "SAWSTAR.vst3"
if not bundle.is_dir():
    raise SystemExit(f"Missing plugin bundle: {bundle}")
with zipfile.ZipFile(sys.argv[2], "w", zipfile.ZIP_DEFLATED) as archive:
    for p in sorted(bundle.rglob("*")):
        archive.write(p, Path("SAWSTAR.vst3") / p.relative_to(bundle))
    for p in sorted((root / "third_party/licenses").rglob("*")):
        if p.is_file(): archive.write(p, Path("licenses") / p.name)
    for name in ["LICENSE", "THIRD_PARTY_NOTICES.md", "docs/PLUGIN_SHELL.md"]:
        archive.write(root / name, Path(name).name)
