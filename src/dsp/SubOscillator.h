// SPDX-License-Identifier: MIT
#pragma once
#include <algorithm>
#include <cmath>
namespace sawstar {
// Mono sub source. Triangle uses eight odd harmonics below Nyquist;
// square uses a polyBLEP edge correction. State is always initialized.
class SubOscillator {
 double phase_=0,step_=0,sr_=48000;int wave_=0;
 double Blep(double t)const{if(t<step_){t/=step_;return 2*t-t*t-1;}if(t>1-step_){t=(t-1)/step_;return t*t+2*t+1;}return 0;}
public:
 void Init(float sr){sr_=std::max(1.f,sr);phase_=0;step_=0;wave_=0;}
 void SetFreq(float hz){step_=std::clamp(double(hz)/sr_,0.,.45);}
 void SetWaveform(int wave){wave_=std::clamp(wave,0,2);}
 float Process(){constexpr double pi=3.14159265358979323846;double out=0;
 if(wave_==0)out=std::sin(2*pi*phase_);
 else if(wave_==1){for(int k=0;k<8;++k){int n=2*k+1;if(n*step_>=.5)break;out+=(k%2?-1.:1.)*std::sin(2*pi*n*phase_)/(n*n);}out*=8/(pi*pi);}
 else out=(phase_<.5?1.:-1.)+Blep(phase_)-Blep(std::fmod(phase_+.5,1.));
 phase_+=step_;if(phase_>=1)phase_-=1;return float(out);
 }
};
}
