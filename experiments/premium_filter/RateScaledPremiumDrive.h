// SPDX-License-Identifier: MIT
#pragma once
#include "PremiumDrive.h"

namespace sawstar::experimental {
// Research policy only: 4x below 176.4 kHz, 2x at/above 176.4 kHz.
// Factor is selected on Init, never switched in a live stream. Both paths
// retain 32 host samples of delay. This is not an approved shipping policy.
template<bool VectorFir = false, bool ReciprocalGain = false, bool Lookup = false> class BasicRateScaledPremiumDrive {
public:
  static constexpr int Latency = PremiumDrive::Latency;
  void Init(double rate) {
    high_ = SafeSampleRate(rate) >= 176400;
    if (high_) two_.Init(rate); else four_.Init(rate);
  }
  unsigned Factor() const { return high_ ? 2 : 4; }
  void Set(double db) { if (high_) two_.Set(db); else four_.Set(db); }
  void SnapToTargets() { if (high_) two_.SnapToTargets(); else four_.SnapToTargets(); }
  void Clear() { if (high_) two_.Clear(); else four_.Clear(); }
  std::array<float, 2> Process(std::array<float, 2> x) {
    return high_ ? two_.Process(x) : four_.Process(x);
  }
private:
  FixedRatePremiumDrive<4, true, false, VectorFir, ReciprocalGain, Lookup> four_;
  FixedRatePremiumDrive<2, true, false, VectorFir, ReciprocalGain, Lookup> two_;
  bool high_ = false;
};
using RateScaledPremiumDrive = BasicRateScaledPremiumDrive<>;
using RateScaledSimdPremiumDrive = BasicRateScaledPremiumDrive<true>;
// Opt-in normalization study; both factors retain the existing rate boundary.
using RateScaledGainPremiumDrive = BasicRateScaledPremiumDrive<true, true>;
// Distinct opt-in study: same rate/FIR/gain policy, lookup tanh only.
using RateScaledLookupPremiumDrive = BasicRateScaledPremiumDrive<true, true, true>;
}
