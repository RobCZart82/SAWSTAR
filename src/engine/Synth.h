// SPDX-License-Identifier: MIT
#pragma once
#include "Synthesis/oscillator.h"
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
  float Process();
  bool Held(int note) const;
  int ActiveVoices() const;
private:
  struct Voice {
    daisysp::Oscillator osc;
    daisysp::Adsr env;
    int note = -1, channel = 0;
    bool held = false, gate = false;
    float velocity = 0;
    uint64_t age = 0;
  };
  std::array<Voice, 16> voices_{};
  std::array<bool, 16> sustain_{};
  uint64_t age_ = 0;
  float gain_ = 0, targetGain_ = 0.25f, smoothing_ = 0.002f;
};
}
