// SPDX-License-Identifier: MIT
#include "midi/BlockMidiQueue.h"
#include "midi/EditorMidiTracker.h"
#include "engine/Synth.h"
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
  sawstar::EditorMidiTracker tracker;
  bool TakeMidiMsgFromEditorOverflow(){bool old=overflow;overflow=false;return old;}
  void SignalMidiMsgFromEditorOverflow(){overflow=true;}
  void OnMidiMsgFromEditorOverflow(){tracker.BeginRecovery();}
 } mPlug;
 bool bypass=false;
 bool GetBypassed()const{return bypass;}
 sawstar::BlockMidiQueue events;
 std::unique_ptr<sawstar::Synth> synth=std::make_unique<sawstar::Synth>();
 Fixture(){synth->Reset(48000);}
 void push(int status,int note,int value){
  if(fromEditor.events.size()<32)fromEditor.events.push_back({0,status,note,value,true});
  else if((status&0xf0)==0x80||(status&0xf0)==0x90)mPlug.SignalMidiMsgFromEditorOverflow();
 }
 void process(int frames){
  data.numSamples=frames;
  // Match the pinned wrapper: MIDI forwarding before audio, callback after it.
  while(!fromEditor.events.empty()){events.Push(fromEditor.events.front());fromEditor.events.pop_front();}
  if(!bypass){
   if(frames>0)mPlug.tracker.ReleaseSome(16,[&](int ch,int note){synth->Midi(0x80|ch,note,0);});
   events.Process(frames,[&](const auto& msg){
    if(!mPlug.tracker.Observe(msg.status,msg.data1,msg.data2))synth->Midi(msg.status,msg.data1,msg.data2);
   },[&](int){synth->ProcessStereo();},[]{std::abort();});
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
 std::cout<<"Overflow survives zero-frame/bypass calls until a rendered block.\n";
}
