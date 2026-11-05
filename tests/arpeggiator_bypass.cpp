// SPDX-License-Identifier: MIT
#include "midi/Arpeggiator.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

struct Event { int status,note,value; };
void Check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
int main(){
  auto arp=std::make_unique<sawstar::Arpeggiator>();arp->Init(8000);
  std::vector<Event> events;
  auto send=[&](int s,int n,int v){events.push_back({s,n,v});};
  auto set=[&](bool on,int mode=0,int octaves=1){arp->Set(on,mode,2,50,octaves,0,false,120,true,send);};
  set(false);
  // Repeated pitches and channels must survive bypass without expanding into
  // generated octave notes. Released keys must not reappear on enable.
  const Event input[]={{0x90,60,91},{0x90,60,83},{0x91,64,75},
                       {0x90,67,99},{0x90,67,0},{0xb0,1,80}};
  for(const auto& e:input)arp->Midi(e.status,e.note,e.value,send);
  Check(events.size()==6,"bypass forwards exactly the input events");
  for(size_t i=0;i<events.size();++i)
    Check(events[i].status==input[i].status&&events[i].note==input[i].note&&events[i].value==input[i].value,"bypass preserves event data and order");
  set(true);events.clear();
  for(int i=0;i<=1000;++i)arp->Process(send);
  std::vector<Event> ons;
  for(auto e:events)if((e.status&240)==0x90&&e.value)ons.push_back(e);
  Check(ons.size()==2&&ons[0].status==0x90&&ons[0].note==60&&ons[0].value==83&&ons[1].status==0x91&&ons[1].note==64,"enable rebuilds unique held roots with current velocity and channel");
  set(false);events.clear();
  arp->Midi(0x80,60,0,send);
  Check(events.empty(),"first duplicate release preserves the restored voice");
  arp->Midi(0x80,60,0,send);
  Check(events.size()==1&&events[0].status==0x80&&events[0].note==60,"last duplicate release balances the restored voice");
  arp->Midi(0xb1,123,0,send);set(true);events.clear();
  for(int i=0;i<2000;++i)arp->Process(send);
  Check(events.empty(),"controller-cleared and released bypass keys stay absent");
  // Settings changed while bypassed take effect on the first generated note.
  set(false,1,2);arp->Midi(0x92,60,101,send);set(true,1,2);events.clear();arp->Process(send);
  Check(events.size()==1&&events[0].status==0x92&&events[0].note==72&&events[0].value==101,"enable uses the latest bypass mode and octave settings");
}
