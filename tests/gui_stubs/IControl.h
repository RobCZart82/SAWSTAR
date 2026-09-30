// SPDX-License-Identifier: MIT
#pragma once
// Minimal graphics/delegate shell; tests compile the actual wheel control.
// Native rendering, hit-testing and platform window dispatch are not simulated.
namespace iplug {
struct IMidiMsg {int mStatus=0,mData1=0,mData2=0,mOffset=0;};
struct TestDelegate {virtual ~TestDelegate()=default;virtual void SendMidiMsgFromUI(const IMidiMsg&)=0;};
namespace igraphics {
struct IMouseMod {};
struct IColor {IColor(int,int,int,int){}};
struct IRECT {
 float L,T,R,B;
 IRECT(float l,float t,float r,float b):L(l),T(t),R(r),B(b){}
 float H()const{return B-T;}
 IRECT GetPadded(float p)const{return {L-p,T-p,R+p,B+p};}
};
struct IGraphics {
 void FillRect(IColor,IRECT){}
 void DrawRect(IColor,IRECT){}
 void DrawLine(IColor,float,float,float,float){}
};
class IControl {
 double value_=0;TestDelegate* delegate_=nullptr;
public:
 IRECT mRECT;
 explicit IControl(IRECT bounds):mRECT(bounds){}
 virtual ~IControl()=default;
 virtual void Draw(IGraphics&){}
 virtual void OnMouseDown(float,float,const IMouseMod&){}
 virtual void OnMouseDrag(float,float,float,float,const IMouseMod&){}
 virtual void OnMouseUp(float,float,const IMouseMod&){}
 virtual void OnMouseDblClick(float,float,const IMouseMod&){}
 virtual void OnTouchCancelled(float,float,const IMouseMod&){}
 virtual void OnMidi(const IMidiMsg&){}
 void SetValue(double v){value_=v;}
 double GetValue()const{return value_;}
 void SetWantsMidi(bool){}
 void SetDirty(bool){}
 TestDelegate* GetDelegate(){return delegate_;}
 void SetTestDelegate(TestDelegate* d){delegate_=d;}
};
}}
