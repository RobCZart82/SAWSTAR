#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Apply SAWSTAR's pinned iPlug2 editor-MIDI overflow recovery hooks."""
from pathlib import Path
import sys


PATCHES = (
    (
        "IPlug/IPlugAPIBase.h",
        b"#include <cstdint>\n",
        b"#include <cstdint>\n#include <atomic>\n",
    ),
    (
        "IPlug/IPlugAPIBase.h",
        b"  void DeferMidiMsg(const IMidiMsg& msg) override { mMidiMsgsFromEditor.Push(msg); }",
        b"  void DeferMidiMsg(const IMidiMsg& msg) override {\n"
        b"    if (!mMidiMsgsFromEditor.Push(msg) &&\n"
        b"        (msg.StatusMsg() == IMidiMsg::kNoteOn || msg.StatusMsg() == IMidiMsg::kNoteOff))\n"
        b"      SignalMidiMsgFromEditorOverflow();\n"
        b"  }\n"
        b"\n"
        b"  // The default preserves normal delivery for plugs without a custom route.\n"
        b"  // Return true only when the plug-in queued/handled this message itself.\n"
        b"  virtual bool ProcessMidiMsgFromEditor(const IMidiMsg&) { return false; }\n"
        b"  virtual void OnMidiMsgFromEditorOverflow() {}\n"
        b"  void SignalMidiMsgFromEditorOverflow() noexcept {\n"
        b"    mMidiMsgFromEditorOverflow.store(true, std::memory_order_release);\n"
        b"  }\n"
        b"  bool TakeMidiMsgFromEditorOverflow() noexcept {\n"
        b"    return mMidiMsgFromEditorOverflow.exchange(false, std::memory_order_acq_rel);\n"
        b"  }",
    ),
    (
        "IPlug/IPlugAPIBase.h",
        b"  IPlugQueue<IMidiMsg> mMidiMsgsFromEditor {MIDI_TRANSFER_SIZE}; // a queue of midi messages generated in the editor by clicking keyboard UI etc",
        b"  IPlugQueue<IMidiMsg> mMidiMsgsFromEditor {MIDI_TRANSFER_SIZE}; // a queue of midi messages generated in the editor by clicking keyboard UI etc\n"
        b"  std::atomic<bool> mMidiMsgFromEditorOverflow{false};",
    ),
    (
        "IPlug/VST3/IPlugVST3_ProcessorBase.cpp",
        b"  while (editorQueue.Pop(msg))\n"
        b"  {\n"
        b"    ProcessMidiMsg(msg);\n"
        b"  }\n",
        b"  while (editorQueue.Pop(msg))\n"
        b"  {\n"
        b"    if (!mPlug.ProcessMidiMsgFromEditor(msg))\n"
        b"      ProcessMidiMsg(msg);\n"
        b"  }\n",
    ),
    (
        "IPlug/VST3/IPlugVST3_ProcessorBase.cpp",
        b"  ProcessAudio(data, setup, ins, outs);\n",
        b"  ProcessAudio(data, setup, ins, outs);\n"
        b"\n"
        b"  // Run recovery after this block has accounted for every accepted editor event.\n"
        b"  if (fromEditor.WasEmpty() && mPlug.TakeMidiMsgFromEditorOverflow())\n"
        b"  {\n"
        b"    if (fromEditor.WasEmpty())\n"
        b"      mPlug.OnMidiMsgFromEditorOverflow();\n"
        b"    else\n"
        b"      mPlug.SignalMidiMsgFromEditorOverflow();\n"
        b"  }\n",
    ),
)


def apply(root):
    root = Path(root)
    for relative, original, replacement in PATCHES:
        path = root / relative
        source = path.read_bytes()
        # Windows checkouts may use CRLF even though the pinned source and
        # patch contexts are written with LF. Match while preserving checkout
        # line endings so the patch remains byte-local and repeatable.
        newline = b"\r\n" if b"\r\n" in source else b"\n"
        original_for_file = original.replace(b"\n", newline)
        replacement_for_file = replacement.replace(b"\n", newline)
        if replacement_for_file in source:
            continue
        if source.count(original_for_file) != 1:
            raise RuntimeError(
                f"Pinned iPlug2 source does not match expected patch context: {relative}"
            )
        path.write_bytes(source.replace(original_for_file, replacement_for_file, 1))

    # Fail closed on partial or drifted application.
    for relative, _, replacement in PATCHES:
        source = (root / relative).read_bytes()
        newline = b"\r\n" if b"\r\n" in source else b"\n"
        if replacement.replace(b"\n", newline) not in source:
            raise RuntimeError(f"iPlug2 patch was not fully applied: {relative}")


if __name__ == "__main__":
    try:
        apply(sys.argv[1] if len(sys.argv) > 1 else Path(__file__).resolve().parents[1] / "third_party/iPlug2")
        print("Pinned iPlug2 editor MIDI overflow recovery enabled.")
    except (OSError, RuntimeError) as error:
        raise SystemExit(str(error))
