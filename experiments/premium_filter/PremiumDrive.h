// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/Safety.h"
#include <array>
#include <cmath>

namespace sawstar::experimental {
// Research-only fixed-factor waveshaper, with explicit interpolation and anti-alias FIRs.
// Polyphase interpolation skips known zero inputs; only retained decimator
// phases compute a convolution. ReferencePremiumDrive retains the full version.
template<unsigned Factor> class FixedRatePremiumDrive {
  static_assert(Factor == 2 || Factor == 4, "Research factors are 2x and 4x");
public:
  static constexpr int Latency = 32;
  void Init(double rate) {
    slew_ = 1 - std::exp(-1 / (.01 * SafeSampleRate(rate)));
    constexpr double pi = 3.14159265358979323846, cutoff = .45 / Factor;
    double sum = 0;
    for (int i = 0; i < static_cast<int>(TapCount); ++i) {
      const int n = i - static_cast<int>(Center);
      const double sinc = n == 0 ? 2 * cutoff : std::sin(2 * pi * cutoff * n) / (pi * n);
      const double window = .42 - .5 * std::cos(2 * pi * i / (TapCount - 1)) + .08 * std::cos(4 * pi * i / (TapCount - 1));
      taps_[i] = sinc * window; sum += taps_[i];
    }
    // Enforce the mathematical symmetry despite libm rounding in the window.
    // Pair averaging preserves the DC sum and changes coefficients only at
    // double-rounding scale; the original full FIR remains the test oracle.
    for (unsigned i = 0; i < Center; ++i)
      taps_[i] = taps_[TapCount - 1 - i] = .5 * (taps_[i] + taps_[TapCount - 1 - i]);
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
        const double x = Tick(up_[ch], phase == 0 ? Factor * double(input[ch]) : 0., true, true);
        const double shaped = gain_ == 1 ? x : std::tanh(gain_ * x) / gain_;
        const double y = Tick(down_[ch], shaped, phase == 0, false);
        if (phase == 0) out = y;
      }
      input[ch] = static_cast<float>(out);
    }
    return input;
  }
private:
  struct Fir { std::array<double, 512> history{}; unsigned cursor = 0; };
  double Tick(Fir& fir, double x, bool output, bool sparse) {
    // Mirrored ring makes the convolution history contiguous without wrapping
    // each tap. Independent sums enable vectorization without fast-math.
    fir.history[fir.cursor] = fir.history[fir.cursor + 256] = x;
    double y = 0;
    if (output) {
      const unsigned first = sparse ? (fir.cursor & (Factor - 1)) : 0;
      const unsigned stride = sparse ? Factor : 1;
      const double* history = fir.history.data() + fir.cursor + 256;
      std::array<double, 4> sums{};
      unsigned i = first;
      if (!sparse || ((TapCount - 1 - first) % Factor) == first) {
        // Pair taps only when the mirrored index has the same residue:
        // all 2x phases, or even 4x phases. Other phases keep the full sum.
        for (; i + 3 * stride < Center; i += 4 * stride) {
          for (unsigned lane = 0; lane < 4; ++lane) {
            const unsigned tap = i + lane * stride;
            sums[lane] += taps_[tap] * (*(history - tap) + *(history - (TapCount - 1 - tap)));
          }
        }
        y = (sums[0] + sums[1]) + (sums[2] + sums[3]);
        for (; i < Center; i += stride)
          y += taps_[i] * (*(history - i) + *(history - (TapCount - 1 - i)));
        if (!sparse || first == 0) y += taps_[Center] * *(history - Center);
      } else {
        for (; i + 3 * stride < taps_.size(); i += 4 * stride) {
          for (unsigned lane = 0; lane < 4; ++lane) {
            const unsigned tap = i + lane * stride;
            sums[lane] += taps_[tap] * *(history - tap);
          }
        }
        y = (sums[0] + sums[1]) + (sums[2] + sums[3]);
        for (; i < taps_.size(); i += stride) y += taps_[i] * *(history - i);
      }
    }
    fir.cursor = (fir.cursor + 1) & 255;
    return y;
  }
  static constexpr unsigned TapCount = 32 * Factor + 1, Center = (TapCount - 1) / 2;
  std::array<double, TapCount> taps_{};
  std::array<Fir, 2> up_{}, down_{};
  double gain_ = 1, target_ = 1, slew_ = 0;
};
using PremiumDrive = FixedRatePremiumDrive<4>;
using PremiumDrive2x = FixedRatePremiumDrive<2>;
}
