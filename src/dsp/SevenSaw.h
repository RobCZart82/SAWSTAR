// SPDX-License-Identifier: MIT
#pragma once
#include "Synthesis/oscillator.h"
#include <array>
namespace sawstar {
struct StereoSample { float left=0, right=0; };
// SAWSTAR's unison tuning/mix/pan layer over seven DaisySP polyBLEP primitives.
class SevenSaw {
public:
  void Init(float sampleRate);
  void SetFreq(float hz);
  void SetShape(float detuneCents, float mix, float width);
  StereoSample Process();
private:
  std::array<daisysp::Oscillator,7> saws_;
  std::array<float,7> ratios_{{1,1,1,1,1,1,1}}, targets_{{1,1,1,1,1,1,1}};
  float rate_=44100, hz_=100, slew_=0.004f;
  float detune_=-1, mix_=0, width_=0, targetMix_=0, targetWidth_=0;
};
}
