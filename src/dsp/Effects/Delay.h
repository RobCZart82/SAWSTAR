// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/SevenSaw.h"
#include <algorithm>
#include <cmath>
#include <vector>
namespace sawstar {
// Original SAWSTAR stereo delay. Storage is prepared on Reset, never in Process/Set.
class Delay {
public:
 void Init(float sr){
  sr_=std::isfinite(sr)?std::clamp(sr,8000.f,384000.f):44100.f;
  for(auto& b:buffer_)b.assign(static_cast<size_t>(sr_*2)+2,0);
  write_=valid_=0;low_[0]=low_[1]=0;enabled_=false;
  mix_=targetMix_=0;feedback_=targetFeedback_=.3f;mode_=targetMode_=0;
  time_=targetTime_=.35f*sr_;tone_=targetTone_=1-std::exp(-6.28318530718f*6000/sr_);
  smooth_=1-std::exp(-1/(.02f*sr_));
 }
 void Set(bool on,float mix,float ms,float feedback,float tone,bool pingPong,bool sync,int division,double bpm){
  enabled_=on;targetMix_=on?Clean(mix,0,100,20)*.01f:0;
  targetFeedback_=Clean(feedback,0,85,30)*.01f;targetMode_=pingPong?1.f:0.f;
  float seconds=Clean(ms,1,2000,350)*.001f;
  if(sync){const double beats[]={.25,.5,.75,1,1.5,2,4};
   if(!std::isfinite(bpm)||bpm<=0)bpm=120;
   seconds=static_cast<float>(60/std::clamp(bpm,1.,1000.)*beats[std::clamp(division,0,6)]);
  }
  targetTime_=std::clamp(seconds,.001f,2.f)*sr_;
  targetTone_=1-std::exp(-6.28318530718f*std::min(Clean(tone,200,16000,6000),sr_*.45f)/sr_);
 }
 bool IsDry()const{return mix_==0&&targetMix_==0;}
 float TimeSeconds()const{return static_cast<float>(targetTime_/sr_);}
 StereoSample Process(StereoSample in){
  if(buffer_[0].empty())return in;
  mix_+=smooth_*(targetMix_-mix_);if(targetMix_==0&&mix_<1.e-7f)mix_=0;
  feedback_+=smooth_*(targetFeedback_-feedback_);mode_+=smooth_*(targetMode_-mode_);
  time_+=smooth_*(targetTime_-time_);tone_+=smooth_*(targetTone_-tone_);
  if(!enabled_&&mix_==0){valid_=0;low_[0]=low_[1]=0;return in;}
  const float read[]={Read(0),Read(1)};
  for(int c=0;c<2;++c){low_[c]+=tone_*(read[c]-low_[c]);if(std::abs(low_[c])<1.e-20f)low_[c]=0;}
  const float mono=(in.left+in.right)*.5f;
  buffer_[0][write_]=in.left*(1-mode_)+mono*mode_+feedback_*(low_[0]*(1-mode_)+low_[1]*mode_);
  buffer_[1][write_]=in.right*(1-mode_)+feedback_*(low_[1]*(1-mode_)+low_[0]*mode_);
  write_=(write_+1)%buffer_[0].size();valid_=std::min(valid_+1,buffer_[0].size());
  if(mix_==0)return in;
  return {in.left*(1-mix_)+low_[0]*mix_,in.right*(1-mix_)+low_[1]*mix_};
 }
private:
 static float Clean(float v,float lo,float hi,float fallback){return std::isfinite(v)?std::clamp(v,lo,hi):fallback;}
 float Read(int channel)const{
  const size_t whole=static_cast<size_t>(time_);const float fraction=static_cast<float>(time_-whole);
  const auto n=buffer_[channel].size();
  const float a=whole<=valid_?buffer_[channel][(write_+n-whole)%n]:0;
  const float b=whole+1<=valid_?buffer_[channel][(write_+n-whole-1)%n]:0;
  return a*(1-fraction)+b*fraction;
 }
 std::vector<float> buffer_[2];size_t write_=0,valid_=0;
 bool enabled_=false;float sr_=44100,smooth_=.001f,low_[2]{};
 float mix_=0,targetMix_=0,feedback_=.3f,targetFeedback_=.3f,mode_=0,targetMode_=0;
 double time_=15435,targetTime_=15435;
 float tone_=.5f,targetTone_=.5f;
};
}
