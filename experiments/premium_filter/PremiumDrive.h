// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/Safety.h"
#include <array>
#include <cmath>

namespace sawstar::experimental {
// Research-only 4x waveshaper, with explicit interpolation and anti-alias FIRs.
// Polyphase interpolation skips known zero inputs; only retained decimator
// phases compute a convolution. ReferencePremiumDrive retains the full version.
class PremiumDrive {
public:
  static constexpr int Latency = 32;
  void Init(double rate) {
    slew_ = 1 - std::exp(-1 / (.01 * SafeSampleRate(rate)));
    constexpr double pi = 3.14159265358979323846, cutoff = .1125;
    double sum = 0;
    for (int i = 0; i < 129; ++i) {
      const int n = i - 64;
      const double sinc = n == 0 ? 2 * cutoff : std::sin(2 * pi * cutoff * n) / (pi * n);
      const double window = .42 - .5 * std::cos(2 * pi * i / 128) + .08 * std::cos(4 * pi * i / 128);
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
      for (int phase = 0; phase < 4; ++phase) {
        const double x = Tick(up_[ch], phase == 0 ? 4. * input[ch] : 0., true, true);
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
      const unsigned first = sparse ? (fir.cursor & 3) : 0;
      const unsigned stride = sparse ? 4 : 1;
      const double* history = fir.history.data() + fir.cursor + 256;
      std::array<double, 4> sums{};
      unsigned i = first;
      for (; i + 3 * stride < taps_.size(); i += 4 * stride) {
        for (unsigned lane = 0; lane < 4; ++lane) {
          const unsigned tap = i + lane * stride;
          sums[lane] += taps_[tap] * *(history - tap);
        }
      }
      y = (sums[0] + sums[1]) + (sums[2] + sums[3]);
      for (; i < taps_.size(); i += stride) y += taps_[i] * *(history - i);
    }
    fir.cursor = (fir.cursor + 1) & 255;
    return y;
  }
  std::array<double, 129> taps_{};
  std::array<Fir, 2> up_{}, down_{};
  double gain_ = 1, target_ = 1, slew_ = 0;
};
}
