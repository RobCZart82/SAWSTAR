// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
using sawstar::Synth;
void check(bool ok,const char* message){if(!ok){std::cerr<<message<<'\n';std::exit(1);}}
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
   daisysp::Adsr expected;expected.Init(sr);expected.SetAttackTime(attack*.001f);
   // Pre-FX difference removes the other fading voices. The new source is
   // one unit-amplitude saw/sine, so it cannot exceed its fresh ADSR value.
   for(int i=0;i<64;++i){s->ProcessStereo();reference->ProcessStereo();
    const auto x=s->PreFX(),y=reference->PreFX();const float bound=expected.Process(true)+2e-5f;
    check(std::abs(x.left-y.left)<=bound&&std::abs(x.right-y.right)<=bound,
          "idle slot must start from zero envelope after a mode round-trip");}
  }
  for(int mode:{1,2})for(int wave:{0,1,3})for(int phase:{3,17,61}){
   auto s=std::make_unique<Synth>();setup(*s,sr,wave);s->SetParameters(-6,1,1,1,500);
   for(int n:{48,55,60,64,67,72})s->Midi(0x90,n,127);
   run(*s,int(sr*.15f)+phase);s->SetVoiceMode(mode,0,true);run(*s,int(sr*.001f));
   auto reference=std::make_unique<Synth>(*s);
   s->Midi(0x90,81,127);s->ProcessStereo();reference->ProcessStereo();
   const auto x=s->PreFX(),y=reference->PreFX();
   check(std::abs(x.left-y.left)<2e-5f&&std::abs(x.right-y.right)<2e-5f,
         "new mono note must preserve the first sample of all outgoing mode voices");
   run(*s,int(sr*.012f));
   check(s->ActiveVoices()==1&&s->Held(81)&&!s->Held(60),"retired poly voices finish while new mono note survives");
   s->Midi(0x80,81,0);
   // One long-release check per mode/rate; the phase/wave grid above tests
   // the transition itself, without repeating seconds of identical decay.
   if(wave==0&&phase==3){run(*s,int(sr*3));check(s->ActiveVoices()==0,"mode transition release completes");}
   else{s->Midi(0xb0,120,0);check(s->ActiveVoices()==0,"panic clears post-transition release");}
  }
  // A mode change without a new note must have a finite retirement deadline.
  for(int from:{0,1,2})for(int to:{0,1,2})if(from!=to){
   auto s=std::make_unique<Synth>();setup(*s,sr);s->SetVoiceMode(from,0,true);
   s->Midi(0x90,60,127);s->Midi(0x90,64,127);run(*s,int(sr*.15f));
   s->SetVoiceMode(to,0,true);run(*s,int(sr*.01f));
   check(s->ActiveVoices()==0&&s->Process()==0,"mode change must retire old audio within ten milliseconds");
  }
  // All slots occupied, mode changes faster than retirement, same-sample
  // changes and repeated notes: fixed storage, no stuck keys or tails.
  auto s=std::make_unique<Synth>();setup(*s,sr);
  for(int n=48;n<64;++n)s->Midi(0x90,n,100);run(*s,1000);
  for(int i=0;i<100;++i){s->SetVoiceMode(i%3,0,true);s->Midi(0x90,36+i%60,100);
   run(*s,i%5);check(s->ActiveVoices()<=16,"mode automation keeps the musical voice limit");}
  for(int ch=0;ch<16;++ch)s->Midi(0xb0|ch,120,0);
  check(s->ActiveVoices()==0&&s->Process()==0,"panic clears mode automation tails");
  s->Midi(0x90,60,100);run(*s,1000);s->SetVoiceMode(2,0,true);s->Reset(sr);
  check(s->ActiveVoices()==0&&s->Process()==0,"reset clears mode retirement");
  setup(*s,sr);s->Midi(0x90,60,127);s->Midi(0x91,67,127);run(*s,int(sr*.1f)+13);
  s->SetVoiceMode(1,0,true);s->Midi(0xb0,120,0);double other=0;
  for(int i=0;i<20;++i){s->Process();other+=std::abs(s->PreFX().left);}
  check(other>1e-4,"channel-local panic preserves the other retiring channel");
  s->Midi(0xb1,120,0);check(s->Process()==0,"panic clears the final retiring channel");
 }
}
