// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include "midi/Arpeggiator.h"
#include <memory>
#include <iostream>
#include <cstdlib>
void check(bool ok,const char* message){if(!ok){std::cerr<<message<<'\n';std::exit(1);}}
struct Rig {
  sawstar::Synth synth;sawstar::Arpeggiator arp;int ons=0;
  explicit Rig(int mode){synth.Reset(8000);synth.SetParameters(-12,1,1,.7,10);synth.SetVoiceMode(mode,0,true);arp.Init(8000);}
  auto Send(){return [this](int s,int n,int v){if((s&240)==144&&v)++ons;synth.Midi(s,n,v);};}
  void Set(bool on,bool hold=false){arp.Set(on,0,2,100,1,0,hold,120,true,Send());}
  void Midi(int s,int n,int v){arp.Midi(s,n,v,Send());}
  void Run(int n){while(n--){arp.Process(Send());check(std::isfinite(synth.Process()),"finite output");}}
};
int main(){
  for(int mode=0;mode<3;++mode){
    auto r=std::make_unique<Rig>(mode);auto& s=r->synth;
    s.Midi(144,60,100);s.Midi(144,60,110);
    check(s.ActiveVoices()==1,"duplicate retriggers one voice");
    s.Midi(128,60,0);check(s.Held(60),"first off retains note");
    s.Midi(144,60,0);check(!s.Held(60),"velocity-zero final off releases");
    r->Run(1000);check(!s.ActiveVoices(),"final off reaches silence");
    s.Midi(128,60,0);s.Midi(144,60,90);s.Midi(128,60,0);check(!s.Held(60),"unmatched off does not underflow");
    s.Midi(144,60,100);s.Midi(144,60,100);s.Midi(176,64,127);
    s.Midi(128,60,0);check(s.Held(60),"pedal does not consume remaining press");
    s.Midi(128,60,0);r->Run(500);check(!s.Held(60)&&s.ActiveVoices()>0,"pedal retains released sound");
    s.Midi(176,64,0);r->Run(1000);check(!s.ActiveVoices(),"pedal release reaches silence");
    for(int cc:{120,123}){
      s.Midi(144,60,90);s.Midi(144,60,90);s.Midi(145,60,100);
      s.Midi(176,cc,0);check(s.Held(60),"panic is channel-local");
      s.Midi(129,60,0);check(!s.Held(60),"other channel final release");
      s.Midi(144,60,90);s.Midi(128,60,0);check(!s.Held(60),"panic cleared all counts");
    }
    s.Midi(144,60,90);s.Midi(144,60,90);s.Midi(176,121,0);s.Midi(128,60,0);
    check(s.Held(60),"controller reset preserves presses");s.Midi(128,60,0);
    s.Midi(144,60,90);s.SetVoiceMode((mode+1)%3,0,true);check(!s.Held(60),"mode change clears phrase");
    s.Reset(8000);check(!s.Held(60),"audio reset clears counts");
    for(bool initiallyOn:{false,true}){
      r=std::make_unique<Rig>(mode);r->Set(initiallyOn);
      r->Midi(144,60,100);r->Midi(144,60,110);r->Run(1);
      if(!initiallyOn)r->Set(true);
      r->Run(1);r->Set(false);
      r->Midi(128,60,0);check(r->synth.Held(60),"ARP bypass keeps overlapping press");
      r->Midi(144,60,120);r->Midi(128,60,0);check(r->synth.Held(60),"new press after bypass remains paired");
      r->Midi(128,60,0);r->Run(1000);check(!r->synth.ActiveVoices(),"bypass final off silences");
    }
    r=std::make_unique<Rig>(mode);r->Set(true);
    r->Midi(144,60,100);r->Midi(144,60,110);r->Midi(128,60,0);r->Run(1200);
    check(r->ons>=2,"ARP retains duplicate root");int before=r->ons;
    r->Midi(128,60,0);r->Run(2000);check(r->ons==before&&!r->synth.ActiveVoices(),"ARP final release stops");
    r=std::make_unique<Rig>(mode);r->Set(true,true);
    r->Midi(144,60,100);r->Midi(144,60,110);r->Midi(176,123,0);
    r->Midi(144,67,100);r->Midi(128,67,0);r->Run(1200);check(r->ons==2,"HOLD starts cleanly after panic");
    r->Set(true,false);r->Run(1000);check(!r->synth.ActiveVoices(),"HOLD off releases");
  }
  auto r=std::make_unique<Rig>(0);
  for(int n=40;n<60;++n)r->synth.Midi(144,n,100);
  check(r->synth.Held(40),"voice stealing preserves input key state");
  for(int n=40;n<60;++n)r->synth.Midi(128,n,0);
  r->Run(1000);check(!r->synth.ActiveVoices(),"stolen and active notes release");
  std::cout<<"Overlapping notes, bypass, modes, pedal, panic and stealing passed\n";
}
