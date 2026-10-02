# SPDX-License-Identifier: MIT
"""Compile the pinned adapter method with the actual production patches."""
import importlib.util
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('patch', root / 'scripts/patch-iplug2-midi-overflow.py')
patch = importlib.util.module_from_spec(spec)
spec.loader.exec_module(patch)
fixture = (root / 'tests/fixtures/iplug2_parameter_changes.inc').read_bytes()
method = fixture[fixture.index(b'void IPlugVST3ProcessorBase::ProcessParameterChanges('):].strip()

def patched(source):
    for relative, old, new in patch.PATCHES:
        if relative.endswith('IPlugVST3_ProcessorBase.cpp') and old in source:
            assert source.count(old) == 1
            source = source.replace(old, new)
    return source

expected = patched(method)
upstream = root / 'third_party/iPlug2/IPlug/VST3/IPlugVST3_ProcessorBase.cpp'
if upstream.exists():
    source = upstream.read_bytes().replace(b'\r\n', b'\n')
    start = source.index(b'void IPlugVST3ProcessorBase::ProcessParameterChanges(')
    end = source.index(b'void IPlugVST3ProcessorBase::ProcessAudio(', start)
    assert patched(source[start:end].strip()) == expected, 'Pinned adapter fixture drifted'
Path(sys.argv[1]).write_bytes(expected + b'\n')
