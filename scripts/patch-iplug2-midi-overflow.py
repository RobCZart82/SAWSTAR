#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Apply pinned iPlug2 MIDI recovery and host controller-point fixes."""
from pathlib import Path
import sys


PATCHES = (
    (
        "IPlug/VST3/IPlugVST3_ProcessorBase.cpp",
        b"void IPlugVST3ProcessorBase::ProcessParameterChanges(ProcessData& data, IPlugQueue<IMidiMsg>& fromProcessor)\n"
        b"{\n"
        b"  IParameterChanges* paramChanges = data.inputParameterChanges;\n"
        b"  \n"
        b"  if (paramChanges)\n"
        b"  {\n"
        b"    int32 numParamsChanged = paramChanges->getParameterCount();\n"
        b"    \n"
        b"    for (int32 i = 0; i < numParamsChanged; i++)\n"
        b"    {\n"
        b"      IParamValueQueue* paramQueue = paramChanges->getParameterData(i);\n"
        b"      if (paramQueue)\n"
        b"      {\n"
        b"        int32 numPoints = paramQueue->getPointCount();\n"
        b"        int32 offsetSamples;\n"
        b"        double value;\n"
        b"        \n"
        b"        if (paramQueue->getPoint(numPoints - 1,  offsetSamples, value) == kResultTrue)\n"
        b"        {\n"
        b"          int idx = paramQueue->getParameterId();\n"
        b"          \n"
        b"          switch (idx)\n"
        b"          {\n"
        b"            case kBypassParam:\n"
        b"            {\n"
        b"              const bool bypassed = (value > 0.5);\n"
        b"\n"
        b"              if (bypassed != GetBypassed())\n"
        b"                SetBypassed(bypassed);\n"
        b"\n"
        b"              break;\n"
        b"            }\n"
        b"            default:\n"
        b"            {\n"
        b"              if (idx >= 0 && idx < mPlug.NParams())\n"
        b"              {\n"
        b"#ifdef PARAMS_MUTEX\n"
        b"                mPlug.mParams_mutex.Enter();\n"
        b"#endif\n"
        b"                mPlug.GetParam(idx)->SetNormalized(value);\n"
        b"              \n"
        b"                // In VST3 non distributed the same parameter value is also set via IPlugVST3Controller::setParamNormalized(ParamID tag, ParamValue value)\n"
        b"                mPlug.OnParamChange(idx, kHost, offsetSamples);\n"
        b"#ifdef PARAMS_MUTEX\n"
        b"                mPlug.mParams_mutex.Leave();\n"
        b"#endif\n"
        b"              }\n"
        b"              else if (idx >= kMIDICCParamStartIdx)\n"
        b"              {\n"
        b"                int index = idx - kMIDICCParamStartIdx;\n"
        b"                int channel = index / kCountCtrlNumber;\n"
        b"                int ctrlr = index % kCountCtrlNumber;\n"
        b"\n"
        b"                IMidiMsg msg;\n"
        b"\n"
        b"                if (ctrlr == kAfterTouch)\n"
        b"                  msg.MakeChannelATMsg((int) (value * 127.), offsetSamples, channel);\n"
        b"                else if (ctrlr == kPitchBend)\n"
        b"                  msg.MakePitchWheelMsg((value * 2.)-1., channel, offsetSamples);\n"
        b"                else\n"
        b"                  msg.MakeControlChangeMsg((IMidiMsg::EControlChangeMsg) ctrlr, value, channel, offsetSamples);\n"
        b"\n"
        b"                fromProcessor.Push(msg);\n"
        b"                ProcessMidiMsg(msg);\n"
        b"              }\n"
        b"            }\n"
        b"              break;\n"
        b"          }\n"
        b"        }\n"
        b"      }\n"
        b"    }\n"
        b"  }\n"
        b"}",
        b"void IPlugVST3ProcessorBase::ProcessParameterChanges(ProcessData& data, IPlugQueue<IMidiMsg>& fromProcessor)\n"
        b"{\n"
        b"  IParameterChanges* paramChanges = data.inputParameterChanges;\n"
        b"  \n"
        b"  if (paramChanges)\n"
        b"  {\n"
        b"    int32 numParamsChanged = paramChanges->getParameterCount();\n"
        b"    \n"
        b"    for (int32 i = 0; i < numParamsChanged; i++)\n"
        b"    {\n"
        b"      IParamValueQueue* paramQueue = paramChanges->getParameterData(i);\n"
        b"      if (paramQueue)\n"
        b"      {\n"
        b"        int32 numPoints = paramQueue->getPointCount();\n"
        b"        int32 offsetSamples;\n"
        b"        double value;\n"
        b"        const int idx = paramQueue->getParameterId();\n"
        b"        const bool midiController = idx != kBypassParam &&\n"
        b"          idx >= mPlug.NParams() && idx >= kMIDICCParamStartIdx;\n"
        b"\n"
        b"        // Preserve every MIDI edge and its offset; regular parameters use the final value.\n"
        b"        for (int32 point = midiController ? 0 : numPoints - 1;\n"
        b"             point >= 0 && point < numPoints; ++point)\n"
        b"        {\n"
        b"          if (paramQueue->getPoint(point, offsetSamples, value) == kResultTrue)\n"
        b"          {\n"
        b"          \n"
        b"            switch (idx)\n"
        b"            {\n"
        b"              case kBypassParam:\n"
        b"              {\n"
        b"                const bool bypassed = (value > 0.5);\n"
        b"\n"
        b"                if (bypassed != GetBypassed())\n"
        b"                  SetBypassed(bypassed);\n"
        b"\n"
        b"                break;\n"
        b"              }\n"
        b"              default:\n"
        b"              {\n"
        b"                if (idx >= 0 && idx < mPlug.NParams())\n"
        b"                {\n"
        b"  #ifdef PARAMS_MUTEX\n"
        b"                  mPlug.mParams_mutex.Enter();\n"
        b"  #endif\n"
        b"                  mPlug.GetParam(idx)->SetNormalized(value);\n"
        b"              \n"
        b"                  // In VST3 non distributed the same parameter value is also set via IPlugVST3Controller::setParamNormalized(ParamID tag, ParamValue value)\n"
        b"                  mPlug.OnParamChange(idx, kHost, offsetSamples);\n"
        b"  #ifdef PARAMS_MUTEX\n"
        b"                  mPlug.mParams_mutex.Leave();\n"
        b"  #endif\n"
        b"                }\n"
        b"                else if (idx >= kMIDICCParamStartIdx)\n"
        b"                {\n"
        b"                  int index = idx - kMIDICCParamStartIdx;\n"
        b"                  int channel = index / kCountCtrlNumber;\n"
        b"                  int ctrlr = index % kCountCtrlNumber;\n"
        b"\n"
        b"                  IMidiMsg msg;\n"
        b"\n"
        b"                  if (ctrlr == kAfterTouch)\n"
        b"                    msg.MakeChannelATMsg((int) (value * 127.), offsetSamples, channel);\n"
        b"                  else if (ctrlr == kPitchBend)\n"
        b"                    msg.MakePitchWheelMsg((value * 2.)-1., channel, offsetSamples);\n"
        b"                  else\n"
        b"                    msg.MakeControlChangeMsg((IMidiMsg::EControlChangeMsg) ctrlr, value, channel, offsetSamples);\n"
        b"\n"
        b"                  fromProcessor.Push(msg);\n"
        b"                  ProcessMidiMsg(msg);\n"
        b"                }\n"
        b"              }\n"
        b"                break;\n"
        b"            }\n"
        b"          }\n"
        b"        }\n"
        b"      }\n"
        b"    }\n"
        b"  }\n"
        b"}",
    ),
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
        b"  // Consumer-side reset only; host reset must be serialized with audio.\n"
        b"  // Snapshot the count so concurrent GUI input cannot extend this loop.\n"
        b"  void DiscardPendingMidiFromEditor() {\n"
        b"    TakeMidiMsgFromEditorOverflow();\n"
        b"    const auto pending = mMidiMsgsFromEditor.ElementsAvailable();\n"
        b"    IMidiMsg discarded;\n"
        b"    for (size_t i = 0; i < pending; ++i)\n"
        b"      if (!mMidiMsgsFromEditor.Pop(discarded)) break;\n"
        b"  }\n"
        b"\n"
        b"  // The default preserves normal delivery for plugs without a custom route.\n"
        b"  // Return true only when the plug-in queued/handled this message itself.\n"
        b"  virtual bool ProcessMidiMsgFromEditor(const IMidiMsg&) { return false; }\n"
        b"  virtual void OnMidiMsgFromEditorOverflow() {}\n"
        b"  virtual bool ProcessAudioWhileBypassed() const { return false; }\n"
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
        b"    if (GetBypassed())\n",
        b"    if (GetBypassed() && !mPlug.ProcessAudioWhileBypassed())\n",
    ),
    (
        "IPlug/VST3/IPlugVST3_ProcessorBase.cpp",
        b"  ProcessAudio(data, setup, ins, outs);\n",
        b"  ProcessAudio(data, setup, ins, outs);\n"
        b"\n"
        b"  // Run recovery after this block has accounted for every accepted editor event.\n"
        b"  if (data.numSamples > 0 && (!GetBypassed() || mPlug.ProcessAudioWhileBypassed()) &&\n"
        b"      fromEditor.WasEmpty() && mPlug.TakeMidiMsgFromEditorOverflow())\n"
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
        # Accept only exact known previous patch versions (never fuzzy edits).
        predecessors = []
        if b"void DiscardPendingMidiFromEditor()" in replacement:
            no_bypass_hook = replacement.replace(
                b"  virtual bool ProcessAudioWhileBypassed() const { return false; }\n", b"")
            start = no_bypass_hook.index(b"  // Consumer-side reset only;")
            end = no_bypass_hook.index(b"  // The default preserves", start)
            predecessors = [no_bypass_hook, no_bypass_hook[:start] + no_bypass_hook[end:]]
        if b"data.numSamples > 0 &&" in replacement:
            old_guard = replacement.replace(
                b"(!GetBypassed() || mPlug.ProcessAudioWhileBypassed())", b"!GetBypassed()")
            predecessors = [old_guard, old_guard.replace(
                b"if (data.numSamples > 0 && !GetBypassed() &&\n      fromEditor.WasEmpty()",
                b"if (fromEditor.WasEmpty()")]
        upgraded = False
        for previous in predecessors:
            previous = previous.replace(b"\n", newline)
            if source.count(previous) == 1:
                path.write_bytes(source.replace(previous, replacement_for_file, 1))
                upgraded = True
                break
        if upgraded:
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
        print("Pinned iPlug2 MIDI recovery and host controller points enabled.")
    except (OSError, RuntimeError) as error:
        raise SystemExit(str(error))
