// SPDX-License-Identifier: MIT
#include "SynthTestRig.h"
#include "midi/EditorMidiTracker.h"
#include <memory>
#include <iostream>
#include <cstdlib>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
int main(){
 for(bool enabled:{false,true})for(bool hold:{false,true})for(bool pedal:{false,true}){
  auto rig=std::make_unique<Rig>();rig->init(8000);
  EditorMidiTracker tracker;auto values=DefaultSnapshot();
  values[size_t(ParameterId::ArpEnabled)]=enabled;
  values[size_t(ParameterId::ArpHold)]=hold;
  auto apply=[&](bool running){
   const bool cleared=ApplyEngineControls(rig->synth,rig->arp,
    [&](ParameterId id){return values[size_t(id)];},
    [&](ParameterId id){return int(values[size_t(id)]);},120,running);
   if(cleared)tracker.Clear();
   return cleared;
  };
  check(!apply(true),"Starting transport discarded ownership");
  for(int channel:{0,15}){
   tracker.Observe(0x90|channel,60,100);rig->midi(0x90|channel,60,100);
   tracker.Observe(0x90|channel,60,90);rig->midi(0x90|channel,60,90);
   if(pedal)rig->midi(0xb0|channel,64,127);
  }
  for(int i=0;i<64;++i)rig->process();
  tracker.BeginRecovery(); // Stop must also discard pending overflow releases.
  check(apply(false)==enabled,"Transport clear report does not match ARP policy");
  if(enabled)check(tracker.PendingReleaseCount()==0,"Stop left recovery debt");
  check(!apply(false),"Repeated stopped blocks discarded fresh input");
  if(!enabled){
   check(tracker.PendingReleaseCount()==4,"Plain MIDI stop discarded editor ownership");
   continue;
  }
  for(int channel:{0,15}){
   rig->midi(0x90|channel,60,100); // Fresh host root while stopped.
   check(tracker.Observe(0x80|channel,60,0),"Stale editor release can steal a host root");
   check(tracker.Observe(0x90|channel,60,0),"Repeated velocity-zero release can steal a host root");
   tracker.Observe(0x90|channel,60,100);rig->midi(0x90|channel,60,100);
   if(!tracker.Observe(0x80|channel,60,0))rig->midi(0x80|channel,60,0);
  }
  int ons=0;auto send=[&](int s,int,int){if((s&0xf0)==0x90)++ons;};
  for(int i=0;i<3000;++i)rig->arp.Process(send);
  check(ons>0,"Fresh host roots stopped after editor releases");
  check(!apply(true),"Transport restart discarded fresh host roots");
  for(int channel:{0,15})rig->midi(0x80|channel,60,0);
  if(hold){values[size_t(ParameterId::ArpHold)]=0;apply(true);}
  ons=0;for(int i=0;i<3000;++i)rig->arp.Process(send);
  check(ons==0,"Host roots failed to release after transport lifecycle");
 }
}
