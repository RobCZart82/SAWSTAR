// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
void check(bool value,const char* why){if(!value){std::cerr<<why<<'\n';std::exit(1);}}
int main(){for(float sr:{44100.f,48000.f,96000.f}){
 for(int mode:{1,2})for(bool pedal:{false,true}){
  sawstar::Synth a,b;
  for(auto s:{&a,&b}){s->Reset(sr);s->SetParameters(-6,3,100,.2,200);
   s->SetVoiceMode(mode,0,true);s->SetFilter(1200,60,100);
   s->Midi(0x90,48,127);s->Midi(0x90,60,127);s->Midi(0x90,80,127);
   for(int i=0;i<int(sr*.4);++i)s->Process();
   if(pedal)s->Midi(0xb0,64,127);
  }
  // Identical timestamp and final key state; only note-off ordering differs.
  for(int note:{80,60,48})a.Midi(0x80,note,0);
  for(int note:{48,60,80})b.Midi(0x80,note,0);
  if(pedal){a.Midi(0xb0,64,0);b.Midi(0xb0,64,0);}
  for(int i=0;i<int(sr);++i){auto x=a.ProcessStereo(),y=b.ProcessStereo();
   check(x.left==y.left&&x.right==y.right,"same-sample release ordering must not change the audio tail");}
  check(a.ActiveVoices()==0&&b.ActiveVoices()==0,"grouped releases must finish");
 }
 for(int mode:{1,2}){
  sawstar::Synth a,b;
  for(auto s:{&a,&b}){s->Reset(sr);s->SetParameters(-6,1,1,1,10);
   s->SetVoiceMode(mode,0,true);s->SetFilter(20000,0,0);
   s->Midi(0x90,48,127);s->Midi(0x90,60,127);
   for(int i=0;i<1003;++i)s->Process();
   s->Midi(0x90,84,127);s->Process();
  }
  a.Midi(0x80,48,0); // Neither sounding note nor next fallback changes.
  for(int i=0;i<int(sr*.01);++i){auto x=a.ProcessStereo(),y=b.ProcessStereo();
   check(x.left==y.left&&x.right==y.right,"unrelated release must not cancel active continuity correction");}
 }
 for(int mode:{1,2})for(int wave:{0,1,2,3}){
  sawstar::Synth s;s.Reset(sr);s.SetParameters(-6,1,1,1,10);
  s.SetVoiceMode(mode,0,true);s.SetWaveforms(wave,wave);s.SetFilter(20000,0,0);
  s.Midi(0x90,48,127);
  for(int n=0;n<12;++n){
   for(int i=0;i<static_cast<int>(sr*.013)+n*7;++i)s.Process();
   auto previous=s.PreFX();s.Midi(0x90,84,80);s.Process();auto next=s.PreFX();
   check(std::abs(next.left-previous.left)<1e-5f&&std::abs(next.right-previous.right)<1e-5f,
         "mono/legato new note must preserve first-sample continuity");
   for(int i=0;i<static_cast<int>(sr*.011)+n*3;++i)s.Process();
   previous=s.PreFX();s.Midi(0x80,84,0);s.Process();next=s.PreFX();
   check(std::abs(next.left-previous.left)<1e-5f&&std::abs(next.right-previous.right)<1e-5f,
         "mono/legato held-note fallback must preserve first-sample continuity");
  }
  s.Midi(0x80,48,0);for(int i=0;i<int(sr);++i)s.Process();
  check(s.ActiveVoices()==0&&s.Process()==0,"mono correction must not leave a hanging voice");
  s.Midi(0x90,48,127);for(int i=0;i<1000;++i)s.Process();
  s.Midi(0x90,84,127);s.Process();s.Midi(0xb0,120,0);
  check(s.Process()==0,"mono panic must silence an active correction");
 }
 for(bool full:{false,true}){
  sawstar::Synth s;s.Reset(sr);s.SetParameters(0,1,1,1,10);s.SetOutputBoost(18);
  s.Midi(0x90,60,127);
  if(full)for(int ch=1;ch<16;++ch)s.Midi(0x90|ch,60,1);
  float previous=0;for(int i=0;i<int(sr/2)+17;++i)previous=s.Process();
  // Same-note velocity retrigger or oldest-voice replacement at full capacity.
  s.Midi(0x90,full?91:60,1);float next=s.Process();
  check(std::abs(next-previous)<.004,"voice reuse must not jump in level");
  check(s.ActiveVoices()==(full?16:1),"voice count remains bounded");
  for(int n=0;n<100;++n){s.Midi(0x90,36+n%60,n%2?1:127);
   for(int i=0;i<7;++i){auto x=s.ProcessStereo();check(std::isfinite(x.left)&&std::abs(x.left)<=.98001f,"dense reuse remains bounded");}}
  for(int ch=0;ch<16;++ch)s.Midi(0xb0|ch,123,0);
  for(int i=0;i<int(sr);++i)s.Process();
  check(s.ActiveVoices()==0&&s.Process()==0,"splice tails release fully");
  s.Midi(0x90,60,127);s.Process();s.Midi(0x90,60,1);s.Process();s.Midi(0xb0,120,0);
  check(s.Process()==0,"panic clears splice immediately");
 }
}}
