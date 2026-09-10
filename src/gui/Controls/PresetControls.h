// SPDX-License-Identifier: MIT
#pragma once
#include <string>
#include "IControl.h"
#include "IPopupMenuControl.h"
#include "presets/FactoryPresets.h"
#include "presets/UserPresets.h"
#include "gui/Controls/ConfirmAction.h"
#include <memory>
#include <functional>
namespace sawstar::gui {
class PresetSelector final : public iplug::igraphics::IControl {
 std::shared_ptr<int> lifetime_=std::make_shared<int>(0);ConfirmAction* confirm_;
 const int& selected_;std::function<void(int)> action_;UserPresetSelection& user_;std::function<Snapshot()> current_;std::function<void(const Snapshot&)> apply_;iplug::igraphics::IPopupMenu menu_;std::vector<fs::path> files_;std::vector<int> menuIndices_;std::function<void()> sync_;
 void Refresh(){files_=ListUserPresets(UserPresetFolder());menu_.Clear();menuIndices_.clear();for(int i=1;i<int(FactoryPresets().size());++i){menu_.AddItem(FactoryPresets()[i].name);menuIndices_.push_back(i);}for(int i=0;i<int(files_.size());++i){menu_.AddItem((files_[i].stem().u8string()+" [User]").c_str());menuIndices_.push_back(int(FactoryPresets().size())+i);}menu_.AddSeparator();menuIndices_.push_back(-1);menu_.AddItem("INIT PRESET");menuIndices_.push_back(0);}
 void Load(int i){if(i==0){confirm_->Ask("Initialize Preset?","Initialize the current sound? Any unsaved changes will be lost.","Initialize",[this,weak=std::weak_ptr<int>(lifetime_)]{if(weak.expired())return;action_(0);if(sync_)sync_();GetUI()->SetAllControlsDirty();});return;}if(i<int(FactoryPresets().size()))action_(i);else{auto path=files_.at(i-FactoryPresets().size());auto values=ReadUserPreset(path);apply_(values);user_.path=path;user_.name=path.stem().u8string();user_.saved=values;user_.active=true;}if(sync_)sync_();GetUI()->SetAllControlsDirty();}
 void Error(const std::exception& e){GetUI()->ShowMessageBox(e.what(),"SAWSTAR",iplug::igraphics::kMB_OK);}
public:
 PresetSelector(const iplug::igraphics::IRECT& r,const int& selected,std::function<void(int)> action,UserPresetSelection& user,std::function<Snapshot()> current,std::function<void(const Snapshot&)> apply,std::function<void()> sync,ConfirmAction* confirm):IControl(r),confirm_(confirm),selected_(selected),action_(action),user_(user),current_(current),apply_(apply),sync_(sync){}
 void Draw(iplug::igraphics::IGraphics& g)override{
  using namespace iplug::igraphics;IColor light(255,210,237,245);g.FillRoundRect(IColor(255,20,26,29),mRECT,5);g.DrawRoundRect(IColor(255,55,95,110),mRECT,5);
  g.DrawText(IText(18,light),"<",mRECT.GetFromLeft(28));g.DrawText(IText(18,light),">",mRECT.GetFromRight(28));
  auto name=user_.active?user_.path.stem().u8string()+(!SnapshotsMatch(current_(),user_.saved)?" *":""):selected_<0?std::string("Custom"):std::string(FactoryPresets()[selected_].name);g.DrawText(IText(14,light),name.c_str(),mRECT.GetHPadded(-30));
 }
 void OnMouseDown(float x,float,const iplug::igraphics::IMouseMod&)override{try{Refresh();int index=selected_;if(user_.active){index=-1;for(int i=0;i<int(files_.size());++i)if(files_[i]==user_.path){index=int(FactoryPresets().size())+i;break;}}int count=int(FactoryPresets().size()+files_.size());
  if(x<mRECT.L+28)Load(index<=1?count-1:index-1);else if(x>mRECT.R-28)Load(index<1||index==count-1?1:index+1);else {auto* popup=GetUI()->GetPopupMenuControl();if(popup)popup->SetMenuForcedSouth(true);GetUI()->CreatePopupMenu(*this,menu_,mRECT);}
 }catch(const std::exception& e){Error(e);}}
 void OnPopupMenuSelection(iplug::igraphics::IPopupMenu* menu,int)override{if(menu&&menu->GetChosenItemIdx()>=0)try{int i=menuIndices_.at(menu->GetChosenItemIdx());if(i>=0)Load(i);}catch(const std::exception& e){Error(e);}}
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
