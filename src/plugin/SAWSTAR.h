// SPDX-License-Identifier: MIT
#pragma once
#include "IPlug_include_in_plug_hdr.h"
#include "engine/Synth.h"
#include "midi/Arpeggiator.h"
#include <atomic>
#include "visual/Scope.h"
#include "visual/Meter.h"
#include <array>
#include "presets/UserPresets.h"

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
  sawstar::Arpeggiator mArp;
  std::array<iplug::IMidiMsg, 1024> mEvents{};
  int mEventCount = 0;
  int mMidiVoiceMode = 0;
  bool mOverflow = false;
  std::array<std::atomic<bool>, 128> mHeld{};
  std::array<bool, 128> mDisplayed{};
#endif
  sawstar::MeterMailbox mMeter;
  std::atomic<float> mCpu{0};
  std::atomic<int> mRate{44100},mVoiceCount{0};
  std::atomic<bool> mArpReset{false};
  std::atomic<bool> mStateRestored{false};
  std::atomic<int> mBend{8192},mMod{0};
  sawstar::Scope mScope;
  sawstar::UserPresetSelection mUserPreset;
  int mFactoryIndex = -1; // GUI-only, derived from the parameter snapshot.
  int mLfoPage = 0; // Editor selection only; both LFOs keep running.
  int mFxPage = 0; // Editor-only effect panel selection.
  float mGuiScale = 0.f; // Instance/editor preference, independent of sound presets.
  int mPage = 0; // Editor-only state; never read by the audio callback.
#if IPLUG_EDITOR
  void SyncRestoredPreset();
#endif
};
