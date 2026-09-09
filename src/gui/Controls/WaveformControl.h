// SPDX-License-Identifier: MIT
#pragma once
#include "IControl.h"
#include <algorithm>
#include <cmath>
namespace sawstar::gui {
// Schematic source shape, not an oscilloscope of the mixed/unison output.
class WaveformControl final:public iplug::igraphics::IControl {
public:
 WaveformControl(const iplug::igraphics::IRECT& r,int param,const char* title,bool sub=false):IControl(r,param),title_(title),sub_(sub){for(int i=0;i<(sub_?3:4);++i)menu_.AddItem(sub_?subNames_[i]:names_[i]);}
 void Draw(iplug::igraphics::IGraphics& g)override{
  using namespace iplug::igraphics;
  const int choice=std::clamp(static_cast<int>(std::lround(GetValue()*(sub_?2:3))),0,sub_?2:3);const int w=sub_?(choice==0?3:choice==1?2:1):choice;
  const IColor cyan(255,54,170,226),light(255,210,237,245);
  g.FillRoundRect(IColor(255,20,26,29),mRECT,4);g.DrawRoundRect(IColor(255,55,95,110),mRECT,4);
  g.DrawText(IText(12,light),title_,IRECT(mRECT.L,mRECT.T,mRECT.R,mRECT.T+18));
  g.DrawText(IText(12,cyan),(sub_?subNames_[choice]:names_[choice]),IRECT(mRECT.L,mRECT.B-20,mRECT.R,mRECT.B));
  const auto curve=IRECT(mRECT.L+12,mRECT.T+23,mRECT.R-12,mRECT.B-25);
  float px=curve.L,py=0;
  for(int i=0;i<=96;++i){float p=i/96.f,y=0;
   switch(w){case 0:y=1-2*p;break;case 1:y=p<.5f?1:-1;break;case 2:y=1-4*std::abs(p-.5f);break;default:y=std::sin(6.283185307179586f*p);}
   const float x=curve.L+p*curve.W(),screenY=curve.MH()-y*curve.H()*.5f;
   if(i)g.DrawLine(cyan,px,py,x,screenY,nullptr,1.5f);px=x;py=screenY;
  }
 }
 void OnMouseDown(float,float,const iplug::igraphics::IMouseMod&)override{GetUI()->CreatePopupMenu(*this,menu_,mRECT.L,mRECT.B);}
 void OnPopupMenuSelection(iplug::igraphics::IPopupMenu* menu,int)override{
  if(menu&&menu->GetChosenItemIdx()>=0){SetValue(menu->GetChosenItemIdx()/(sub_?2.:3.));SetDirty(true);}
 }
private:
 bool sub_;inline static constexpr const char* subNames_[3]={"Sine","Triangle","Square"};
 const char* title_;iplug::igraphics::IPopupMenu menu_;
 inline static constexpr const char* names_[4]={"Saw","Square","Triangle","Sine"};
};
}
