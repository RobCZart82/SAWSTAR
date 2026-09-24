#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Apply SAWSTAR's pinned iPlug2 editor-MIDI overflow recovery hooks."""
from pathlib import Path
import sys


PATCHES = (
    (
        "IPlug/IPlugAPIBase.h",
        b"  void DeferMidiMsg(const IMidiMsg& msg) override { mMidiMsgsFromEditor.Push(msg); }",
        b"  void DeferMidiMsg(const IMidiMsg& msg) override { mMidiMsgsFromEditor.Push(msg); }\n"
        b"\n"
        b"  // SAWSTAR patch: audio-thread callbacks for editor MIDI queue recovery.\n"
        b"  virtual void OnMidiMsgFromEditor(const IMidiMsg&) {}\n"
        b"  virtual void OnMidiMsgFromEditorOverflow() {}\n",
    ),
    (
        "IPlug/VST3/IPlugVST3_ProcessorBase.h",
        b'#include "IPlugVST3_Defs.h"\n',
        b'#include "IPlugVST3_Defs.h"\n\n#include <atomic>\n',
    ),
    (
        "IPlug/VST3/IPlugVST3_ProcessorBase.h",
        b"  IPlugVST3ProcessorBase(Config c, IPlugAPIBase& plug);\n",
        b"  IPlugVST3ProcessorBase(Config c, IPlugAPIBase& plug);\n"
        b"  void SignalMidiMsgFromEditorOverflow() noexcept {\n"
        b"    mMidiMsgFromEditorOverflow.store(true, std::memory_order_release);\n"
        b"  }\n",
    ),
    (
        "IPlug/VST3/IPlugVST3_ProcessorBase.h",
        b"  bool mSidechainActive = false;\n",
        b"  bool mSidechainActive = false;\n"
        b"  std::atomic<bool> mMidiMsgFromEditorOverflow{false};\n",
    ),
    (
        "IPlug/VST3/IPlugVST3_ProcessorBase.cpp",
        b"  while (editorQueue.Pop(msg))\n"
        b"  {\n"
        b"    ProcessMidiMsg(msg);\n"
        b"  }\n",
        b"  while (editorQueue.Pop(msg))\n"
        b"  {\n"
        b"    ProcessMidiMsg(msg);\n"
        b"    mPlug.OnMidiMsgFromEditor(msg);\n"
        b"  }\n"
        b"\n"
        b"  if (mMidiMsgFromEditorOverflow.exchange(false, std::memory_order_acq_rel))\n"
        b"    mPlug.OnMidiMsgFromEditorOverflow();\n",
    ),
    (
        "IPlug/VST3/IPlugVST3_Processor.cpp",
        b"        mMidiMsgsFromEditor.Push(msg);\n"
        b"        return kResultOk;\n",
        b"        // SAWSTAR patch: preserve a recovery signal if a note edge is dropped.\n"
        b"        if (!mMidiMsgsFromEditor.Push(msg) &&\n"
        b"            (msg.StatusMsg() == IMidiMsg::kNoteOn || msg.StatusMsg() == IMidiMsg::kNoteOff))\n"
        b"          SignalMidiMsgFromEditorOverflow();\n"
        b"        return kResultOk;\n",
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
