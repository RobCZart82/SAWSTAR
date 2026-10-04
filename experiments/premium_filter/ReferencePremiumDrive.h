// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/Safety.h"
#include <array>
#include <cmath>

namespace sawstar::experimental {
// Offline fixed-factor waveshaper, with explicit interpolation and anti-alias FIRs.
// Direct convolution intentionally favors a readable reference over CPU cost.
template<unsigned Factor> class FixedReferencePremiumDrive {
  static_assert(Factor == 4 || Factor == 8, "Offline reference factors are 4x or 8x");
public:
  static constexpr int Latency = 32;
  void Init(double rate) {
    slew_ = 1 - std::exp(-1 / (.01 * SafeSampleRate(rate)));
    constexpr double pi = 3.14159265358979323846, cutoff = .45 / Factor;
    double sum = 0;
    for (int i = 0; i < static_cast<int>(TapCount); ++i) {
      const int n = i - static_cast<int>((TapCount - 1) / 2);
      const double sinc = n == 0 ? 2 * cutoff : std::sin(2 * pi * cutoff * n) / (pi * n);
      const double window = .42 - .5 * std::cos(2 * pi * i / (TapCount - 1)) + .08 * std::cos(4 * pi * i / (TapCount - 1));
      taps_[i] = sinc * window; sum += taps_[i];
    }
    for (auto& tap : taps_) tap /= sum;
    gain_ = target_ = 1; Clear();
  }
  void Set(double db) { target_ = std::pow(10., FiniteClamp(db, 0., 24., 0.) / 20); }
  void SnapToTargets() { gain_ = target_; }
  void Clear() { up_ = {}; down_ = {}; }
  std::array<float, 2> Process(std::array<float, 2> input) {
    gain_ += slew_ * (target_ - gain_);
    for (size_t ch = 0; ch < 2; ++ch) {
      if (!std::isfinite(input[ch])) { up_[ch] = {}; down_[ch] = {}; input[ch] = 0; }
      double out = 0;
      for (int phase = 0; phase < static_cast<int>(Factor); ++phase) {
        const double x = Tick(up_[ch], phase == 0 ? double(Factor) * input[ch] : 0.);
        const double shaped = gain_ == 1 ? x : std::tanh(gain_ * x) / gain_;
        const double y = Tick(down_[ch], shaped);
        if (phase == 0) out = y;
      }
      input[ch] = static_cast<float>(out);
    }
    return input;
  }
private:
  static constexpr unsigned TapCount = 32 * Factor + 1, RingSize = Factor == 8 ? 512 : 256;
  struct Fir { std::array<double, RingSize> history{}; unsigned cursor = 0; };
  double Tick(Fir& fir, double x) {
    fir.history[fir.cursor] = x;
    double y = 0;
    for (unsigned i = 0; i < taps_.size(); ++i) y += taps_[i] * fir.history[(fir.cursor - i) & (RingSize - 1)];
    fir.cursor = (fir.cursor + 1) & (RingSize - 1);
    return y;
  }
  std::array<double, TapCount> taps_{};
  std::array<Fir, 2> up_{}, down_{};
  double gain_ = 1, target_ = 1, slew_ = 0;
};
using ReferencePremiumDrive = FixedReferencePremiumDrive<4>;
using ReferencePremiumDrive8x = FixedReferencePremiumDrive<8>;
}
