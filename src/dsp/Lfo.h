// SPDX-License-Identifier: MIT
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
namespace sawstar {
struct LfoOutput { float cutoff=0,pitch=0,amp=1,pan=0; };
// One global LFO per synth. Tempo sync follows BPM, not host timeline phase.
class Lfo {
public:
 void Init(float rate){rate_=rate;phase_=0;value_=0;depths_.fill(0);slew_=1-std::exp(-1.f/(.01f*rate));edge_=1-std::exp(-1.f/(.002f*rate));hz_=targetHz_=1;}
 void Set(float hz,float depth,int shape,int target,bool sync,int division,double bpm,bool retrigger){
  static constexpr float beats[]={4,2,1,.5f,.25f,.125f};
  bpm=std::isfinite(bpm)&&bpm>0?std::clamp(bpm,1.,1000.):120.;
  targetHz_=sync?static_cast<float>(bpm/60.)/beats[std::clamp(division,0,5)]:Safe(hz,.05f,20);
  depth_=Safe(depth,0,100)*.01f;shape_=std::clamp(shape,0,3);target_=std::clamp(target,0,3);retrigger_=retrigger;
 }
 void Trigger(){if(retrigger_)phase_=0;}
 LfoOutput Process(){
  hz_+=slew_*(targetHz_-hz_);
  float raw=0;
  switch(shape_){case 0:raw=std::sin(6.283185307179586*phase_);break;case 1:raw=1-4*std::abs(phase_-.5);break;case 2:raw=2*phase_-1;break;default:raw=phase_<.5?1:-1;}
  value_+=edge_*(raw-value_);
  phase_+=hz_/rate_;phase_-=std::floor(phase_);
  for(int i=0;i<4;++i)depths_[i]+=slew_*((i==target_?depth_:0)-depths_[i]);
  return {value_*depths_[0]*24, value_*depths_[1],1-depths_[2]*(value_+1)*.5f,value_*depths_[3]};
 }
 float Value()const{return value_;}
 double Phase()const{return phase_;}
private:
 static float Safe(float x,float lo,float hi){return std::isfinite(x)?std::clamp(x,lo,hi):lo;}
 float rate_=44100,slew_=0,edge_=0,hz_=1,targetHz_=1,value_=0,depth_=0;
 double phase_=0;int shape_=0,target_=0;bool retrigger_=false;
 std::array<float,4> depths_{};
};
}
