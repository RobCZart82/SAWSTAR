// SPDX-License-Identifier: MIT
#pragma once
#include "IPlug_include_in_plug_hdr.h"
#include "engine/Synth.h"
#include <atomic>
#include <array>

class SAWSTAR final : public iplug::Plugin {
public:
  bool SerializeState(iplug::IByteChunk& chunk) const override;
  int UnserializeState(const iplug::IByteChunk& chunk, int startPos) override;
  explicit SAWSTAR(const iplug::InstanceInfo& info);
#if IPLUG_DSP
  void OnReset() override;
  void OnIdle() override;
  void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int frames) override;
  void ProcessMidiMsg(const iplug::IMidiMsg& msg) override;
#endif
private:
#if IPLUG_DSP
  sawstar::Synth mSynth;
  std::array<iplug::IMidiMsg, 1024> mEvents{};
  int mEventCount = 0;
  bool mOverflow = false;
  std::array<std::atomic<bool>, 128> mHeld{};
  std::array<bool, 128> mDisplayed{};
#endif
  int mPage = 0; // Editor-only state; never read by the audio callback.
};
