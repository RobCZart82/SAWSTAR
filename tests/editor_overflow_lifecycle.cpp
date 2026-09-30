// SPDX-License-Identifier: MIT
#include "midi/BlockMidiQueue.h"
#include "midi/EditorMidiTracker.h"
#include "engine/Synth.h"
#include "midi/Arpeggiator.h"
#include <vector>
#include <deque>
#include <memory>
#include <iostream>
#include <cstdlib>
void check(bool ok,const char* text){if(!ok){std::cerr<<text<<'\n';std::exit(1);}}
struct Fixture {
 struct { int numSamples=0; } data;
 struct FIFO {
  std::deque<sawstar::BlockMidiEvent> events;
  bool WasEmpty()const{return events.empty();}
 } fromEditor;
 struct Plug {
  bool overflow=false;
  bool processBypass=false;
  bool ProcessAudioWhileBypassed()const{return processBypass;}
  sawstar::EditorMidiTracker tracker;
  bool TakeMidiMsgFromEditorOverflow(){bool old=overflow;overflow=false;return old;}
  void SignalMidiMsgFromEditorOverflow(){overflow=true;}
  void OnMidiMsgFromEditorOverflow(){tracker.BeginRecovery();}
 } mPlug;
 bool bypass=false;
 bool GetBypassed()const{return bypass;}
 sawstar::BlockMidiQueue events;
 sawstar::Arpeggiator arp;
 std::vector<float> output;
 std::unique_ptr<sawstar::Synth> synth=std::make_unique<sawstar::Synth>();
 Fixture(){synth->Reset(48000);arp.Init(48000);}
 void host(int status,int note,int value,int offset=0){events.Push({offset,status,note,value,false});}
 void enableArp(bool on){arp.Set(on,0,2,55,1,0,false,120,true,[&](int s,int n,int v){synth->Midi(s,n,v);});}
 void push(int status,int note,int value){
  if(fromEditor.events.size()<32)fromEditor.events.push_back({0,status,note,value,true});
  else if((status&0xf0)==0x80||(status&0xf0)==0x90)mPlug.SignalMidiMsgFromEditorOverflow();
 }
 void process(int frames){
  data.numSamples=frames;output.clear();
  auto send=[&](int s,int n,int v){synth->Midi(s,n,v);};
  // Match the pinned wrapper: MIDI forwarding before audio, callback after it.
  while(!fromEditor.events.empty()){events.Push(fromEditor.events.front());fromEditor.events.pop_front();}
  #include "editor_bypass_branch.inc"
  {} else {
   if(frames>0)mPlug.tracker.ReleaseSome(16,[&](int ch,int note){arp.Midi(0x80|ch,note,0,send);});
   events.Process(frames,[&](const auto& msg){
    if(!msg.fromEditor || !mPlug.tracker.Observe(msg.status,msg.data1,msg.data2))arp.Midi(msg.status,msg.data1,msg.data2,send);
   },[&](int){
    arp.Process(send);const auto value=synth->ProcessStereo();
    float l=1,r=1;float* outputs[]={&l,&r};
    sawstar::WriteHostOutput(value,outputs,2,0,bypass);
    output.push_back(l);output.push_back(r);
   },[]{std::abort();});
  }
#include "editor_overflow_hook.inc"
 }
};
int main(){
 for(bool bypass : {false,true}){
  Fixture f;f.push(0x90,60,100);
  for(int i=0;i<31;++i)f.push(0xb0,7,0);
  f.push(0x80,60,0); // dropped from full upstream FIFO
  f.bypass=bypass;
  f.process(bypass?64:0);
  check(f.mPlug.overflow,"Overflow notification consumed before MIDI reached tracker");
  check(!f.synth->Held(60),"Non-rendering callback unexpectedly started a note");
  f.bypass=false;f.process(64);
  check(f.mPlug.tracker.PendingReleaseCount()==1,"Accepted note missing from recovery snapshot");
  f.process(64);
  check(!f.synth->Held(60),"Editor note remained held after deferred recovery");
  f.process(48000);
  check(f.synth->ActiveVoices()==0,"Recovered voice failed to release");
 }
 // Opted-in synth policy: MIDI and DSP advance while output is muted.
 for(bool arpOn : {false,true}) {
  Fixture f,reference;f.mPlug.processBypass=true;reference.mPlug.processBypass=true;
  f.enableArp(arpOn);reference.enableArp(arpOn);
  auto both=[&](int s,int n,int v,int offset=0){f.host(s,n,v,offset);reference.host(s,n,v,offset);};
  both(0x90,60,100);f.process(128);reference.process(128);
  f.bypass=true;
  both(0xb0,64,127);both(0x80,60,0,17); // pre-bypass note released under pedal
  both(0x90,64,100,31);both(0x80,64,0,95); // complete gesture during bypass
  f.process(128);reference.process(128);
  check(f.events.Size()==0,"Bypass left stale MIDI queued");
  check(f.output.size()==256,"Bypass skipped engine processing");
  for(float sample:f.output)check(sample==0,"Bypassed instrument emitted audio");
  both(0xb0,64,0);both(0x90,67,100,23);
  f.process(128);reference.process(128);
  f.bypass=false;
  f.process(128);reference.process(128);
  check(f.output==reference.output,"Unbypass differs from continuously running engine");
  both(0x80,67,0);f.process(48000);reference.process(48000);
  check(f.synth->ActiveVoices()==0,"Bypass lifecycle left active voices");
 }
 // Editor overflow during bypass must preserve a same-note host contribution.
 Fixture f;f.mPlug.processBypass=true;f.bypass=true;
 f.host(0x90,60,100);f.push(0x90,60,100);
 for(int i=0;i<31;++i)f.push(0xb0,7,0);
 f.push(0x80,60,0);f.process(64);f.process(64);
 check(f.synth->Held(60),"Bypass editor recovery released host ownership");
 check(f.events.Size()==0,"Bypass recovery queued stale events");
 f.host(0x80,60,0);f.process(48000);
 check(!f.synth->Held(60)&&f.synth->ActiveVoices()==0,"Host release during bypass was lost");
 std::cout<<"Overflow deferral and opt-in silent bypass lifecycle pass.\n";
}
