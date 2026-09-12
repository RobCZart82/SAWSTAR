// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/SevenSaw.h"
#include <array>
namespace sawstar {
// Stereo TPT filter bank. Audio-thread owned; mode 0 preserves the original low-pass.
class LowPass {
public:
  void Init(float sampleRate);
  void Clear();
  void SnapToTargets(){g_=targetG_;k_=targetK_;mix_=targetMix_;drive_=targetDrive_;weights_.fill(0);weights_[mode_]=1;}
  void Set(float cutoffHz, float resonancePercent, float mixPercent);
  void SetCharacter(float driveDb, int mode);
  StereoSample Process(StereoSample input);
private:
  std::array<double,2> dcInput_{},dcOutput_{};
  double dcPole_=0;
  std::array<double,2> ic1_{},ic2_{},cascade1_{},cascade2_{};
  std::array<double,4> weights_{{1,0,0,0}};
  int mode_=0;
  double drive_=0,targetDrive_=0;
  double rate_=44100,slew_=0.002,g_=1,targetG_=1,k_=2,targetK_=2,mix_=0,targetMix_=0;
  float cutoff_=-1;
};
}
