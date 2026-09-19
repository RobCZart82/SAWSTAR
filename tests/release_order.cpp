// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<"\n";std::exit(1);}}
int main(){for(float sr:{44100.f,48000.f,96000.f}){

 for(int mode:{1,2})for(bool pedal:{false,true}){
  auto aa=std::make_unique<sawstar::Synth>(),bb=std::make_unique<sawstar::Synth>();
  auto& a=*aa;auto& b=*bb;
  for(auto s:{&a,&b}){s->Reset(sr);s->SetParameters(-6,3,100,.2,200);
   s->SetVoiceMode(mode,0,true);s->SetFilter(1200,60,100);
   for(int note:{48,60,80})s->Midi(0x90,note,127);
   for(int i=0;i<int(sr*.4);++i)s->Process();
   if(pedal)s->Midi(0xb0,64,127);
  }
  for(int note:{80,60,48})a.Midi(0x80,note,0);
  for(int note:{48,60,80})b.Midi(0x80,note,0);
  if(pedal){a.Midi(0xb0,64,0);b.Midi(0xb0,64,0);}
  for(int i=0;i<int(sr);++i){auto x=a.ProcessStereo(),y=b.ProcessStereo();
   check(x.left==y.left&&x.right==y.right,"same-sample release order changes audio");}
  check(a.ActiveVoices()==0&&b.ActiveVoices()==0,"grouped releases finish");
 }
}}
