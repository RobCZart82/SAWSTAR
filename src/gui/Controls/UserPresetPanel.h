// SPDX-License-Identifier: MIT
#pragma once
#include "gui/Controls/Theme.h"
#include "presets/UserPresets.h"
#include <functional>
#include <memory>
namespace sawstar::gui {
class UserPresetPanel final:public IControl{
 std::shared_ptr<int> lifetime_=std::make_shared<int>(0);
 UserPresetSelection& state_;std::function<Snapshot()> current_;std::function<void(const Snapshot&)> apply_;std::function<void()> init_;IPopupMenu menu_;std::vector<fs::path> files_;WDL_String file_,folder_;
 IRECT NameRect()const{return IRECT(mRECT.L,mRECT.T+20,mRECT.R,mRECT.T+52);}
 void Error(const std::exception& e){state_.status=e.what();SetDirty(false);}
 void Load(const fs::path& p){auto values=ReadUserPreset(p);apply_(values);state_.saved=values;state_.path=p;state_.name=p.stem().u8string();state_.active=true;state_.status="Loaded user preset";GetUI()->SetAllControlsDirty();}
public:
 UserPresetPanel(IRECT r,UserPresetSelection& s,std::function<Snapshot()> current,std::function<void(const Snapshot&)> apply,std::function<void()> init):IControl(r),state_(s),current_(current),apply_(apply),init_(init){}
 void Draw(IGraphics& g)override{
 g.DrawText(IText(12,Blue).WithAlign(EAlign::Near),"USER PRESETS — click name to edit",mRECT.GetFromTop(19));g.FillRect(IColor(255,9,15,18),NameRect());g.DrawRect(Border,NameRect());g.DrawText(IText(14,Text).WithAlign(EAlign::Near),state_.name.c_str(),NameRect().GetHPadded(-8));
 g.DrawText(IText(11,Text).WithAlign(EAlign::Near),state_.status.c_str(),IRECT(mRECT.L,mRECT.T+54,mRECT.R,mRECT.T+78));
 const char* labels[]={"Save As…","Load…","User Library ▾","Rename","Delete","Init"};for(int i=0;i<6;++i){auto r=IRECT(mRECT.L+i*mRECT.W()/6,mRECT.B-32,mRECT.L+(i+1)*mRECT.W()/6-4,mRECT.B);g.FillRoundRect(PanelColor,r,3);g.DrawRoundRect(Border,r,3);g.DrawText(IText(11,Text),labels[i],r);}
 }
 void OnTextEntryCompletion(const char* str,int)override{state_.name=str;SetDirty(false);}
 void OnPopupMenuSelection(IPopupMenu* m,int)override{if(!m)return;int i=m->GetChosenItemIdx();if(i>=0&&i<int(files_.size()))try{Load(files_[i]);}catch(const std::exception& e){Error(e);}}
 void OnMouseDown(float x,float y,const IMouseMod&)override{
 if(NameRect().Contains(x,y)){GetUI()->CreateTextEntry(*this,IText(14,Text),NameRect(),state_.name.c_str());return;}if(y<mRECT.B-32)return;
 int action=int((x-mRECT.L)/(mRECT.W()/6));try{
 if(action==0){auto root=UserPresetFolder();if(root.empty())throw std::runtime_error("User preset folder is unavailable.");fs::create_directories(root);folder_.Set(root.u8string().c_str());file_.Set((ValidPresetName(state_.name)?state_.name+".sawstar":"My Sound.sawstar").c_str());const auto values=current_();
 GetUI()->PromptForFile(file_,folder_,EFileAction::Save,"sawstar",[this,weak=std::weak_ptr<int>(lifetime_),values](const WDL_String& f,const WDL_String&){if(weak.expired()||!f.GetLength())return;try{auto p=fs::u8path(f.Get());if(p.extension().empty())p+=".sawstar";if(p.extension()!=".sawstar"||!ValidPresetName(p.stem().u8string()))throw std::runtime_error("Use a valid name and the .sawstar extension.");SaveUserPreset(p,values);state_.path=p;state_.name=p.stem().u8string();state_.saved=values;state_.active=true;state_.status="Saved user preset";GetUI()->SetAllControlsDirty();}catch(const std::exception& e){Error(e);}});}
 else if(action==1){folder_.Set(UserPresetFolder().u8string().c_str());file_.Set("");GetUI()->PromptForFile(file_,folder_,EFileAction::Open,"sawstar",[this,weak=std::weak_ptr<int>(lifetime_)](const WDL_String& f,const WDL_String&){if(weak.expired())return;if(f.GetLength())try{Load(fs::u8path(f.Get()));}catch(const std::exception& e){Error(e);}});}
 else if(action==2){files_=ListUserPresets(UserPresetFolder());menu_.Clear();for(const auto& p:files_)menu_.AddItem(p.stem().u8string().c_str());if(files_.empty())state_.status="No user presets yet. Enter a name and Save As.";else GetUI()->CreatePopupMenu(*this,menu_,mRECT.L,mRECT.B);}
 else if(action==3){if(!state_.active)throw std::runtime_error("Load or save a User preset first. Factory presets are read-only.");state_.path=RenameUserPreset(state_.path,state_.name);state_.status="Renamed user preset";}
 else if(action==4){if(!state_.active)throw std::runtime_error("Select a User preset first. Factory presets are read-only.");GetUI()->ShowMessageBox("Remove this user preset from the library? A .deleted backup will be kept beside it.","SAWSTAR",kMB_YESNO,[this,weak=std::weak_ptr<int>(lifetime_)](EMsgBoxResult r){if(weak.expired())return;if(r==kYES)try{ArchiveUserPreset(state_.path);state_.active=false;state_.status="Removed from library; backup retained.";GetUI()->SetAllControlsDirty();}catch(const std::exception& e){Error(e);}});}
 else if(action==5){init_();state_.active=false;state_.status="Init loaded";}
 GetUI()->SetAllControlsDirty();
 }catch(const std::exception& e){Error(e);}
 }
};
}
