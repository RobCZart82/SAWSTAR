// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/Safety.h"
#include <algorithm>
#include <array>
#include <cmath>
namespace sawstar {
// Mono sub source. All shapes share one phase; changes crossfade over 10 ms.
// Triangle uses eight odd harmonics below Nyquist; square uses polyBLEP edges.
class SubOscillator {
  double phase_=0,step_=0,sr_=48000;
  int wave_=0,remaining_=0;
  bool started_=false;
  std::array<double,3> weights_{{1,0,0}};
  double Blep(double t)const {
    if(t<step_){t/=step_;return 2*t-t*t-1;}
    if(t>1-step_){t=(t-1)/step_;return t*t+2*t+1;}
    return 0;
  }
  double Sample(int wave)const {
    constexpr double pi=3.14159265358979323846;
    if(wave==0)return std::sin(2*pi*phase_);
    if(wave==2)return (phase_<.5?1.:-1.)+Blep(phase_)-Blep(std::fmod(phase_+.5,1.));
    double out=0;
    for(int k=0;k<8;++k){int n=2*k+1;if(n*step_>=.5)break;out+=(k%2?-1.:1.)*std::sin(2*pi*n*phase_)/(n*n);}
    return out*8/(pi*pi);
  }
public:
  void Init(float sr) {
    sr_=SafeSampleRate(sr);phase_=0;step_=0;wave_=0;remaining_=0;
    started_=false;weights_={1,0,0};
  }
  void SetFreq(float hz){step_=std::clamp(double(FiniteClamp(hz,0.f,20000.f,0.f))/sr_,0.,.45);}
  void SetWaveform(int wave) {
    wave=std::clamp(wave,0,2);
    if(wave==wave_)return; // Repeated block updates must not restart the fade.
    wave_=wave;
    if(!started_){weights_={0,0,0};weights_[wave_]=1;return;}
    // Retarget from the current mixture, including a partially completed fade.
    remaining_=std::max(1,int(sr_*.01));
  }
  void SnapToTargets(){weights_={0,0,0};weights_[wave_]=1;remaining_=0;}
  float Process() {
    started_=true;
    if(remaining_>0) {
      for(int i=0;i<3;++i)weights_[i]+=((i==wave_?1.:0.)-weights_[i])/remaining_;
      if(--remaining_==0){weights_={0,0,0};weights_[wave_]=1;}
    }
    double out=0;
    for(int i=0;i<3;++i)if(weights_[i]>0)out+=weights_[i]*Sample(i);
    phase_+=step_;if(phase_>=1)phase_-=1;
    return float(out);
  }
};
}
