// SPDX-License-Identifier: MIT
#pragma once
#include "IControl.h"
#include <algorithm>
#include <cmath>
namespace sawstar::gui {
class NoiseSelector final:public iplug::igraphics::IControl {
public:
 explicit NoiseSelector(const iplug::igraphics::IRECT& r):IControl(r,{62,26}) {
   for(auto name:names_)menu_.AddItem(name);
 }
 void Draw(iplug::igraphics::IGraphics& g)override {
   using namespace iplug::igraphics;
   const int source=std::clamp(int(std::lround(GetValue(0)*3)),0,3);
   const int choice=source?source-1:std::clamp(int(std::lround(GetValue(1))),0,1);
   g.FillRoundRect(IColor(255,20,26,29),mRECT,4);
   g.DrawRoundRect(IColor(255,55,95,110),mRECT,4);
   g.DrawText(IText(14,IColor(255,54,170,226)),names_[choice],mRECT.GetPadded(-4));
 }
 void OnMouseDown(float,float,const iplug::igraphics::IMouseMod&)override {
   GetUI()->CreatePopupMenu(*this,menu_,mRECT.L,mRECT.B);
 }
 void OnPopupMenuSelection(iplug::igraphics::IPopupMenu* menu,int)override {
   if(menu&&menu->GetChosenItemIdx()>=0){SetValue((menu->GetChosenItemIdx()+1)/3.,0);SetDirty(true,0);}
 }
private:
 iplug::igraphics::IPopupMenu menu_;
 inline static constexpr const char* names_[3]={"White Noise","Dark Noise","Pink Noise"};
};
}
