// SPDX-License-Identifier: MIT
#pragma once
#include "PremiumDrive.h"
#include "RateScaledPremiumDrive.h"
#include "PremiumLowPass.h"
#include "dsp/SevenSaw.h"

namespace sawstar::experimental {
// Offline four-mode integration adapter, never used by the shipped Synth.
// Clean mix has the same 32-sample delay as the oversampled wet path.
// This is not the final state/parameter/mode compatibility implementation.
template<class Drive> class BasicEnginePremiumFilter {
public:
  void Init(float rate) {
    drive_.Init(rate); filter_.Init(rate);
    slew_ = 1 - std::exp(-1 / (.01 * SafeSampleRate(rate)));
    mix_ = targetMix_ = 0; Clear();
  }
  void Clear() { drive_.Clear(); filter_.Clear(); dry_ = {}; cursor_ = 0; }
  void Set(float cutoff, float resonance, float mix) {
    filter_.Set(cutoff, resonance);
    targetMix_ = FiniteClamp(mix, 0.f, 100.f, 0.f) * .01;
  }
  void SetCharacter(float db, int mode) {
    filter_.SetMode(mode);
    drive_.Set(db);
  }
  void SnapToTargets() { drive_.SnapToTargets(); filter_.SnapToTargets(); mix_ = targetMix_; }
  StereoSample Process(StereoSample input) {
    // Invalid samples clear both wet and delayed dry state on that channel.
    std::array<float, 2> x{input.left, input.right};
    for (size_t ch = 0; ch < 2; ++ch) if (!std::isfinite(x[ch])) {
      for (auto& frame : dry_) frame[ch] = 0;
    }
    auto shaped = drive_.Process(x);
    for (size_t ch = 0; ch < 2; ++ch) if (!std::isfinite(x[ch])) shaped[ch] = x[ch];
    const auto wet = filter_.Process(shaped);
    const auto dry = dry_[cursor_];
    for (auto& sample : x) if (!std::isfinite(sample)) sample = 0;
    dry_[cursor_] = x; cursor_ = (cursor_ + 1) % Drive::Latency;
    mix_ += slew_ * (targetMix_ - mix_);
    return {static_cast<float>(dry[0] + mix_ * (wet[0] - dry[0])),
            static_cast<float>(dry[1] + mix_ * (wet[1] - dry[1]))};
  }
private:
  Drive drive_;
  PremiumLowPass filter_;
  std::array<std::array<float, 2>, Drive::Latency> dry_{};
  unsigned cursor_ = 0;
  double slew_ = 0, mix_ = 0, targetMix_ = 0;
};
using EnginePremiumFilter = BasicEnginePremiumFilter<PremiumDrive>;
using RateScaledEnginePremiumFilter = BasicEnginePremiumFilter<RateScaledPremiumDrive>;
}
