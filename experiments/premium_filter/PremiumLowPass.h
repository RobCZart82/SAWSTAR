// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/Safety.h"
#ifdef SAWSTAR_FILTER_STATE_TRACE
#include "engine/FilterStateTrace.h"
#endif
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace sawstar::experimental {
// Linear four-mode research candidate. Not selected by the shipped engine.
// Independently implemented trapezoidal SVF stages; damping at zero resonance
// factors the fourth-order Butterworth denominator. No copied library code.
class PremiumLowPass {
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
      controlsMoving_ = true;
      cutoff_ = cutoff;
      targetG_ = std::tan(3.14159265358979323846 * cutoff / rate_);
    }
    resonance = FiniteClamp(resonance, 0., 100., 0.);
    if (resonance != resonance_) {
      controlsMoving_ = true;
      resonance_ = resonance;
      targetK_ = 1.8477590650225735 * std::pow(.1 / 1.8477590650225735, resonance / 100.);
      targetK12_ = std::sqrt(2.) * std::pow(.1 / std::sqrt(2.), resonance / 100.);
    }
  }
  // Existing parameter contract: LP12=0, LP24=1, HP12=2, BP12=3.
  void SetMode(int mode) { mode_ = std::clamp(mode, 0, 3); }
  void SnapToTargets() {
    g_ = targetG_; k_ = targetK_; k12_ = targetK12_;
    UpdateCoefficients(); controlsMoving_ = false;
    weights_ = {}; weights_[mode_] = 1;
  }
  void Clear() {
    for (auto& channel : stages_) channel = {};
    twelve_ = {};
  }
#ifdef SAWSTAR_FILTER_STATE_TRACE
  void CaptureState(::sawstar::FilterStateTrace& out) const {
    out.Add("filter.mode",0,mode_);out.Add("filter.controls_moving",0,controlsMoving_);
    const std::array<double,13> controls{rate_,slew_,cutoff_,resonance_,g_,targetG_,k_,targetK_,k12_,targetK12_,a1_,a2_,a12_};
    constexpr std::array<const char*,13> names{"filter.rate","filter.slew","filter.cutoff","filter.resonance","filter.g","filter.target_g","filter.k","filter.target_k","filter.k12","filter.target_k12","filter.a1","filter.a2","filter.a12"};
    for(unsigned i=0;i<names.size();++i)out.Add(names[i],0,controls[i]);
    for(unsigned i=0;i<4;++i)out.Add("filter.weights",i,weights_[i]);
    for(unsigned ch=0;ch<2;++ch){
      for(unsigned stage=0;stage<2;++stage){out.Add("filter.integrator.band",ch*3+stage,stages_[ch][stage].band);out.Add("filter.integrator.low",ch*3+stage,stages_[ch][stage].low);}
      out.Add("filter.integrator.band",ch*3+2,twelve_[ch].band);out.Add("filter.integrator.low",ch*3+2,twelve_[ch].low);
    }
    out.Append(operations_);
  }
#endif
  std::array<float, 2> Process(std::array<float, 2> input) {
#ifdef SAWSTAR_FILTER_STATE_TRACE
    operations_.Clear();traceStep_=0;
#endif
    if (controlsMoving_) {
      const double previousG = g_, previousK = k_, previousK12 = k12_;
      g_ += slew_ * (targetG_ - g_);
      k_ += slew_ * (targetK_ - k_);
      k12_ += slew_ * (targetK12_ - k12_);
      // Only stop when the original recurrence no longer changes any value.
      // No tolerance, quantization or shortened ramp changes the sound.
      controlsMoving_ = g_ != previousG || k_ != previousK || k12_ != previousK12;
      UpdateCoefficients();
    }
    for (size_t i = 0; i < weights_.size(); ++i)
      weights_[i] += slew_ * ((static_cast<int>(i) == mode_ ? 1. : 0.) - weights_[i]);
    for (size_t i = 0; i < weights_.size(); ++i) {
      // Preserve total weight while ending negligible tails before denormals.
      if (static_cast<int>(i) != mode_ && weights_[i] < 1e-24) {
        weights_[mode_] += weights_[i]; weights_[i] = 0;
      }
    }
    for (size_t ch = 0; ch < input.size(); ++ch) {
      if (!std::isfinite(input[ch])) {
        stages_[ch] = {};
        twelve_[ch] = {};
        input[ch] = 0;
        continue;
      }
      const double first = Step(input[ch], stages_[ch][0], g_, a1_);
      const double lp24 = Step(first, stages_[ch][1], g_, a2_);
      // Keep both networks running so mode changes never expose stale state.
      double band = 0;
      const double low = Step(input[ch], twelve_[ch], g_, a12_, &band);
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
  void UpdateCoefficients() {
    a1_ = 1. / (1. + g_ * (g_ + k_));
    constexpr double k2 = .7653668647301795;
    a2_ = 1. / (1. + g_ * (g_ + k2));
    a12_ = 1. / (1. + g_ * (g_ + k12_));
  }
  struct State { double band = 0, low = 0; };
#ifndef SAWSTAR_FILTER_STATE_TRACE
  static
#endif
  double Step(double x, State& state, double g, double a, double* bandOut = nullptr) {
#ifdef SAWSTAR_FILTER_STATE_TRACE
    const unsigned at=traceStep_++;
    operations_.Add("filter.step.input",at,x);
    operations_.Add("filter.step.before.band",at,state.band);
    operations_.Add("filter.step.before.low",at,state.low);
#endif
    const double band = a * (state.band + g * (x - state.low));
    const double low = state.low + g * band;
#ifdef SAWSTAR_FILTER_STATE_TRACE
    operations_.Add("filter.step.band",at,band);operations_.Add("filter.step.low",at,low);
#endif
    state.band = 2 * band - state.band;
    state.low = 2 * low - state.low;
    if (std::abs(state.band) < 1e-24) state.band = 0;
    if (std::abs(state.low) < 1e-24) state.low = 0;
    if (bandOut) *bandOut = band;
    return low;
  }
#ifdef SAWSTAR_FILTER_STATE_TRACE
  ::sawstar::NamedStateTrace<32> operations_{};
  unsigned traceStep_=0;
#endif
  std::array<std::array<State, 2>, 2> stages_{};
  std::array<State, 2> twelve_{};
  std::array<double, 4> weights_{0, 1, 0, 0};
  int mode_ = 1;
  double rate_ = 44100, slew_ = 0, cutoff_ = -1, resonance_ = -1;
  double g_ = 1, targetG_ = 1, k_ = 2, targetK_ = 2;
  double k12_ = 2, targetK12_ = 2;
  bool controlsMoving_ = true;
  double a1_ = 0, a2_ = 0, a12_ = 0;
};
}
