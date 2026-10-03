// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/Safety.h"
#include <array>
#include <cmath>

namespace sawstar::experimental {
// Linear four-pole research candidate. Not selected by the shipped engine.
// Independently implemented trapezoidal SVF stages; damping at zero resonance
// factors the fourth-order Butterworth denominator. No copied library code.
class PremiumLowPass {
public:
  void Init(double sampleRate) {
    rate_ = SafeSampleRate(sampleRate);
    slew_ = 1. - std::exp(-1. / (.01 * rate_));
    cutoff_ = -1;
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
    const double r = FiniteClamp(resonance, 0., 100., 0.) / 100.;
    targetK_ = 1.8477590650225735 * std::pow(.1 / 1.8477590650225735, r);
  }
  void SnapToTargets() { g_ = targetG_; k_ = targetK_; }
  void Clear() { for (auto& channel : stages_) channel = {}; }
  std::array<float, 2> Process(std::array<float, 2> input) {
    g_ += slew_ * (targetG_ - g_);
    k_ += slew_ * (targetK_ - k_);
    const double a1 = 1. / (1. + g_ * (g_ + k_));
    constexpr double k2 = .7653668647301795;
    const double a2 = 1. / (1. + g_ * (g_ + k2));
    for (size_t ch = 0; ch < input.size(); ++ch) {
      if (!std::isfinite(input[ch])) {
        stages_[ch] = {};
        input[ch] = 0;
        continue;
      }
      const double first = Step(input[ch], stages_[ch][0], g_, a1);
      input[ch] = static_cast<float>(Step(first, stages_[ch][1], g_, a2));
    }
    return input;
  }
private:
  struct State { double band = 0, low = 0; };
  static double Step(double x, State& state, double g, double a) {
    const double band = a * (state.band + g * (x - state.low));
    const double low = state.low + g * band;
    state.band = 2 * band - state.band;
    state.low = 2 * low - state.low;
    if (std::abs(state.band) < 1e-24) state.band = 0;
    if (std::abs(state.low) < 1e-24) state.low = 0;
    return low;
  }
  std::array<std::array<State, 2>, 2> stages_{};
  double rate_ = 44100, slew_ = 0, cutoff_ = -1;
  double g_ = 1, targetG_ = 1, k_ = 2, targetK_ = 2;
};
}
