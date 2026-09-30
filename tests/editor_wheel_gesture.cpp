// SPDX-License-Identifier: MIT
#include "gui/Controls/PerformanceWheel.h"
#include "midi/EditorWheelMailbox.h"
#include <vector>
#include <iostream>
#include <cstdlib>
struct Delegate : iplug::TestDelegate {
 std::vector<iplug::IMidiMsg> sent;
 sawstar::EditorWheelMailbox mailbox;
 void SendMidiMsgFromUI(const iplug::IMidiMsg& m)override {
  sent.push_back(m);mailbox.Publish(m.mStatus,m.mData1,m.mData2,m.mOffset);
 }
};
void check(bool ok,const char* message){if(!ok){std::cerr<<message<<'\n';std::exit(1);}}
int main(){
 using sawstar::gui::PerformanceWheel;
 Delegate d;PerformanceWheel pitch({0,0,20,100},true);pitch.SetTestDelegate(&d);
 pitch.OnMidi({0xe0,0,100,0});const auto host=pitch.GetValue();
 pitch.EndGesture();
 check(d.sent.empty()&&pitch.GetValue()==host,"Idle editor close overwrote host pitch");
 pitch.OnMouseDown(0,0,{});pitch.OnMouseDrag(0,25,0,0,{});
 pitch.EndGesture();
 check(d.sent.back().mData1==0&&d.sent.back().mData2==64,"Close failed to center active pitch");
 const auto count=d.sent.size();pitch.EndGesture();pitch.OnMouseUp(0,0,{});
 check(d.sent.size()==count,"Close plus mouse-up sent duplicate pitch reset");
 int final=-1;d.mailbox.Drain([&](int,int lo,int hi){final=lo+(hi<<7);});
 check(final==8192,"Close center did not survive latest-value delivery");
 pitch.OnMouseDown(0,0,{});pitch.OnTouchCancelled(0,0,{});
 check(d.sent.back().mData2==64,"Cancelled pitch gesture remained bent");
 pitch.OnMidi({0xe0,0,90,0});const auto before=d.sent.size();pitch.EndGesture();
 check(d.sent.size()==before&&pitch.GetValue()>.5,"Feedback created gesture ownership");
 Delegate modD;PerformanceWheel mod({0,0,20,100},false);mod.SetTestDelegate(&modD);
 mod.OnMouseDown(0,20,{});const auto value=mod.GetValue();const auto sent=modD.sent.size();
 mod.EndGesture();mod.OnMouseUp(0,0,{});
 check(mod.GetValue()==value&&modD.sent.size()==sent,"Close reset latching modulation");
 pitch.OnMouseDown(0,10,{});pitch.OnMouseUp(0,0,{});
 check(pitch.GetValue()==.5,"Normal pitch release stopped springing back");
 std::cout<<"Actual wheel gesture logic: close/cancel/host feedback/MOD pass.\n";
}
