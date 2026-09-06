// SPDX-License-Identifier: MIT
#include "dsp/FilterModulation.h"
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
int main(){
  using namespace sawstar;
  for(float sr:{8000.f,44100.f,48000.f,96000.f}) {
    FilterModulation m;m.Init(sr);m.Set(400,0,100,10,100,1,10);m.Trigger(true);
    check(m.Process(60,true)==400,"tracking pivot");
    for(int i=0;i<16;++i)m.Process(72,true);
    check(m.Process(72,true)==800,"octave tracking");
    m.Set(400,0,50,10,100,1,10);m.Trigger(true);
    check(std::abs(m.Process(72,true)-400*std::sqrt(2.f))<.01f,"half tracking");
    for(float amount:{-24.f,24.f}) {
      m.Init(sr);m.Set(400,amount,0,10,100,1,10);m.Trigger(true);
      float first=m.Process(60,true),peak=0;
      for(int i=0;i<sr/10;++i)peak=m.Process(60,true);
      check(std::abs(peak-(amount>0?1600:100))<.1f,"envelope octave range");
      check(amount>0?peak>first:peak<first,"attack direction");
      for(int i=0;i<sr/2;++i)peak=m.Process(60,false);
      check(std::abs(peak-400)<.01f,"release returns to base");
    }
    m.Init(sr);m.Set(400,24,0,1,10,0,10);m.Trigger(true);
    float end=0;for(int i=0;i<sr/2;++i)end=m.Process(60,true);
    check(std::abs(end-400)<.01f,"decay reaches zero sustain");
    m.Trigger(true);check(m.Process(60,true)<800,"hard retrigger starts low");
    m.Set(20000,96,100,1,1,1,1);m.Trigger(true);
    for(int i=0;i<sr/10;++i){float hz=m.Process(127,true);check(hz>=20&&hz<=std::min(20000.f,sr*.45f),"upper frequency bound");}
    m.Set(20,-96,100,1,1,1,1);m.Trigger(true);
    for(int i=0;i<100;++i)check(m.Process(0,true)==20,"lower frequency bound");
    Synth a,b;a.Reset(sr);b.Reset(sr);
    for(auto* s:{&a,&b}){s->SetParameters(-12,1,20,.7,10);s->SetSaw(20,65,75);s->SetFilter(600,30,100);s->Midi(0x90,60,100);}
    b.SetFilterEnvelope(0,0,2000,2000,1,2000);
    for(int i=0;i<sr/5;++i){auto x=a.ProcessStereo(),y=b.ProcessStereo();check(x.left==y.left&&x.right==y.right,"zero modulation preserves audio");}
    a.SetFilterEnvelope(96,100,1,1,1,10);
    for(int n=0;n<128;++n){a.Midi(0x90,n,127);for(int i=0;i<64;++i){auto x=a.ProcessStereo();check(std::isfinite(x.left)&&std::abs(x.left)<=1&&std::isfinite(x.right)&&std::abs(x.right)<=1,"polyphonic modulation bounds");}}
    a.Midi(0xb0,64,127);a.Midi(0x80,127,0);
    check(a.ActiveVoices()>0,"sustain keeps voices");
    a.Midi(0xb0,120,0);auto silence=a.ProcessStereo();check(silence.left==0&&silence.right==0,"modulated panic silence");
  }
}
