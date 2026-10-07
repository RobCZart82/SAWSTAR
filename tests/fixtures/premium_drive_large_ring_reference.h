// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/Safety.h"
#include "../../experiments/premium_filter/ResearchTanh.h"
#include "../../experiments/premium_filter/FirLanes4.h"
#include <array>
#include <cmath>

namespace sawstar::ring_reference {
// Frozen c5f5d6351b7dbed9e30af329c99934dcda5cc7f6, before the 2x ring reduction.
// Only namespace/include qualification differs. Shared tanh/SIMD primitives
// are unchanged by the ring study; preserve this fixture's 256-slot ring.
// Research-only fixed-factor waveshaper, with explicit interpolation and anti-alias FIRs.
// Interpolation keeps only host-rate samples, never inserting/storing zeros.
// Only retained decimator phases convolve. ReferencePremiumDrive is the full oracle.
// Nonlinear=false is an offline cost control, never a selectable sound mode.
// VectorFir=true is a separately qualified research candidate; default remains scalar.
template<unsigned Factor, bool Nonlinear = true, bool Research = false, bool VectorFir = false> class FixedRatePremiumDrive {
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
    for (unsigned phase = 0; phase < Factor; ++phase)
      for (unsigned i = 0; i < PhaseTapCount; ++i) {
        const unsigned tap = phase + i * Factor;
        phaseTaps_[phase][i] = tap < TapCount ? taps_[tap] : 0.;
      }
    gain_ = target_ = 1; Clear();
  }
  void Set(double db) { target_ = std::pow(10., FiniteClamp(db, 0., 24., 0.) / 20); }
  void SnapToTargets() { gain_ = target_; }
  void Clear() { up_ = {}; down_ = {}; }
  std::array<float, 2> Process(std::array<float, 2> input) {
    gain_ += slew_ * (target_ - gain_);
    for (size_t ch = 0; ch < 2; ++ch) {
      if (!std::isfinite(input[ch])) { up_[ch] = {}; down_[ch] = {}; input[ch] = 0; }
      auto& up = up_[ch];
      up.history[up.cursor] = up.history[up.cursor + HostRingSize] = Factor * double(input[ch]);
      // Phase and decimator-output choices are compile-time constants. Keep
      // the original evaluation order and consume every oversampled phase.
      const double out = ProcessPhase<0>(up, down_[ch]);
      ProcessPhase<1>(up, down_[ch]);
      if constexpr (Factor == 4) {
        ProcessPhase<2>(up, down_[ch]);
        ProcessPhase<3>(up, down_[ch]);
      }
      up.cursor = (up.cursor + 1) & (HostRingSize - 1);
      input[ch] = static_cast<float>(out);
    }
    return input;
  }
private:
  // At most 33 nonzero host samples are needed by every interpolation phase.
  // Mirroring 64 slots keeps the compact convolution contiguous through wrap.
  static constexpr unsigned HostRingSize = 64;
  struct HostFir { std::array<double, 2 * HostRingSize> history{}; unsigned cursor = 0; };
  struct Fir { std::array<double, 512> history{}; unsigned cursor = 0; };
  template<unsigned Phase> double ProcessPhase(const HostFir& up, Fir& down) {
    const double x = Interpolate<Phase>(up);
    double shaped = x;
    if constexpr (Nonlinear) {
      if (gain_ != 1) {
        if constexpr (Research) shaped = sawstar::experimental::ResearchTanh(gain_ * x) / gain_;
        else shaped = std::tanh(gain_ * x) / gain_;
      }
    }
    return Tick<Phase == 0>(down, shaped);
  }
  template<unsigned Phase> double Interpolate(const HostFir& fir) const {
    static_assert(Phase < Factor, "Invalid interpolation phase");
    const double* history = fir.history.data() + fir.cursor + HostRingSize;
    const double* taps = phaseTaps_[Phase].data();
    if constexpr (VectorFir) {
      sawstar::experimental::detail::FirLanes4 sums;
      if constexpr (((TapCount - 1 - Phase) % Factor) == Phase) {
        constexpr unsigned last = Phase == 0 ? PhaseTapCount - 1 : PhaseTapCount - 2;
        for (unsigned i = 0; i < HalfPhase; i += 4)
          sums.Symmetric(taps + i, history - i, history - last + i);
        double y = sums.Sum();
        if constexpr (Phase == 0) y += taps[HalfPhase] * *(history - HalfPhase);
        return y;
      } else {
        for (unsigned i = 0; i < PhaseTapCount - 1; i += 4)
          sums.Straight(taps + i, history - i);
        return sums.Sum();
      }
    }
    std::array<double, 4> sums{};
    double y = 0;
    if constexpr (((TapCount - 1 - Phase) % Factor) == Phase) {
      // Keep the preceding sparse FIR's lane sums and addition order exactly.
      constexpr unsigned last = Phase == 0 ? PhaseTapCount - 1 : PhaseTapCount - 2;
      for (unsigned i = 0; i < HalfPhase; i += 4) {
        for (unsigned lane = 0; lane < 4; ++lane) {
          const unsigned tap = i + lane;
          sums[lane] += taps[tap] * (*(history - tap) + *(history - (last - tap)));
        }
      }
      y = (sums[0] + sums[1]) + (sums[2] + sums[3]);
      if constexpr (Phase == 0) y += taps[HalfPhase] * *(history - HalfPhase);
    } else {
      // The odd 4x phases each have 32 taps; both arrays are contiguous.
      for (unsigned i = 0; i < PhaseTapCount - 1; i += 4) {
        for (unsigned lane = 0; lane < 4; ++lane) {
          const unsigned tap = i + lane;
          sums[lane] += taps[tap] * *(history - tap);
        }
      }
      y = (sums[0] + sums[1]) + (sums[2] + sums[3]);
    }
    return y;
  }
  template<bool Output> double Tick(Fir& fir, double x) {
    // Mirrored ring makes the convolution history contiguous without wrapping
    // each tap. Independent sums enable vectorization without fast-math.
    fir.history[fir.cursor] = fir.history[fir.cursor + 256] = x;
    double y = 0;
    if constexpr (Output) {
      const double* history = fir.history.data() + fir.cursor + 256;

      if constexpr (VectorFir) {
        sawstar::experimental::detail::FirLanes4 sums;
        for (unsigned i = 0; i < Center; i += 4)
          sums.Symmetric(taps_.data() + i, history - i, history - (TapCount - 1) + i);
        y = sums.Sum();
        y += taps_[Center] * *(history - Center);
      } else {
        std::array<double, 4> sums{};
        unsigned i = 0;
        for (; i + 3 < Center; i += 4) {
          for (unsigned lane = 0; lane < 4; ++lane) {
            const unsigned tap = i + lane;
            sums[lane] += taps_[tap] * (*(history - tap) + *(history - (TapCount - 1 - tap)));
          }
        }
        y = (sums[0] + sums[1]) + (sums[2] + sums[3]);
        for (; i < Center; ++i)
          y += taps_[i] * (*(history - i) + *(history - (TapCount - 1 - i)));
        y += taps_[Center] * *(history - Center);
      }
    }
    fir.cursor = (fir.cursor + 1) & 255;
    return y;
  }
  static constexpr unsigned TapCount = 32 * Factor + 1, Center = (TapCount - 1) / 2;
  static constexpr unsigned PhaseTapCount = 33, HalfPhase = 16;
  std::array<double, TapCount> taps_{};
  std::array<std::array<double, PhaseTapCount>, Factor> phaseTaps_{};
  std::array<HostFir, 2> up_{};
  std::array<Fir, 2> down_{};
  double gain_ = 1, target_ = 1, slew_ = 0;
};
using PremiumDrive = FixedRatePremiumDrive<4>;
using PremiumDrive2x = FixedRatePremiumDrive<2>;
}
