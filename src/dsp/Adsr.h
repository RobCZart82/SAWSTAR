// SPDX-License-Identifier: MIT
#pragma once

#include <cmath>
#include <cstdint>

namespace sawstar {

// SAWSTAR's one-pole ADSR. The segment coefficients and public float output
// preserve the existing DaisySP response; only the accumulated envelope state
// uses double precision so tiny long-time increments do not stop advancing.
class Adsr {
public:
  enum Segment : std::uint8_t {
    Idle = 0,
    Attack = 1,
    Decay = 2,
    Release = 4
  };

  void Init(float sampleRate, int blockSize = 1) {
    sampleRate_ = static_cast<int>(sampleRate / blockSize);
    attackShape_ = -1.f;
    attackTarget_ = 0.f;
    attackTime_ = decayTime_ = releaseTime_ = -1.f;
    sustain_ = .7f;
    level_ = 0.0;
    gate_ = false;
    segment_ = Idle;
    SetAttackTime(.1f);
    SetDecayTime(.1f);
    SetReleaseTime(.1f);
  }

  void Retrigger(bool hard) {
    segment_ = Attack;
    if (hard) level_ = 0.0;
  }

  void SetAttackTime(float seconds, float shape = 0.f) {
    if (seconds == attackTime_ && shape == attackShape_) return;
    attackTime_ = seconds;
    attackShape_ = shape;
    attackHighPrecision_ = seconds > 1.f;
    if (seconds > 0.f) {
      const float target = 9.f * std::pow(shape, 10.f) + .3f * shape + 1.01f;
      attackTarget_ = target;
      const float logTarget = std::log(1.f - (1.f / target));
      if (seconds <= 1.f) {
        attackCoefficient_ = 1.f - std::exp(logTarget / (seconds * sampleRate_));
      } else {
        attackCoefficient_ = 1.0 - std::exp(static_cast<double>(logTarget) /
            (static_cast<double>(seconds) * sampleRate_));
      }
    } else {
      attackCoefficient_ = 1.0;
    }
  }

  void SetDecayTime(float seconds) {
    SetCoefficient(seconds, decayTime_, decayCoefficient_);
    decayHighPrecision_ = seconds > 1.f;
  }

  void SetReleaseTime(float seconds) {
    SetCoefficient(seconds, releaseTime_, releaseCoefficient_);
    releaseHighPrecision_ = seconds > 1.f;
  }

  void SetSustainLevel(float value) {
    sustain_ = value <= 0.f ? -.01f : value > 1.f ? 1.f : value;
  }

  std::uint8_t GetCurrentSegment() const { return segment_; }
  bool IsRunning() const { return segment_ != Idle; }

  float Process(bool gate) {
    if (gate && !gate_) segment_ = Attack;
    else if (!gate && gate_) segment_ = Release;
    gate_ = gate;

    float coefficient = attackCoefficient_;
    if (segment_ == Decay) coefficient = decayCoefficient_;
    else if (segment_ == Release) coefficient = releaseCoefficient_;
    const float target = segment_ == Decay ? sustain_ : -.01f;

    switch (segment_) {
      case Idle:
        return 0.f;
      case Attack: {
        if (!attackHighPrecision_) {
          float value = static_cast<float>(level_);
          value += static_cast<float>(attackCoefficient_) * (attackTarget_ - value);
          level_ = value;
          if (value > 1.f) {
            level_ = 1.0;
            value = 1.f;
            segment_ = Decay;
          }
          return value;
        }
        level_ += static_cast<double>(coefficient) *
                  (static_cast<double>(attackTarget_) - level_);
        float output = static_cast<float>(level_);
        if (output > 1.f) {
          level_ = 1.0;
          output = 1.f;
          segment_ = Decay;
        }
        return output;
      }
      case Decay:
      case Release: {
        const bool highPrecision = segment_ == Decay
            ? decayHighPrecision_ : releaseHighPrecision_;
        if (!highPrecision) {
          float value = static_cast<float>(level_);
          value += static_cast<float>(coefficient) * (target - value);
          level_ = value;
          if (value < 0.f) {
            level_ = 0.0;
            value = 0.f;
            segment_ = Idle;
          }
          return value;
        }
        level_ += static_cast<double>(coefficient) *
                  (static_cast<double>(target) - level_);
        float output = static_cast<float>(level_);
        if (output < 0.f) {
          level_ = 0.0;
          output = 0.f;
          segment_ = Idle;
        }
        return output;
      }
      default:
        return 0.f;
    }
  }

private:
  void SetCoefficient(float seconds, float& previous, double& coefficient) {
    if (seconds == previous) return;
    previous = seconds;
    if (seconds <= 0.f) coefficient = 1.0;
    else if (seconds <= 1.f)
      coefficient = 1.f - std::exp(-1.f / (seconds * sampleRate_));
    else
      coefficient = 1.0 - std::exp(-1.0 / (static_cast<double>(seconds) * sampleRate_));
  }

  float sustain_ = 0.f;
  double level_ = 0.0;
  float attackShape_ = -1.f;
  float attackTarget_ = 0.f;
  float attackTime_ = -1.f;
  float decayTime_ = -1.f;
  float releaseTime_ = -1.f;
  double attackCoefficient_ = 0.0;
  double decayCoefficient_ = 0.0;
  double releaseCoefficient_ = 0.0;
  int sampleRate_ = 44100;
  std::uint8_t segment_ = Idle;
  bool gate_ = false;
  bool attackHighPrecision_ = false;
  bool decayHighPrecision_ = false;
  bool releaseHighPrecision_ = false;
};

} // namespace sawstar
