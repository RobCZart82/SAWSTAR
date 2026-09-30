// SPDX-License-Identifier: MIT
#include <cstddef>
#include <deque>
#include <cstdlib>
#include <iostream>
using IMidiMsg = int;
// Deterministic SPSC contract fixture: inject a producer append during Pop.
// The compiled reset method below is extracted from the production iPlug patch.
struct Queue {
  std::deque<int> data;
  bool appendOnPop=false;
  std::size_t pops=0;
  std::size_t ElementsAvailable() const {return data.size();}
  bool Pop(int& msg) {
    ++pops;
    if(data.empty())return false;
    msg=data.front(); data.pop_front();
    if(appendOnPop)data.push_back(999);
    return true;
  }
};
struct Adapter {
  Queue mMidiMsgsFromEditor;
  bool overflow=false;
  bool TakeMidiMsgFromEditorOverflow(){const bool old=overflow;overflow=false;return old;}
#include "editor_reset_hook.inc"
};
void check(bool ok,const char* reason){if(!ok){std::cerr<<reason<<'\n';std::exit(1);}}
int main(){
  Adapter a;
  a.mMidiMsgsFromEditor.data={60,61,62};a.overflow=true;
  a.DiscardPendingMidiFromEditor();
  check(a.mMidiMsgsFromEditor.data.empty(),"pre-reset editor events replay after reset");
  check(!a.overflow,"pre-reset recovery flag survived");
  a.DiscardPendingMidiFromEditor();
  check(a.mMidiMsgsFromEditor.pops==3,"empty reset attempted reads");
  a.mMidiMsgsFromEditor.data={64};
  a.DiscardPendingMidiFromEditor();
  a.mMidiMsgsFromEditor.data.push_back(65);
  int note=0;check(a.mMidiMsgsFromEditor.Pop(note)&&note==65,"fresh post-reset input lost");
  for(int i=0;i<1024;++i)a.mMidiMsgsFromEditor.data.push_back(i);
  a.mMidiMsgsFromEditor.appendOnPop=true;
  const auto before=a.mMidiMsgsFromEditor.pops;
  a.DiscardPendingMidiFromEditor();
  check(a.mMidiMsgsFromEditor.pops-before==1024,"reset loop follows producer indefinitely");
  check(a.mMidiMsgsFromEditor.data.size()==1024,"post-snapshot messages discarded");
  for(int value:a.mMidiMsgsFromEditor.data)check(value==999,"old message survived snapshot drain");
  std::cout<<"Editor reset discards only the pending snapshot.\n";
}
