// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/Safety.h"
#include <algorithm>
#include <cmath>
namespace sawstar {
// Mid remains unchanged; only existing stereo information is widened.
class Width {
  float extra_=0, target_=0, step_=1.f/960;
public:
  void Init(float sr){sr=SafeSampleRate(sr);extra_=target_=0;step_=1.f/(.02f*std::max(sr,8000.f));}
  void Set(bool on,float percent){target_=on&&std::isfinite(percent)?std::clamp(percent*.01f,0.f,1.f):0.f;}
  void Process(float& left,float& right){
    extra_+=std::clamp(target_-extra_,-step_,step_);
    if(extra_==0)return; // exact bypass after the 20 ms ramp
    const float side=(left-right)*.5f*extra_;
    left+=side;right-=side;
  }
};
}
