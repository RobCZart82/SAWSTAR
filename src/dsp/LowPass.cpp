// SPDX-License-Identifier: MIT
#include "dsp/LowPass.h"
#include <algorithm>
#include <cmath>
namespace sawstar {
namespace {
float Safe(float v,float lo,float hi){return std::isfinite(v)?std::clamp(v,lo,hi):lo;}
}
void LowPass::Init(float sampleRate){
  rate_=std::isfinite(sampleRate)&&sampleRate>=8000?sampleRate:44100;
  slew_=1-std::exp(-1/(0.01*rate_));cutoff_=-1;
  Set(12000,0,0);g_=targetG_;k_=targetK_;mix_=0;Clear();
}
void LowPass::Clear(){ic1_.fill(0);ic2_.fill(0);}
void LowPass::Set(float hz,float resonance,float mix){
  hz=Safe(hz,20,static_cast<float>(std::min(20000.,rate_*0.45)));
  if(hz!=cutoff_){cutoff_=hz;targetG_=std::tan(3.14159265358979323846*hz/rate_);}
  targetK_=2-1.9*Safe(resonance,0,100)*0.01; // Q=0.5 ... 10; no self-oscillation.
  targetMix_=Safe(mix,0,100)*0.01;
}
StereoSample LowPass::Process(StereoSample input){
  g_+=slew_*(targetG_-g_);k_+=slew_*(targetK_-k_);mix_+=slew_*(targetMix_-mix_);
  if(targetMix_==0 && mix_<1e-9)mix_=0;
  const double a=1/(1+g_*(g_+k_));
  std::array<float,2> samples{{input.left,input.right}};
  for(size_t ch=0;ch<2;++ch){
    if(!std::isfinite(samples[ch])){ic1_[ch]=ic2_[ch]=0;samples[ch]=0;continue;}
    const double v1=a*(ic1_[ch]+g_*(samples[ch]-ic2_[ch]));
    const double v2=ic2_[ch]+g_*v1;
    ic1_[ch]=2*v1-ic1_[ch];ic2_[ch]=2*v2-ic2_[ch];
    // Keep long decays out of the denormal range without depending on host flags.
    if(std::abs(ic1_[ch])<1e-24)ic1_[ch]=0;
    if(std::abs(ic2_[ch])<1e-24)ic2_[ch]=0;
    samples[ch]=static_cast<float>(samples[ch]+mix_*(v2-samples[ch]));
  }
  return {samples[0],samples[1]};
}
}
