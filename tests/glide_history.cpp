// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<"\n";std::exit(1);}}
int main(){for(float sr:{44100.f,48000.f,96000.f}){
 {
  auto chordOwner=std::make_unique<sawstar::Synth>();
  auto referenceOwner=std::make_unique<sawstar::Synth>();
  auto& chord=*chordOwner;auto& reference=*referenceOwner;
  for(int mode:{1,2})for(float glide:{0.f,3.f,15.f,120.f})for(int transpose:{0,12}){
   for(auto s:{&chord,&reference}){
    s->Reset(sr);s->SetVoiceMode(mode,glide,true);
    s->SetParameters(-6,1,1,1,100);s->SetWaveforms(3,3);
    s->SetMixer(100,0,0,0,0,-1,0,0);s->SetFilter(20000,0,0);
    s->Midi(0x90,68+transpose,100);s->Midi(0x90,80+transpose,100);
    for(int i=0;i<int(sr*.25f);++i)s->Process();
    s->Midi(0x80,80+transpose,0);
    for(int i=0;i<int(sr*.025276f);++i)s->Process();
    s->Midi(0x80,68+transpose,0);
   }
   // No old key remains. Intermediate same-sample notes in this NEW phrase
   // must not inherit the previous phrase's rendered-pitch status.
   for(int note:{56,63,68,72,75,80})chord.Midi(0x90,note+transpose,100);
   reference.Midi(0x90,80+transpose,100);
   for(int i=0;i<int(sr*.15f);++i){chord.Process();reference.Process();
    check(std::abs(chord.PreFX().left-reference.PreFX().left)<1e-5f&&
          std::abs(chord.PreFX().right-reference.PreFX().right)<1e-5f,
          "later same-sample chord must not glide from an unrendered intermediate note");}
   for(int note:{56,63,68,72,75,80})check(chord.Held(note+transpose),"later chord keeps every held key");
  }
 }
}}
