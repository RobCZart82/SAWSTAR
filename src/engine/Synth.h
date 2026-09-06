// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/SevenSaw.h"
#include "dsp/LowPass.h"
#include "dsp/FilterModulation.h"
#include "Control/adsr.h"
#include <array>
#include <cstdint>
namespace sawstar {
// Audio-thread owned. Fixed storage: no allocation or locks while rendering.
class Synth {
public:
  void Reset(double sampleRate);
  void SetParameters(double gainDb, double attackMs, double decayMs, double sustain, double releaseMs);
  void Midi(int status, int data1, int data2);
  void SetFilter(float cutoffHz, float resonancePercent, float mixPercent);
  void SetFilterEnvelope(float amount, float tracking, float attack, float decay, float sustain, float release);
  void SetPerformance(float bendRange,float modDepth);
  int PitchBend(int channel) const { return bend_[channel&15]; }
  int ModWheel(int channel) const { return mod_[channel&15]; }
  void SetSaw(float detuneCents, float mixPercent, float widthPercent);
  StereoSample ProcessStereo();
  float Process() { const auto s=ProcessStereo(); return (s.left+s.right)*0.5f; }
  bool Held(int note) const;
  int ActiveVoices() const;
private:
  struct Voice {
    SevenSaw osc;
    LowPass filter;
    FilterModulation filterMod;
    daisysp::Adsr env;
    int note = -1, channel = 0;
    bool held = false, gate = false;
    float velocity = 0;
    uint64_t age = 0;
  };
  std::array<Voice, 16> voices_{};
  std::array<bool, 16> sustain_{};
  std::array<int,16> bend_{{8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192}}, mod_{};
  std::array<float,16> bendRatio_{{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}}, bendTarget_{{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}};
  float bendRange_=2,modDepth_=24;
  void UpdateBend(int channel);
  uint64_t age_ = 0;
  float cutoff_=12000, resonance_=0, filterMix_=0;
  float amount_=0, tracking_=0, filterAttack_=10, filterDecay_=200, filterSustain_=0, filterRelease_=250;
  float gain_ = 0, targetGain_ = 0.25f, smoothing_ = 0.002f;
};
}
