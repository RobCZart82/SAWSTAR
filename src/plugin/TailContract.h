// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/Safety.h"
#include <cmath>

namespace sawstar {
// Fixed maximum, rather than the current preset: shortening the report could
// discard delay/reverb history left by earlier settings. This changes host
// scheduling only; it never truncates, fades or resets the audio signal.
inline double TailBudgetSeconds() {
  constexpr double threshold = 1.e-6; // -120 dB amplitude, finite engineering tail
  constexpr double maxRelease = 10.;
  constexpr double maxDelay = 2., maxFeedback = .85;
  constexpr double maxReverbDecay = 10.; // RT60, not time to exact silence
  // ADSR releases toward -.01; from level 1, zero is reached after ln(101)*T.
  const double envelope = maxRelease * std::log(101.);
  const double chorus = .021; // largest 17+4 ms feed-forward tap
  // Include the first echo and the geometric accumulation of feedback history.
  const double delay = maxDelay * (1. + std::ceil(
      std::log(threshold * (1. - maxFeedback)) / std::log(maxFeedback)));
  // Pessimistic FDN memory/feedback bound using the longest possible tap and
  // shortest possible tap, including the complete size-smoothing range.
  const double reverb = 2. * maxReverbDecay * (.0739 * 1.6) / (.0297 * .6);
  // One second for parameter settling/interpolation, rounded upward to 10 s.
  // This is a tested conservative budget, not a bound on ongoing MIDI or
  // arbitrary automation which keeps exciting a time-varying delay network.
  return 10. * std::ceil((envelope + chorus + delay + reverb + 1.) / 10.);
}

inline int TailSamples(double sampleRate) {
  // Same rate limits as the DSP, retained in double for fractional host rates.
  // At 384 kHz this is below iPlug's INT_MAX infinite-tail sentinel.
  const double rate = FiniteClamp(sampleRate, 8000., 384000., 44100.);
  return static_cast<int>(std::ceil(TailBudgetSeconds() * rate));
}
}
