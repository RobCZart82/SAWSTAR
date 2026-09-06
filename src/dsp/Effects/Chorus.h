// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/SevenSaw.h"
#include <array>
#include <algorithm>
#include <cmath>
namespace sawstar {
// Original SAWSTAR stereo chorus. No feedback, allocation or shared state.
class Chorus {
public:
 void Init(float sampleRate) {
  rate_=std::isfinite(sampleRate)?std::clamp(sampleRate,8000.f,384000.f):44100.f;
  for(auto& b:buffer_)b.fill(0);
  write_=0;phase_=0;mix_=targetMix_=0;hz_=targetHz_=.3f;depth_=targetDepth_=.35f;
  smooth_=1.f-std::exp(-1.f/(.02f*rate_));
 }
 void Set(bool enabled,float mix,float hz,float depth) {
  targetMix_=enabled?Clean(mix,0,100,25)*.01f:0;
  targetHz_=Clean(hz,.05f,3.f,.3f);targetDepth_=Clean(depth,0,100,35)*.01f;
 }
 bool IsDry()const{return mix_==0 && targetMix_==0;}
 StereoSample Process(StereoSample in) {
  mix_+=smooth_*(targetMix_-mix_);if(targetMix_==0&&mix_<1.e-7f)mix_=0;
  hz_+=smooth_*(targetHz_-hz_);depth_+=smooth_*(targetDepth_-depth_);
  buffer_[0][write_]=in.left;buffer_[1][write_]=in.right;
  StereoSample out=in;
  if(mix_!=0){
   const float a=static_cast<float>(phase_)*6.28318530718f;
   const float excursion=4.f*depth_;
   const float l=.5f*(Read(0,12.f+excursion*std::sin(a))+Read(0,17.f+excursion*std::sin(a+2.1f)));
   const float r=.5f*(Read(1,12.f+excursion*std::sin(a+1.57079632679f))+Read(1,17.f+excursion*std::sin(a+3.67f)));
   out={in.left*(1-mix_)+l*mix_,in.right*(1-mix_)+r*mix_};
  }
  write_=(write_+1)%kSize;phase_+=hz_/rate_;phase_-=std::floor(phase_);
  return out;
 }
private:
 static float Clean(float v,float lo,float hi,float fallback){return std::isfinite(v)?std::clamp(v,lo,hi):fallback;}
 float Read(size_t channel,float milliseconds)const {
  const float delay=std::min(milliseconds*.001f*rate_,static_cast<float>(kSize-2));
  float pos=static_cast<float>(write_)-delay;if(pos<0)pos+=kSize;
  const auto i=static_cast<size_t>(pos);const float f=pos-static_cast<float>(i);
  return buffer_[channel][i]*(1-f)+buffer_[channel][(i+1)%kSize]*f;
 }
 static constexpr size_t kSize=8192;
 std::array<std::array<float,kSize>,2> buffer_{};
 size_t write_=0;double phase_=0;
 float rate_=44100,smooth_=.001f,mix_=0,targetMix_=0,hz_=.3f,targetHz_=.3f,depth_=.35f,targetDepth_=.35f;
};
}
