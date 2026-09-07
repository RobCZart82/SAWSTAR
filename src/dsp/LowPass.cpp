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
  dcPole_=std::exp(-2*3.14159265358979323846*5/rate_);
  slew_=1-std::exp(-1/(0.01*rate_));cutoff_=-1;
  mode_=0;weights_={{1,0,0,0}};drive_=targetDrive_=0;
  Set(12000,0,0);g_=targetG_;k_=targetK_;mix_=0;Clear();
}
void LowPass::Clear(){dcInput_.fill(0);dcOutput_.fill(0);ic1_.fill(0);ic2_.fill(0);cascade1_.fill(0);cascade2_.fill(0);}
void LowPass::SetCharacter(float driveDb,int mode){targetDrive_=Safe(driveDb,0,24);mode_=std::clamp(mode,0,3);}
void LowPass::Set(float hz,float resonance,float mix){
  hz=Safe(hz,20,static_cast<float>(std::min(20000.,rate_*0.45)));
  if(hz!=cutoff_){cutoff_=hz;targetG_=std::tan(3.14159265358979323846*hz/rate_);}
  targetK_=2-1.9*Safe(resonance,0,100)*0.01; // Q=0.5 ... 10; no self-oscillation.
  targetMix_=Safe(mix,0,100)*0.01;
}
StereoSample LowPass::Process(StereoSample input){
  g_+=slew_*(targetG_-g_);k_+=slew_*(targetK_-k_);mix_+=slew_*(targetMix_-mix_);
  if(targetMix_==0 && mix_<1e-9)mix_=0;
  drive_+=slew_*(targetDrive_-drive_);
  if(targetDrive_==0 && drive_<1e-9)drive_=0;
  for(size_t i=0;i<weights_.size();++i)weights_[i]+=slew_*((static_cast<int>(i)==mode_?1.:0.)-weights_[i]);
  const double driveGain=drive_==0?1.:std::pow(10.,drive_/20.);
  const double driveDenominator=drive_==0?1.:std::tanh(driveGain);
  const double driveBlend=std::min(1.,drive_/6.);
  const double a=1/(1+g_*(g_+k_)),cascadeA=1/(1+g_*(g_+2.));
  std::array<float,2> samples{{input.left,input.right}};
  for(size_t ch=0;ch<2;++ch){
    if(!std::isfinite(samples[ch])){ic1_[ch]=ic2_[ch]=cascade1_[ch]=cascade2_[ch]=dcInput_[ch]=dcOutput_[ch]=0;samples[ch]=0;continue;}
    const double dry=samples[ch];
    double driven=drive_==0?dry:dry+driveBlend*(std::tanh(dry*driveGain)/driveDenominator-dry);
    // Remove saturation-induced DC at 5 Hz. Follow the drive blend so zero
    // drive retains the exact clean path, while histories remain warm.
    const double dcFree=driven-dcInput_[ch]+dcPole_*dcOutput_[ch];
    dcInput_[ch]=driven;dcOutput_[ch]=std::abs(dcFree)<1.e-24?0:dcFree;
    driven+=driveBlend*(dcFree-driven);
    const double v1=a*(ic1_[ch]+g_*(driven-ic2_[ch]));
    const double v2=ic2_[ch]+g_*v1;
    ic1_[ch]=2*v1-ic1_[ch];ic2_[ch]=2*v2-ic2_[ch];
    // Keep long decays out of the denormal range without depending on host flags.
    if(std::abs(ic1_[ch])<1e-24)ic1_[ch]=0;
    if(std::abs(ic2_[ch])<1e-24)ic2_[ch]=0;
    // A non-resonant second stage gives LP24 its extra slope without squaring resonance.
    const double c1=cascadeA*(cascade1_[ch]+g_*(v2-cascade2_[ch]));
    const double c2=cascade2_[ch]+g_*c1;
    cascade1_[ch]=2*c1-cascade1_[ch];cascade2_[ch]=2*c2-cascade2_[ch];
    if(std::abs(cascade1_[ch])<1e-24)cascade1_[ch]=0;
    if(std::abs(cascade2_[ch])<1e-24)cascade2_[ch]=0;
    const double high=driven-k_*v1-v2;
    // k*v1 gives unity at center frequency for the band-pass across the Q range.
    const double wet=weights_[0]*v2+weights_[1]*c2+weights_[2]*high+weights_[3]*k_*v1;
    samples[ch]=static_cast<float>(dry+mix_*(wet-dry));
  }
  return {samples[0],samples[1]};
}
}
