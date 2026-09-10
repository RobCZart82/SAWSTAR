// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/Safety.h"
#include <array>
#include <algorithm>
#include <cmath>
namespace sawstar {
// Four routes. Fade each source/destination weight independently when rerouting.
class Modulation {
public:
 using Values=std::array<float,5>; // cutoff semitones, pitch semitones, amp, pan, color
 void Init(float sr){sr=SafeSampleRate(sr);active_=dirty_=false;weights_={};combined_={};sources_={};targets_={};amounts_={};slew_=1-std::exp(-1.f/(.01f*sr));}
 void Set(int row,int source,int target,float amount){if(row<0||row>=4)return;
  source=std::clamp(source,0,5);target=std::clamp(target,0,4);
  amount=std::isfinite(amount)?std::clamp(amount*.01f,-1.f,1.f):0;
  if(sources_[row]!=source||targets_[row]!=target||amounts_[row]!=amount)dirty_=true;
  sources_[row]=source;targets_[row]=target;amounts_[row]=amount;
 }
 void Process(){if(!dirty_)return;dirty_=false;active_=false;combined_={};for(int r=0;r<4;++r)for(int s=0;s<5;++s)for(int t=0;t<5;++t){
  auto& w=weights_[r][s][t];const float goal=sources_[r]==s+1&&targets_[r]==t?amounts_[r]:0;
  if(std::abs(goal-w)>1.e-4f){w+=slew_*(goal-w);dirty_=true;}else w=goal;
  active_|=w!=0;combined_[s][t]+=w;
 }}
 Values Evaluate(const Values& sources)const {
  Values out{};if(!active_)return out;for(int s=0;s<5;++s)for(int t=0;t<5;++t)out[t]+=combined_[s][t]*sources[s];
  out[0]=std::clamp(out[0]*48,-96.f,96.f);out[1]=std::clamp(out[1]*12,-48.f,48.f);
  for(int i=2;i<5;++i)out[i]=std::clamp(out[i],-1.f,1.f);return out;
 }
private:
 bool active_=false,dirty_=false;
 std::array<std::array<std::array<float,5>,5>,4> weights_{};
 std::array<std::array<float,5>,5> combined_{};
 std::array<int,4> sources_{},targets_{};std::array<float,4> amounts_{};float slew_=0;
};
}
