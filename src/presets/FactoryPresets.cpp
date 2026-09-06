// SPDX-License-Identifier: MIT
#include "presets/FactoryPresets.h"
#include <cmath>
#include <initializer_list>
#include <utility>
namespace sawstar {
namespace {
Snapshot Sound(std::initializer_list<std::pair<ParameterId,double>> edits) {
  auto s=DefaultSnapshot();for(auto p:edits)s[static_cast<size_t>(p.first)]=p.second;return s;
}
}
const std::array<FactoryPreset,6>& FactoryPresets() {
 using P=ParameterId;
 static const std::array<FactoryPreset,6> presets{{
  {"init","Init","Basic","One saw, open signal path. Start shaping your own sound.",DefaultSnapshot()},
  {"wide_lead","Wide Saw Lead","Lead","Seven detuned saws add width. MOD opens the low-pass filter.",Sound({{P::SawMix,70},{P::SawDetune,23},{P::SawWidth,90},{P::FilterCutoff,2600},{P::FilterMix,100},{P::FilterResonance,15},{P::AmpRelease,180}})},
  {"soft_pad","Soft Pad","Pad","Slow amp attack and release soften the wide saw ensemble.",Sound({{P::SawMix,85},{P::SawDetune,16},{P::SawWidth,100},{P::AmpAttack,850},{P::AmpDecay,1500},{P::AmpSustain,.8},{P::AmpRelease,1800},{P::FilterCutoff,1100},{P::FilterMix,100},{P::FilterEnvAmount,12},{P::FilterAttack,1400},{P::FilterDecay,1800},{P::FilterSustain,.5},{P::FilterRelease,1800}})},
  {"saw_pluck","Saw Pluck","Pluck","A short positive filter envelope makes the attack bright.",Sound({{P::SawMix,45},{P::SawDetune,12},{P::AmpAttack,2},{P::AmpDecay,180},{P::AmpSustain,.12},{P::AmpRelease,170},{P::FilterCutoff,260},{P::FilterMix,100},{P::FilterResonance,25},{P::FilterEnvAmount,48},{P::FilterAttack,2},{P::FilterDecay,140},{P::FilterSustain,0},{P::FilterRelease,140},{P::FilterKeyTrack,70}})},
  {"deep_bass","Deep Bass","Bass","A centered saw and low cutoff keep the low notes focused.",Sound({{P::SawWidth,0},{P::AmpAttack,3},{P::AmpDecay,180},{P::AmpSustain,.6},{P::AmpRelease,110},{P::FilterCutoff,220},{P::FilterMix,100},{P::FilterResonance,12},{P::FilterEnvAmount,20},{P::FilterAttack,3},{P::FilterDecay,180},{P::FilterSustain,.15},{P::FilterRelease,110},{P::FilterKeyTrack,50}})},
  {"bright_keys","Bright Keys","Keys","Moderate unison and a falling filter envelope give a key-like shape.",Sound({{P::SawMix,35},{P::SawDetune,8},{P::SawWidth,65},{P::AmpAttack,4},{P::AmpDecay,500},{P::AmpSustain,.25},{P::AmpRelease,450},{P::FilterCutoff,1500},{P::FilterMix,100},{P::FilterEnvAmount,18},{P::FilterAttack,4},{P::FilterDecay,350},{P::FilterSustain,.2},{P::FilterRelease,350},{P::FilterKeyTrack,100}})}
 }};
 return presets;
}
int MatchFactoryPreset(const Snapshot& s) {
 const auto& presets=FactoryPresets();
 for(size_t p=0;p<presets.size();++p){bool match=true;
  for(size_t i=0;i<s.size();++i)if(!std::isfinite(s[i])||std::abs(s[i]-presets[p].values[i])>1e-7*(1+std::abs(presets[p].values[i]))){match=false;break;}
  if(match)return static_cast<int>(p);
 }return -1;
}
int StepFactoryPreset(int current,int direction){
 const int n=static_cast<int>(FactoryPresets().size());
 if(current<0||current>=n)return direction<0?n-1:0;
 return (current+(direction<0?n-1:1))%n;
}
}
