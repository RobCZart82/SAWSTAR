// SPDX-License-Identifier: MIT
#include "dsp/SevenSaw.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
void check(bool ok,const char* message){if(!ok){std::cerr<<message<<'\n';std::exit(1);}}
int main(){
  constexpr int length=8192;
  constexpr double tau=6.283185307179586;
  std::cout<<"sample_rate,waveform,frequency,peak,rms,dc,residual_fraction\n";
  for(float sr:{44100.f,48000.f,96000.f})for(int wave=0;wave<4;++wave)for(float requested:{110.f,440.f,4000.f,8000.f}){
    const int bin=static_cast<int>(std::round(requested*length/sr));
    const float hz=bin*sr/length;
    sawstar::SevenSaw oscillator;oscillator.Init(sr);oscillator.SetFreq(hz);oscillator.SetWaveform(wave);
    for(int i=0;i<int(sr);++i)oscillator.Process();
    double power=0,dc=0,re=0,im=0;float peak=0;
    for(int i=0;i<length;++i){const auto x=oscillator.Process();
      check(std::isfinite(x.left)&&x.left==x.right&&std::abs(x.left)<2.1f,"high register source stays finite, centered and bounded");
      power+=double(x.left)*x.left;dc+=x.left;peak=std::max(peak,std::abs(x.left));
      re+=x.left*std::cos(tau*bin*i/length);im+=x.left*std::sin(tau*bin*i/length);
    }
    const double residual=std::max(0.,1-2*(re*re+im*im)/(length*power));
    check(power>1.e-3,"source remains audible at high pitch");
    if(wave==3)check(residual<.001,"sine has less than 0.1 percent non-fundamental energy");
    // For other waves residual includes intended harmonics; it is not an alias score.
    std::cout<<sr<<','<<wave<<','<<hz<<','<<peak<<','<<std::sqrt(power/length)<<','<<dc/length<<','<<residual<<'\n';
  }
}
