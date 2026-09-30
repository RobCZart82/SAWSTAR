# SPDX-License-Identifier: MIT
"""Compile the actual post-audio patch in the lifecycle regression."""
import importlib.util
from pathlib import Path
import sys
root = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('patch', root / 'scripts/patch-iplug2-midi-overflow.py')
patch = importlib.util.module_from_spec(spec)
spec.loader.exec_module(patch)
body = next(new for _, old, new in patch.PATCHES if old == b'  ProcessAudio(data, setup, ins, outs);\n')
Path(sys.argv[1]).write_bytes(body.split(b'\n', 1)[1])
