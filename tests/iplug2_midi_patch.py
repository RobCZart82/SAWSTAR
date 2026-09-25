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

    api = second["IPlug/IPlugAPIBase.h"]
    consumer = second["IPlug/VST3/IPlugVST3_ProcessorBase.cpp"]
    if b"if (!mMidiMsgsFromEditor.Push(msg)" not in api or b"SignalMidiMsgFromEditorOverflow();" not in api:
        raise SystemExit("The actual editor queue producer does not report dropped note edges.")
    if b"if (!mPlug.ProcessMidiMsgFromEditor(msg))\n      ProcessMidiMsg(msg);" not in consumer:
        raise SystemExit("The VST3 consumer does not deliver editor MIDI exactly once.")
    if any(relative.endswith("IPlugVST3_Processor.cpp") for relative, _, _ in patch.PATCHES):
        raise SystemExit("The overflow signal must not target the unused distributed VST3 producer.")
    audio = consumer.find(b"ProcessAudio(data, setup, ins, outs);")
    recovery = consumer.find(b"TakeMidiMsgFromEditorOverflow()")
    if (audio < 0 or recovery < audio or
        b"if (fromEditor.WasEmpty() && mPlug.TakeMidiMsgFromEditorOverflow())" not in consumer or
        b"else\n      mPlug.SignalMidiMsgFromEditorOverflow();" not in consumer):
        raise SystemExit("Editor recovery must wait until accepted editor events have been processed.")

print("iPlug2 MIDI overflow patch applies cleanly and is idempotent.")
