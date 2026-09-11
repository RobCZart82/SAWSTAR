// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/SevenSaw.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>
namespace sawstar {
// Original eight-line feedback delay network with an orthogonal mixing matrix.
// Buffers allocate in Init only; sample processing and parameter updates do not allocate.
class Reverb {
public:
 // Audio-thread panic: discard history without reallocating or changing settings.
 void Clear(){write_=valid_=0;low_.fill(0);}
 void Init(float sr){
  sr_=std::isfinite(sr)?std::clamp(sr,8000.f,384000.f):44100.f;
  for(auto& b:buffer_)b.assign(static_cast<size_t>(sr_*.13f)+2,0);
  write_=valid_=0;low_.fill(0);mix_=targetMix_=0;on_=false;
  smooth_=1-std::exp(-1/(.02f*sr_));
  Set(false,20,50,2.5f,6000);
  delays_=targetDelays_;gains_=targetGains_;tone_=targetTone_;
 }
 void Set(bool on,float mix,float size,float decay,float damping){
  on_=on;targetMix_=on?Clean(mix,0,100,20)*.01f:0;
  const double scale=.6+Clean(size,0,100,50)*.01;
  const double seconds=Clean(decay,.2f,10,2.5f);
  for(size_t i=0;i<8;++i){
   targetDelays_[i]=kTimes[i]*scale*sr_;
   targetGains_[i]=static_cast<float>(std::exp(std::log(.001)*kTimes[i]*scale/seconds));
  }
  targetTone_=1-std::exp(-6.28318530718f*std::min(Clean(damping,500,16000,6000),sr_*.45f)/sr_);
 }
 bool IsDry()const{return mix_==0&&targetMix_==0;}
 StereoSample Process(StereoSample in){
  if(buffer_[0].empty())return in;
  mix_+=smooth_*(targetMix_-mix_);if(targetMix_==0&&mix_<1.e-7f)mix_=0;
  tone_+=smooth_*(targetTone_-tone_);
  for(size_t i=0;i<8;++i){delays_[i]+=smooth_*(targetDelays_[i]-delays_[i]);gains_[i]+=smooth_*(targetGains_[i]-gains_[i]);}
  if(!on_&&mix_==0){valid_=0;low_.fill(0);return in;}
  std::array<float,8> taps{};float total=0,wetL=0,wetR=0;
  for(size_t i=0;i<8;++i){
   taps[i]=Read(i);low_[i]+=tone_*(taps[i]-low_[i]);if(std::abs(low_[i])<1.e-20f)low_[i]=0;
   taps[i]=low_[i]*gains_[i];total+=taps[i];
   wetL+=low_[i]*kLeft[i];wetR+=low_[i]*kRight[i];
  }
  // I - 2vv^T, v=(1,...,1)/sqrt(8), preserves feedback energy before losses.
  for(size_t i=0;i<8;++i)buffer_[i][write_]=taps[i]-.25f*total+.35355339059f*(in.left*kLeft[i]+in.right*kRight[i]);
  write_=(write_+1)%buffer_[0].size();valid_=std::min(valid_+1,buffer_[0].size());
  if(mix_==0)return in;
  return {in.left*(1-mix_)+wetL*.35355339059f*mix_,in.right*(1-mix_)+wetR*.35355339059f*mix_};
 }
private:
 static float Clean(float v,float lo,float hi,float fallback){return std::isfinite(v)?std::clamp(v,lo,hi):fallback;}
 float Read(size_t channel)const{
  const size_t whole=static_cast<size_t>(delays_[channel]),n=buffer_[channel].size();
  const float fraction=static_cast<float>(delays_[channel]-whole);
  const float a=whole<=valid_?buffer_[channel][(write_+n-whole)%n]:0;
  const float b=whole+1<=valid_?buffer_[channel][(write_+n-whole-1)%n]:0;
  return a*(1-fraction)+b*fraction;
 }
 inline static constexpr double kTimes[]={.0297,.0371,.0411,.0437,.0531,.0593,.0677,.0739};
 inline static constexpr float kLeft[]={1,1,-1,-1,1,1,-1,-1},kRight[]={1,-1,1,-1,1,-1,1,-1};
 std::array<std::vector<float>,8> buffer_;
 std::array<double,8> delays_{},targetDelays_{};
 std::array<float,8> low_{},gains_{},targetGains_{};
 size_t write_=0,valid_=0;bool on_=false;
 float sr_=44100,smooth_=.001f,mix_=0,targetMix_=0,tone_=.5f,targetTone_=.5f;
};
}
