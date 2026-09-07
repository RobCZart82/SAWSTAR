// SPDX-License-Identifier: MIT
#pragma once
#include "IControl.h"
#include "presets/FactoryPresets.h"
#include <functional>
namespace sawstar::gui {
class PresetSelector final : public iplug::igraphics::IControl {
public:
 PresetSelector(const iplug::igraphics::IRECT& r,const int& selected,std::function<void(int)> action)
 :IControl(r),selected_(selected),action_(std::move(action)){
  for(const auto& p:FactoryPresets())menu_.AddItem(p.name);
 }
 void Draw(iplug::igraphics::IGraphics& g)override{
  using namespace iplug::igraphics;IColor light(255,210,237,245);
  g.FillRoundRect(IColor(255,20,26,29),mRECT,5);g.DrawRoundRect(IColor(255,55,95,110),mRECT,5);
  g.DrawText(IText(18,light),"<",mRECT.GetFromLeft(28));g.DrawText(IText(18,light),">",mRECT.GetFromRight(28));
  g.DrawText(IText(14,light),selected_<0?"Custom":FactoryPresets()[selected_].name,mRECT.GetHPadded(-30));
 }
 void OnMouseDown(float x,float y,const iplug::igraphics::IMouseMod&)override{
  if(x<mRECT.L+28)action_(StepFactoryPreset(selected_,-1));
  else if(x>mRECT.R-28)action_(StepFactoryPreset(selected_,1));
  else GetUI()->CreatePopupMenu(*this,menu_,mRECT.L,mRECT.B);
 }
 void OnPopupMenuSelection(iplug::igraphics::IPopupMenu* menu,int)override{
  if(menu&&menu->GetChosenItemIdx()>=0&&menu->GetChosenItemIdx()<static_cast<int>(FactoryPresets().size()))action_(menu->GetChosenItemIdx());
 }
private:
 const int& selected_;std::function<void(int)> action_;iplug::igraphics::IPopupMenu menu_;
};
class PresetRow final : public iplug::igraphics::IControl {
public:
 PresetRow(const iplug::igraphics::IRECT& r,int index,const int& selected,std::function<void(int)> action)
 :IControl(r),index_(index),selected_(selected),action_(std::move(action)){}
 void Draw(iplug::igraphics::IGraphics& g)override{
  using namespace iplug::igraphics;const auto& p=FactoryPresets()[index_];
  g.FillRoundRect(index_==selected_?IColor(255,22,77,100):IColor(255,20,26,29),mRECT,4);
  auto text=IText(14,IColor(255,210,237,245)).WithAlign(EAlign::Near);
  g.DrawText(text,p.name,IRECT(mRECT.L+12,mRECT.T,mRECT.L+177,mRECT.B));
  g.DrawText(text.WithFGColor(IColor(255,54,170,226)),p.category,IRECT(mRECT.L+185,mRECT.T,mRECT.L+255,mRECT.B));
  g.DrawText(text.WithSize(13),p.lesson,IRECT(mRECT.L+265,mRECT.T,mRECT.R-10,mRECT.B));
 }
 void OnMouseDown(float,float,const iplug::igraphics::IMouseMod&)override{action_(index_);}
private:
 int index_;const int& selected_;std::function<void(int)> action_;
};
}
