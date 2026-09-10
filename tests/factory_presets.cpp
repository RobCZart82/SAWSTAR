// SPDX-License-Identifier: MIT
#include "presets/FactoryPresets.h"
#include "SynthTestRig.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
int main(){
 using namespace sawstar;const auto& presets=FactoryPresets();std::set<std::string> keys,names;
 check(presets[0].values==DefaultSnapshot(),"Init must use parameter defaults");
 for(size_t p=0;p<presets.size();++p){const auto& v=presets[p].values;
  check(keys.insert(presets[p].key).second&&names.insert(presets[p].name).second,"unique preset identities");
  for(size_t i=0;i<v.size();++i)check(std::isfinite(v[i])&&v[i]>=kParameters[i].minimum&&v[i]<=kParameters[i].maximum,"valid factory value");
  check(MatchFactoryPreset(v)==static_cast<int>(p),"factory recognition");
  if(std::string(presets[p].category)=="Templates"){for(auto id:{ParameterId::ChorusEnabled,ParameterId::DelayEnabled,ParameterId::ReverbEnabled,ParameterId::ArpEnabled,ParameterId::LfoDepth,ParameterId::Lfo2Depth})check(v[size_t(id)]==0,"templates stay dry and unmodulated");}
  auto bytes=EncodeState(v);Snapshot restored{};check(DecodeState(bytes.data(),bytes.size(),restored)==bytes.size()&&restored==v,"factory save/recall");
  auto edited=v;edited[0]+=.01;check(MatchFactoryPreset(edited)==-1,"edited sound is Custom");
  for(float sr:{44100.f,48000.f,96000.f}){
   auto configure=[&](Rig& s,const Snapshot& values){s.init(sr);s.apply(values);};
   Rig s;configure(s,v);
   s.midi(0x90,p==4?36:60,100);double energy=0;
   for(int i=0;i<sr*2;++i){auto x=s.process();check(std::isfinite(x.left)&&std::isfinite(x.right)&&std::abs(x.left)<=1&&std::abs(x.right)<=1,"factory audio bounds");energy+=x.left*x.left+x.right*x.right;}
   check(energy>.001,"factory preset produces audio");
   if(p==6||p==7){
    Rig chord,withoutSecond;configure(chord,v);auto reduced=v;reduced[21]=0;configure(withoutSecond,reduced);
    for(int note:{48,55,60,64,67}){chord.midi(0x90,note,110);withoutSecond.midi(0x90,note,110);}
    double peak=0,difference=0,stereo=0;
    for(int i=0;i<sr*5;++i){auto x=chord.process(),y=withoutSecond.process();
     check(std::isfinite(x.left)&&std::isfinite(x.right),"dual oscillator chord finite");
     peak=std::max(peak,static_cast<double>(std::max(std::abs(x.left),std::abs(x.right))));
     difference+=std::abs(x.left-y.left)+std::abs(x.right-y.right);stereo+=std::abs(x.left-x.right);
    }
    check(peak>.01&&peak<.98,"dual oscillator chord has output headroom");
    check(difference/sr>.01,"OSC2 contributes to new sound");check(stereo/sr>.01,"new sound has stereo width");
    std::cout<<presets[p].name<<" @ "<<sr<<" Hz chord peak "<<20*std::log10(peak)<<" dBFS\n";
   }
   s.midi(0x80,p==4?36:60,0);for(int i=0;i<sr*std::max(12.,1.+5.*v[4]/1000.);++i)s.process();check(s.synth.ActiveVoices()==0,"factory release finishes");
  }
 }
 check(StepFactoryPreset(0,-1)==int(presets.size())-1&&StepFactoryPreset(int(presets.size())-1,1)==0,"selector wraps");
 check(StepFactoryPreset(-1,1)==0&&StepFactoryPreset(-1,-1)==int(presets.size())-1,"Custom navigation");
}
