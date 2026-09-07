// SPDX-License-Identifier: MIT
#pragma once
#include "IControl.h"
#include <algorithm>
#include <cmath>
namespace sawstar::gui {
// Channel 1 audition controls; incoming MIDI on other channels stays independent.
class PerformanceWheel final : public iplug::igraphics::IControl {
public:
  PerformanceWheel(const iplug::igraphics::IRECT& bounds,bool pitch)
  : IControl(bounds),pitch_(pitch) { SetValue(pitch?.5:0);SetWantsMidi(true); }
  void Draw(iplug::igraphics::IGraphics& g) override {
    using namespace iplug::igraphics;
    const auto r=mRECT.GetPadded(-3);
    g.FillRect(IColor(255,22,40,50),r);
    g.DrawRect(IColor(255,117,137,147),r);
    for(int i=1;i<8;++i)g.DrawLine(IColor(255,55,75,85),r.L+2,r.T+r.H()*i/8,r.R-2,r.T+r.H()*i/8);
    float y=r.B-3-static_cast<float>(GetValue())*(r.H()-6);
    g.FillRect(IColor(255,54,170,226),IRECT(r.L+2,y-1,r.R-2,y+1));
  }
  void OnMouseDown(float x,float y,const iplug::igraphics::IMouseMod&) override { Move(y); }
  void OnMouseDrag(float x,float y,float dx,float dy,const iplug::igraphics::IMouseMod&) override { Move(y); }
  void OnMouseUp(float,float,const iplug::igraphics::IMouseMod&) override { if(pitch_)Send(.5); }
  void OnMouseDblClick(float,float,const iplug::igraphics::IMouseMod&) override { Send(pitch_?.5:0); }
  void OnMidi(const iplug::IMidiMsg& msg) override {
    if((msg.mStatus&15)!=0)return;
    if(pitch_ && (msg.mStatus&240)==0xe0) {
      int raw=msg.mData1+(msg.mData2<<7);
      SetValue(raw<8192?raw/16384.:.5+(raw-8192)/16382.);SetDirty(false);
    } else if(!pitch_ && (msg.mStatus&240)==0xb0 && msg.mData1==1) {
      SetValue(msg.mData2/127.);SetDirty(false);
    }
  }
private:
  void Move(float y){Send(std::max(0.,std::min(1.,double(mRECT.B-y)/mRECT.H())));}
  void Send(double value){
    SetValue(value);SetDirty(false);
    iplug::IMidiMsg msg;
    if(pitch_) {
      int raw=static_cast<int>(std::round(value<=.5?value*16384:8192+(value-.5)*16382));
      msg.mStatus=0xe0;msg.mData1=raw&127;msg.mData2=raw>>7;
    } else {msg.mStatus=0xb0;msg.mData1=1;msg.mData2=static_cast<int>(std::round(value*127));}
    msg.mOffset=0;GetDelegate()->SendMidiMsgFromUI(msg);
  }
  bool pitch_;
};
}
