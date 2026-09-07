// SPDX-License-Identifier: MIT
#include "midi/Arpeggiator.h"
#include "engine/Synth.h"
#include <vector>
#include <memory>
#include <array>
#include <cstdlib>
#include <iostream>
#include <cmath>
void check(bool v,const char* s){if(!v){std::cerr<<s<<'\n';std::exit(1);}}
struct Event {int time,status,note,value;bool operator==(const Event& b)const{return time==b.time&&status==b.status&&note==b.note&&value==b.value;}};
struct Rig {
 sawstar::Arpeggiator arp;int time=0;std::vector<Event> events;
 Rig(){arp.Init(8000);}
 auto Send(){return [this](int s,int n,int v){events.push_back({time,s,n,v});};}
 void Set(bool on=true,int mode=0,float gate=50,int oct=1,float swing=0,bool hold=false,bool running=true){arp.Set(on,mode,2,gate,oct,swing,hold,120,running,Send());}
 void Midi(int s,int n,int v){arp.Midi(s,n,v,Send());}
 void Run(int n){for(int i=0;i<n;++i,++time)arp.Process(Send());}
 std::vector<Event> Ons(){std::vector<Event> out;for(auto e:events)if((e.status&240)==144&&e.value)out.push_back(e);return out;}
};
int main(){
 for(int mode=0;mode<5;++mode){auto r=std::make_unique<Rig>();r->Set(true,mode);r->Midi(144,67,91);r->Midi(144,60,83);r->Midi(144,64,75);r->Run(6001);auto a=r->Ons();
  const int expected[][7]={{60,64,67,60,64,67,60},{67,64,60,67,64,60,67},{60,64,67,64,60,64,67},{0},{67,60,64,67,60,64,67}};
  check(a.size()==7,"rate emits seven notes at 0..6000");
  for(size_t i=0;i<a.size();++i){check(a[i].time==int(i)*1000,"sample accurate straight rate");if(mode!=3)check(a[i].note==expected[mode][i],"arp ordering");else check(a[i].note==60||a[i].note==64||a[i].note==67,"random membership");}
  for(auto e:r->events)if((e.status&240)==128)check(e.time%1000==500,"gate note-off halfway through step");
 }
 auto r=std::make_unique<Rig>();r->Set(true,0,50,2);r->Midi(145,60,97);r->Run(2001);auto a=r->Ons();check(a[0].note==60&&a[1].note==72&&a[0].status==145&&a[0].value==97,"octaves preserve channel velocity");
 r=std::make_unique<Rig>();r->Set(true,0,100,1,50);r->Midi(144,60,100);r->Run(4001);a=r->Ons();check(a.size()==5&&a[1].time==1500&&a[2].time==2000&&a[3].time==3500,"swing preserves pair length");
 // Hold replacement, hold release, transport stop and restart without orphan notes.
 r=std::make_unique<Rig>();r->Set(true,0,50,1,0,true);r->Midi(144,60,100);r->Midi(128,60,0);r->Run(1200);check(r->Ons().size()==2,"hold repeats released key");r->Midi(144,67,90);r->Midi(128,67,0);r->Run(2200);a=r->Ons();check(a.back().note==67,"new hold phrase replaces old chord");
 r->Set(true,0,50,1,0,false);auto n=r->Ons().size();r->Run(2000);check(r->Ons().size()==n,"hold off clears latched phrase");
 r->Midi(144,60,100);r->Run(100);r->Set(true,0,50,1,0,true,false);n=r->Ons().size();r->Run(2000);check(r->Ons().size()==n,"transport stop flushes hold");
 // Pedal release and controller reset cannot sustain generated steps.
 r=std::make_unique<Rig>();r->Set();r->Midi(144,60,100);r->Midi(176,64,127);r->Midi(128,60,0);r->Run(1200);check(r->Ons().size()==2,"pedal latches root");r->Midi(176,64,0);n=r->Ons().size();r->Run(2000);check(r->Ons().size()==n,"pedal release stops arp");
 r->Midi(144,60,100);r->Run(1);r->Midi(176,123,0);n=r->Ons().size();r->Run(2000);check(r->Ons().size()==n,"all notes off clears pattern");
 // Block boundary independence: repeated parameter/transport polling cannot restart.
 auto p=std::make_unique<Rig>(),q=std::make_unique<Rig>();p->Set();q->Set();p->Midi(144,60,100);q->Midi(144,60,100);p->Run(8192);for(int i=0;i<8192;i+=64){q->Set();q->Run(64);}check(p->events==q->events,"block size independence");
 // Disabled mode preserves input note events, including velocity-zero note-off.
 r=std::make_unique<Rig>();r->Set(false);r->Midi(144,60,100);r->Midi(144,60,0);r->Run(2000);check(r->events.size()==2&&r->events[1].status==144&&r->events[1].value==0,"disabled transparent MIDI path");
 // Integration: gate=100% same-sample off/on and rapid chord changes still release.
 for(int mode=0;mode<3;++mode){auto synth=std::make_unique<sawstar::Synth>();auto arp=std::make_unique<sawstar::Arpeggiator>();synth->Reset(44100);synth->SetParameters(0,1,1,.7,10);synth->SetVoiceMode(mode,0,true);arp->Init(44100);
  auto send=[&](int s,int n,int v){synth->Midi(s,n,v);};arp->Set(true,2,3,100,4,75,true,180,true,send);
  arp->Midi(144,60,100,send);arp->Midi(144,64,90,send);arp->Midi(144,67,80,send);
  for(int i=0;i<44100;++i){arp->Process(send);check(std::isfinite(synth->Process()),"finite arp audio");}
  arp->Clear(send);for(int i=0;i<22050;++i){arp->Process(send);synth->Process();}check(synth->ActiveVoices()==0,"arp reset releases poly mono legato");
 }
}
