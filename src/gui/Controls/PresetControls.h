// SPDX-License-Identifier: MIT
#pragma once
#include <string>
#include "IControl.h"
#include "presets/FactoryPresets.h"
#include "presets/UserPresets.h"
#include <functional>
namespace sawstar::gui {
class PresetSelector final : public iplug::igraphics::IControl {
 const int& selected_;std::function<void(int)> action_;UserPresetSelection& user_;std::function<Snapshot()> current_;std::function<void(const Snapshot&)> apply_;iplug::igraphics::IPopupMenu menu_;std::vector<fs::path> files_;std::function<void()> sync_;
 void Refresh(){files_=ListUserPresets(UserPresetFolder());menu_.Clear();for(const auto& p:FactoryPresets())menu_.AddItem(p.name);for(const auto& p:files_)menu_.AddItem((p.stem().u8string()+" [User]").c_str());}
 void Load(int i){if(i<int(FactoryPresets().size()))action_(i);else{auto path=files_.at(i-FactoryPresets().size());auto values=ReadUserPreset(path);apply_(values);user_.path=path;user_.name=path.stem().u8string();user_.saved=values;user_.active=true;}if(sync_)sync_();GetUI()->SetAllControlsDirty();}
 void Error(const std::exception& e){GetUI()->ShowMessageBox(e.what(),"SAWSTAR",iplug::igraphics::kMB_OK);}
public:
 PresetSelector(const iplug::igraphics::IRECT& r,const int& selected,std::function<void(int)> action,UserPresetSelection& user,std::function<Snapshot()> current,std::function<void(const Snapshot&)> apply,std::function<void()> sync):IControl(r),selected_(selected),action_(action),user_(user),current_(current),apply_(apply),sync_(sync){}
 void Draw(iplug::igraphics::IGraphics& g)override{
  using namespace iplug::igraphics;IColor light(255,210,237,245);g.FillRoundRect(IColor(255,20,26,29),mRECT,5);g.DrawRoundRect(IColor(255,55,95,110),mRECT,5);
  g.DrawText(IText(18,light),"<",mRECT.GetFromLeft(28));g.DrawText(IText(18,light),">",mRECT.GetFromRight(28));
  auto name=user_.active?user_.path.stem().u8string()+(current_()!=user_.saved?" *":""):selected_<0?std::string("Custom"):std::string(FactoryPresets()[selected_].name);g.DrawText(IText(14,light),name.c_str(),mRECT.GetHPadded(-30));
 }
 void OnMouseDown(float x,float,const iplug::igraphics::IMouseMod&)override{try{Refresh();int index=selected_;if(user_.active){index=-1;for(int i=0;i<int(files_.size());++i)if(files_[i]==user_.path){index=int(FactoryPresets().size())+i;break;}}int count=int(FactoryPresets().size()+files_.size());
  if(x<mRECT.L+28)Load(index<0?count-1:(index+count-1)%count);else if(x>mRECT.R-28)Load(index<0?0:(index+1)%count);else GetUI()->CreatePopupMenu(*this,menu_,mRECT.L,mRECT.B);
 }catch(const std::exception& e){Error(e);}}
 void OnPopupMenuSelection(iplug::igraphics::IPopupMenu* menu,int)override{if(menu&&menu->GetChosenItemIdx()>=0)try{Load(menu->GetChosenItemIdx());}catch(const std::exception& e){Error(e);}}
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
