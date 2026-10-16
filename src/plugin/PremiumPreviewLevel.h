// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/Safety.h"
#include <cmath>

namespace sawstar {
// Listening-preview output calibration. No change to filter/Drive or FX input.
// Blend to unity for a dry filter; never attempt signal-following auto gain.
class PremiumPreviewLevel {
public:
  static constexpr float WetGain = 1.4125375446227544f; // +3 dB, deliberately below full Classic matching.
  void Reset(float rate) {
    slew_ = 1.f - std::exp(-1.f / (.005f * SafeSampleRate(rate)));
    mix_ = 0;
  }
  void Snap(float percent) { mix_ = Mix(percent); }
  float Process(float percent) {
    mix_ += slew_ * (Mix(percent) - mix_);
    return 1.f + (WetGain - 1.f) * mix_;
  }
private:
  static float Mix(float percent) { return FiniteClamp(percent, 0.f, 100.f, 0.f) * .01f; }
  float mix_ = 0, slew_ = 1;
};
}
