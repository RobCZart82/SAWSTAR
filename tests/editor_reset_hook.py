# SPDX-License-Identifier: MIT
"""Extract the production patch hook for its compiled queue-contract regression."""
import importlib.util
from pathlib import Path
import sys
root = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('patch', root / 'scripts/patch-iplug2-midi-overflow.py')
patch = importlib.util.module_from_spec(spec)
spec.loader.exec_module(patch)
replacement = next(new.decode() for _, _, new in patch.PATCHES
                   if b'void DiscardPendingMidiFromEditor()' in new)
start = replacement.index('  void DiscardPendingMidiFromEditor()')
end = replacement.index('\n  }', start) + len('\n  }')
Path(sys.argv[1]).write_text(replacement[start:end] + '\n')
# Ensure the tested hook is actually used by the plugin's reset entry point.
source = (root / 'src/plugin/SAWSTAR.cpp').read_text()
reset = source.split('void SAWSTAR::OnReset() {', 1)[1].split('void SAWSTAR::ProcessBlock', 1)[0]
assert reset.index('DiscardPendingMidiFromEditor();') < reset.index('mSynth.Reset(')
# Transport-induced ARP clears must invalidate editor ownership before recovery.
block = source.split('void SAWSTAR::ProcessBlock(', 1)[1].split('void SAWSTAR::ProcessMidiMsg(', 1)[0]
assert 'if(sawstar::ApplyEngineControls(mSynth,mArp,' in block
assert 'GetTempo(),GetTransportIsRunning()))mEditorMidiTracker.Clear();' in block
assert block.index('GetTransportIsRunning()))mEditorMidiTracker.Clear();') < block.index('mEditorMidiTracker.ReleaseSome(')
