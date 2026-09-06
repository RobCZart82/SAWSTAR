// SPDX-License-Identifier: MIT
#include "presets/FactoryPresets.h"
#include "engine/Synth.h"
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
  auto bytes=EncodeState(v);Snapshot restored{};check(DecodeState(bytes.data(),bytes.size(),restored)==bytes.size()&&restored==v,"factory save/recall");
  auto edited=v;edited[0]+=.01;check(MatchFactoryPreset(edited)==-1,"edited sound is Custom");
  for(float sr:{44100.f,48000.f,96000.f}){
   auto configure=[&](Synth& s,const Snapshot& v){s.Reset(sr);s.SetParameters(v[0],v[1],v[2],v[3],v[4]);s.SetSaw(v[5],v[6],v[7]);
   s.SetFilter(v[8],v[9],v[10]);s.SetFilterEnvelope(v[11],v[12],v[13],v[14],v[15],v[16]);s.SetPerformance(v[17],v[18]);s.SetOutputBoost(v[19]);s.SetMixer(v[20],v[21],v[22],v[23],v[24],v[25],v[26],v[30]);s.SetOsc2(v[27],v[28],v[29]);s.SetFilterCharacter(v[31],v[32]);s.SetWaveforms(v[33],v[34]);s.SetLfo(v[35],v[36],v[37],v[38],v[39]!=0,v[40],120,v[41]!=0);};
   Synth s;configure(s,v);
   s.Midi(0x90,p==4?36:60,100);double energy=0;
   for(int i=0;i<sr*2;++i){auto x=s.ProcessStereo();check(std::isfinite(x.left)&&std::isfinite(x.right)&&std::abs(x.left)<=1&&std::abs(x.right)<=1,"factory audio bounds");energy+=x.left*x.left+x.right*x.right;}
   check(energy>.001,"factory preset produces audio");
   if(p>=6){
    Synth chord,withoutSecond;configure(chord,v);auto reduced=v;reduced[21]=0;configure(withoutSecond,reduced);
    for(int note:{48,55,60,64,67}){chord.Midi(0x90,note,110);withoutSecond.Midi(0x90,note,110);}
    double peak=0,difference=0,stereo=0;
    for(int i=0;i<sr*5;++i){auto x=chord.ProcessStereo(),y=withoutSecond.ProcessStereo();
     check(std::isfinite(x.left)&&std::isfinite(x.right),"dual oscillator chord finite");
     peak=std::max(peak,static_cast<double>(std::max(std::abs(x.left),std::abs(x.right))));
     difference+=std::abs(x.left-y.left)+std::abs(x.right-y.right);stereo+=std::abs(x.left-x.right);
    }
    check(peak>.01&&peak<.98,"dual oscillator chord has output headroom");
    check(difference/sr>.01,"OSC2 contributes to new sound");check(stereo/sr>.01,"new sound has stereo width");
    std::cout<<presets[p].name<<" @ "<<sr<<" Hz chord peak "<<20*std::log10(peak)<<" dBFS\n";
   }
   s.Midi(0x80,p==4?36:60,0);for(int i=0;i<sr*std::max(12.,1.+5.*v[4]/1000.);++i)s.ProcessStereo();check(s.ActiveVoices()==0,"factory release finishes");
  }
 }
 check(StepFactoryPreset(0,-1)==7&&StepFactoryPreset(7,1)==0,"selector wraps");
 check(StepFactoryPreset(-1,1)==0&&StepFactoryPreset(-1,-1)==7,"Custom navigation");
}
