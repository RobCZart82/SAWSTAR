// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
// Keep large test fixtures off the Windows Debug main-thread stack.
// Allocation is test setup only, outside the audio processing loop.
void check(bool value,const char* why){if(!value){std::cerr<<why<<'\n';std::exit(1);}}
int main(){for(float sr:{44100.f,48000.f,96000.f}){
 {
  auto owner=std::make_unique<sawstar::Synth>();auto& s=*owner;
  s.Reset(sr);s.SetVoiceMode(1,0,true);s.SetParameters(-6,1,1,1,100);
  s.SetWaveforms(3,3);s.SetFilter(20000,0,0);
  s.Midi(0x90,48,127);for(int i=0;i<int(sr*.2f)+17;++i)s.Process();
  s.Midi(0x80,48,0);s.Midi(0x91,72,127);s.Process();
  s.Midi(0x81,72,0);s.Midi(0x92,84,127);s.Process();
  s.Midi(0xb2,120,0);s.Process();
  check(s.ActiveVoices()==0&&std::abs(s.PreFX().left)>1e-4f,
        "channel-local panic must preserve other-channel transition tails without a main voice");
  s.Midi(0xb0,120,0);s.Midi(0xb1,120,0);
  check(s.Process()==0,"panic on every contributing channel must clear all transition branches");
  s.Reset(sr);check(s.Process()==0,"reset must clear all overlapping transition branches");
 }
 {
  auto changingOwner=std::make_unique<sawstar::Synth>();
  auto continuingOwner=std::make_unique<sawstar::Synth>();
  auto& changing=*changingOwner;auto& continuing=*continuingOwner;
  for(int mode:{1,2})for(int note:{36,48,60})for(int interval:{12,36})
  for(int wave:{0,1,2,3})for(int phase:{0,7,23,71}){
   for(auto s:{&changing,&continuing}){s->Reset(sr);s->SetVoiceMode(mode,0,true);
    s->SetParameters(-6,1,1,1,100);s->SetWaveforms(wave,wave);s->SetFilter(20000,0,0);
    s->Midi(0x90,note,127);for(int i=0;i<int(sr*.05f)+phase;++i)s->Process();}
   changing.Midi(0x90,note+interval,127);
   changing.Process();continuing.Process();
   changing.Midi(0x80,note+interval,0);
   // A one-sample excursion barely enters the mix. The return should follow
   // the continued low tone, including natural saw/square edges: flattening
   // an ordinary edge to the previous sample must not add a lasting offset.
   for(int i=0;i<int(sr*.0005f);++i){changing.Process();continuing.Process();
    check(std::abs(changing.PreFX().left-continuing.PreFX().left)<.01f,
          "overlapping retarget must preserve the outgoing waveform and its natural edges");}
  }
 }
 {
  auto chordOwner=std::make_unique<sawstar::Synth>();
  auto referenceOwner=std::make_unique<sawstar::Synth>();
  auto& chord=*chordOwner;auto& reference=*referenceOwner;
  for(int mode:{1,2})for(bool overlap:{false,true})for(float glide:{3.f,15.f,120.f}){
   for(auto s:{&chord,&reference}){
    // Reuse the fixture across resets: a previous phrase must not count as
    // rendered pitch history for the new processing lifecycle.
    s->Reset(sr);s->SetVoiceMode(mode,0,overlap);s->Midi(0x90,69,100);
    for(int i=0;i<64;++i)s->Process();
    s->Reset(sr);s->SetVoiceMode(mode,s==&chord?glide:0.f,overlap);
    s->SetParameters(-6,3,100,.8,100);s->SetWaveforms(0,1);
    s->SetMixer(90,25,10,0,0,-1,0,0);s->SetFilter(233,60,100);s->SetFilterCharacter(3,1);
    for(int i=0;i<int(sr*.02f);++i)s->Process();
    // All note-ons share one sample. None of the intermediate priorities
    // has generated sound, so the first chord has no pitch to glide FROM.
    for(int note:{58,65,70,73,77,82})s->Midi(0x90,note,100);
   }
   for(int i=0;i<int(sr*.15f);++i){auto x=chord.ProcessStereo(),y=reference.ProcessStereo();
    check(x.left==y.left&&x.right==y.right,
          "first same-sample mono chord must not glide from an unrendered intermediate note");}
   for(int note:{58,65,70,73,77,82})check(chord.Held(note),"first chord keeps every held key");
  }
 }
 {
  auto aOwner=std::make_unique<sawstar::Synth>();auto bOwner=std::make_unique<sawstar::Synth>();
  auto& a=*aOwner;auto& b=*bOwner;
  for(int mode:{0,1,2}){
   for(auto v:{&a,&b}){v->Reset(sr);v->SetVoiceMode(mode,0,true);
    v->SetParameters(-6,4,20,.1f,100);v->SetWaveforms(3,3);v->SetFilter(20000,0,0);
    v->Midi(0x90,60,100);for(int i=0;i<int(sr*.2);++i)v->Process();}
   a.Midi(0x80,60,0);a.Midi(0x90,60,100);
   b.Midi(0x90,60,100);b.Midi(0x80,60,0);
   for(int i=0;i<int(sr*.05);++i){auto x=a.ProcessStereo(),y=b.ProcessStereo();
    check(x.left==y.left&&x.right==y.right,"adjacent repeated note must not depend on on/off ordering");}
   a.Midi(0x80,60,0);b.Midi(0x80,60,0);
   for(int i=0;i<int(sr);++i){a.Process();b.Process();}
   check(a.ActiveVoices()==0&&b.ActiveVoices()==0,"repeated note counters must release fully");
  }
  // Two keys on different channels at identical pitch isolate the envelope
  // behavior from an intentional pitch change during last-note fallback.
  for(float filterAmount:{0.f,48.f}){
  for(auto v:{&a,&b}){v->Reset(sr);v->SetVoiceMode(1,0,true);
   v->SetParameters(-6,4,20,.1f,100);v->SetWaveforms(3,3);v->SetFilter(20000,0,0);
   if(filterAmount>0){v->SetFilter(400,60,100);v->SetFilterEnvelope(filterAmount,0,4,20,.1f,100);}
   v->Midi(0x90,60,100);v->Midi(0x91,60,100);
   for(int i=0;i<int(sr*.3);++i)v->Process();}
  a.Midi(0x81,60,0);
  for(int i=0;i<int(sr*.05);++i){a.Process();b.Process();
   if(i>int(sr*.007))check(std::abs(a.PreFX().left-b.PreFX().left)<1e-5f,
                         "note-off fallback must not create a fresh amplitude/filter attack");}
  }
 }
 {
  auto changingOwner=std::make_unique<sawstar::Synth>();
  auto continuingOwner=std::make_unique<sawstar::Synth>();
  auto& changing=*changingOwner;auto& continuing=*continuingOwner;
  for(auto s:{&changing,&continuing}){s->Reset(sr);s->SetVoiceMode(2,0,true);
   s->SetParameters(-6,1,1,1,1);s->SetFilter(20000,0,0);s->SetWaveforms(3,3);
   s->Midi(0x90,48,127);for(int i=0;i<int(sr*.05)+3;++i)s->Process();}
  changing.Midi(0x90,96,127);
  // A four-octave jump must initially follow the continued old waveform,
  // not merely match its final sample and immediately switch derivatives.
  for(int i=0;i<int(sr*.0005);++i){changing.Process();continuing.Process();
   check(std::abs(changing.PreFX().left-continuing.PreFX().left)<.05f,
         "transition must retain the old waveform beyond the first sample");}
 }
 for(int mode:{1,2}){
  auto owner=std::make_unique<sawstar::Synth>();auto& s=*owner;s.Reset(sr);s.SetVoiceMode(mode,0,true);
  s.SetParameters(-6,1,1,1,1);s.SetFilter(1500,80,100);
  s.Midi(0x90,48,127);for(int i=0;i<1000;++i)s.Process();
  // Retarget faster than any transition can finish; then release immediately.
  for(int n=0;n<100;++n){s.Midi(0x90,60+n%24,100);
   for(int i=0;i<7;++i){auto x=s.ProcessStereo();
    check(std::isfinite(x.left)&&std::isfinite(x.right)&&std::abs(x.left)<=.98001f&&std::abs(x.right)<=.98001f,
          "dense mono crossfades remain finite and bounded");}
   s.Midi(0x80,60+n%24,0);}
  s.Midi(0x80,48,0);for(int i=0;i<int(sr*.2);++i)s.Process();
  check(s.ActiveVoices()==0&&s.Process()==0,"short release must finish even with a transition tail");
  for(int target:{0,1,2}){
   s.SetVoiceMode(mode,0,true);s.Midi(0x90,48,127);for(int i=0;i<1000;++i)s.Process();
   s.Midi(0x90,84,127);s.Process();s.SetVoiceMode(target,0,true);
   for(int ch=0;ch<16;++ch)s.Midi(0xb0|ch,120,0);
   check(s.Process()==0&&s.ActiveVoices()==0,"mode switch and panic must clear transition audio");
  }
 }
 for(int mode:{1,2})for(bool pedal:{false,true}){
  auto aOwner=std::make_unique<sawstar::Synth>();
  auto bOwner=std::make_unique<sawstar::Synth>();
  auto& a=*aOwner;auto& b=*bOwner;
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
  auto aOwner=std::make_unique<sawstar::Synth>();
  auto bOwner=std::make_unique<sawstar::Synth>();
  auto& a=*aOwner;auto& b=*bOwner;
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
  auto owner=std::make_unique<sawstar::Synth>();auto& s=*owner;auto reference=std::make_unique<sawstar::Synth>();s.Reset(sr);s.SetParameters(-6,1,1,1,10);
  s.SetVoiceMode(mode,0,true);s.SetWaveforms(wave,wave);s.SetFilter(20000,0,0);
  s.Midi(0x90,48,127);
  for(int n=0;n<12;++n){
   for(int i=0;i<static_cast<int>(sr*.013)+n*7;++i)s.Process();
   *reference=s;reference->Process();auto expected=reference->PreFX();
   s.Midi(0x90,84,127);s.Process();auto next=s.PreFX();
   check(std::abs(next.left-expected.left)<1e-5f&&std::abs(next.right-expected.right)<1e-5f,
         "mono/legato new note must continue the outgoing waveform naturally");
   for(int i=0;i<static_cast<int>(sr*.011)+n*3;++i)s.Process();
   *reference=s;reference->Process();expected=reference->PreFX();
   s.Midi(0x80,84,0);s.Process();next=s.PreFX();
   check(std::abs(next.left-expected.left)<1e-5f&&std::abs(next.right-expected.right)<1e-5f,
         "mono/legato held-note fallback must continue the outgoing waveform naturally");
  }
  s.Midi(0x80,48,0);for(int i=0;i<int(sr);++i)s.Process();
  check(s.ActiveVoices()==0&&s.Process()==0,"mono correction must not leave a hanging voice");
  s.Midi(0x90,48,127);for(int i=0;i<1000;++i)s.Process();
  s.Midi(0x90,84,127);s.Process();s.Midi(0xb0,120,0);
  check(s.Process()==0,"mono panic must silence an active correction");
 }
 for(bool full:{false,true}){
  auto owner=std::make_unique<sawstar::Synth>();auto& s=*owner;s.Reset(sr);s.SetParameters(0,1,1,1,10);s.SetOutputBoost(18);
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
