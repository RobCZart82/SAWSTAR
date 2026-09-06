// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/SevenSaw.h"
#include <array>
namespace sawstar {
// Stereo 12 dB/oct TPT state-variable low-pass. Audio-thread owned.
class LowPass {
public:
  void Init(float sampleRate);
  void Clear();
  void Set(float cutoffHz, float resonancePercent, float mixPercent);
  StereoSample Process(StereoSample input);
private:
  std::array<double,2> ic1_{},ic2_{};
  double rate_=44100,slew_=0.002,g_=1,targetG_=1,k_=2,targetK_=2,mix_=0,targetMix_=0;
  float cutoff_=-1;
};
}
