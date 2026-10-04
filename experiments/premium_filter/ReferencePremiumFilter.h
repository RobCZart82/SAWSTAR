// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/Safety.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace sawstar::experimental {
// Frozen, uncached four-mode oracle from b5e3625 (PR #54).
// Offline diagnostic only. Keep arithmetic independent of the optimized filter.
// Independently implemented trapezoidal SVF stages; damping at zero resonance
// factors the fourth-order Butterworth denominator. No copied library code.
class ReferencePremiumFilter {
public:
  void Init(double sampleRate) {
    rate_ = SafeSampleRate(sampleRate);
    slew_ = 1. - std::exp(-1. / (.01 * rate_));
    cutoff_ = resonance_ = -1;
    mode_ = 1; weights_ = {0, 1, 0, 0};
    Set(12000, 0);
    SnapToTargets();
    Clear();
  }
  void Set(double cutoff, double resonance) {
    cutoff = FiniteClamp(cutoff, 20., std::min(20000., .45 * rate_), 20.);
    if (cutoff != cutoff_) {
      cutoff_ = cutoff;
      targetG_ = std::tan(3.14159265358979323846 * cutoff / rate_);
    }
    resonance = FiniteClamp(resonance, 0., 100., 0.);
    if (resonance != resonance_) {
      resonance_ = resonance;
      targetK_ = 1.8477590650225735 * std::pow(.1 / 1.8477590650225735, resonance / 100.);
      targetK12_ = std::sqrt(2.) * std::pow(.1 / std::sqrt(2.), resonance / 100.);
    }
  }
  // Existing parameter contract: LP12=0, LP24=1, HP12=2, BP12=3.
  void SetMode(int mode) { mode_ = std::clamp(mode, 0, 3); }
  void SnapToTargets() {
    g_ = targetG_; k_ = targetK_; k12_ = targetK12_;
    weights_ = {}; weights_[mode_] = 1;
  }
  void Clear() {
    for (auto& channel : stages_) channel = {};
    twelve_ = {};
  }
  std::array<float, 2> Process(std::array<float, 2> input) {
    g_ += slew_ * (targetG_ - g_);
    k_ += slew_ * (targetK_ - k_);
    k12_ += slew_ * (targetK12_ - k12_);
    for (size_t i = 0; i < weights_.size(); ++i)
      weights_[i] += slew_ * ((static_cast<int>(i) == mode_ ? 1. : 0.) - weights_[i]);
    for (size_t i = 0; i < weights_.size(); ++i) {
      // Preserve total weight while ending negligible tails before denormals.
      if (static_cast<int>(i) != mode_ && weights_[i] < 1e-24) {
        weights_[mode_] += weights_[i]; weights_[i] = 0;
      }
    }
    const double a1 = 1. / (1. + g_ * (g_ + k_));
    constexpr double k2 = .7653668647301795;
    const double a2 = 1. / (1. + g_ * (g_ + k2));
    const double a12 = 1. / (1. + g_ * (g_ + k12_));
    for (size_t ch = 0; ch < input.size(); ++ch) {
      if (!std::isfinite(input[ch])) {
        stages_[ch] = {};
        twelve_[ch] = {};
        input[ch] = 0;
        continue;
      }
      const double first = Step(input[ch], stages_[ch][0], g_, a1);
      const double lp24 = Step(first, stages_[ch][1], g_, a2);
      // Keep both networks running so mode changes never expose stale state.
      double band = 0;
      const double low = Step(input[ch], twelve_[ch], g_, a12, &band);
      const double high = input[ch] - k12_ * band - low;
      // Normalize BP at cutoff across Q; resonance narrows its bandwidth.
      const double wet = weights_[0] * low + weights_[1] * lp24
        + weights_[2] * high + weights_[3] * k12_ * band;
      // Finite float input can still overflow float output at resonant gain.
      if (!std::isfinite(wet) || std::abs(wet) > std::numeric_limits<float>::max()) {
        stages_[ch] = {}; twelve_[ch] = {}; input[ch] = 0;
      } else input[ch] = static_cast<float>(wet);
    }
    return input;
  }
private:
  struct State { double band = 0, low = 0; };
  static double Step(double x, State& state, double g, double a, double* bandOut = nullptr) {
    const double band = a * (state.band + g * (x - state.low));
    const double low = state.low + g * band;
    state.band = 2 * band - state.band;
    state.low = 2 * low - state.low;
    if (std::abs(state.band) < 1e-24) state.band = 0;
    if (std::abs(state.low) < 1e-24) state.low = 0;
    if (bandOut) *bandOut = band;
    return low;
  }
  std::array<std::array<State, 2>, 2> stages_{};
  std::array<State, 2> twelve_{};
  std::array<double, 4> weights_{0, 1, 0, 0};
  int mode_ = 1;
  double rate_ = 44100, slew_ = 0, cutoff_ = -1, resonance_ = -1;
  double g_ = 1, targetG_ = 1, k_ = 2, targetK_ = 2;
  double k12_ = 2, targetK12_ = 2;
};
}
