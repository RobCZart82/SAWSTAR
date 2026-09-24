// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<"\n";std::exit(1);}}
using sawstar::Synth;
void run(Synth& s,int frames){for(int i=0;i<frames;++i){auto x=s.ProcessStereo();
 check(std::isfinite(x.left)&&std::isfinite(x.right)&&std::abs(x.left)<=.98001f&&std::abs(x.right)<=.98001f,"bounded mode transition output");}}
void setup(Synth& s,float sr,int wave=0){
 s.Reset(sr);s.SetParameters(-6,100,20,.8,500);s.SetWaveforms(wave,wave);
 s.SetMixer(100,0,0,0,0,-1,0,0);s.SetSaw(0,0,0);s.SetFilter(20000,0,0);
 s.SetFilterEnvelope(0,0,100,20,0,500);
}
int main(){
 for(float sr:{44100.f,48000.f,96000.f}){
  // Reusing a truly idle slot must obey Attack, even if that slot was
  // previously stopped while its envelope was at sustain.
  for(int mode:{1,2})for(int wave:{0,3})for(float attack:{100.f,1000.f}){
   auto s=std::make_unique<Synth>();setup(*s,sr,wave);
   s->Midi(0x90,60,127);s->Midi(0x90,64,127);run(*s,int(sr*.75037f));
   s->SetVoiceMode(mode,0,true);s->Midi(0x90,67,127);run(*s,int(sr*.02f));
   s->SetVoiceMode(0,0,true);s->SetParameters(-6,attack,20,.8,500);
   auto reference=std::make_unique<Synth>(*s);
   s->Midi(0x90,69,127);
   sawstar::Adsr expected;expected.Init(sr);expected.SetAttackTime(attack*.001f);
   // Pre-FX difference removes the other fading voices. The new source is
   // one unit-amplitude saw/sine, so it cannot exceed its fresh ADSR value.
   for(int i=0;i<64;++i){s->ProcessStereo();reference->ProcessStereo();
    const auto x=s->PreFX(),y=reference->PreFX();const float bound=expected.Process(true)+2e-5f;
    check(std::abs(x.left-y.left)<=bound&&std::abs(x.right-y.right)<=bound,
          "idle slot must start from zero envelope after a mode round-trip");}
  }

}}
