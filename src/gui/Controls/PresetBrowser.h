// SPDX-License-Identifier: MIT
#pragma once
#include "gui/Controls/Theme.h"
#include "gui/Controls/ConfirmAction.h"
#include "presets/Library.h"
#include "gui/NativeImport.h"
#include <functional>
#include <memory>
#include <optional>
namespace sawstar::gui {
class PresetBrowser final:public IControl {
 std::shared_ptr<int> lifetime_=std::make_shared<int>(0);
 ConfirmAction* confirm_;PresetLibrary library_;UserPresetSelection& user_;std::function<Snapshot()> current_;std::function<void(const Snapshot&)> apply_;std::function<void(int)> factory_;
 std::vector<int> visible_;std::string category_="All",query_,selected_,status_;int scroll_=0,editing_=0;bool live_=false;
 std::optional<Snapshot> preview_,clipboard_;WDL_String file_,folder_;
 static constexpr int Rows=12;
 IRECT Search()const{return IRECT(331,94,616,124);}
 IRECT Row(int i)const{return IRECT(206,138+i*35,616,171+i*35);}
 IRECT Action(int i)const{if(i==2)return IRECT(1091,529,1253,569);if(i==6)return IRECT(1091,581,1253,621);int row=i==0?0:i==7?1:i==1?2:i==3?3:i==4?4:5;return IRECT(1091,141+row*49,1253,181+row*49);}
 const LibraryEntry* Selected()const{for(const auto& e:library_.entries)if(e.key==selected_)return &e;return nullptr;}
 void Error(const std::exception& e){status_=e.what();SetDirty(false);}
 void Filter(){visible_=library_.Filter(category_,query_);scroll_=std::clamp(scroll_,0,std::max(0,int(visible_.size())-Rows));}
 void Select(const std::string& key){selected_=key;preview_.reset();if(const auto* e=Selected())try{preview_=e->Read();status_.clear();}catch(const std::exception& err){status_=err.what();}SetDirty(false);}
 void Refresh(){library_.Refresh();Filter();if(!Selected()&&!visible_.empty())selected_=library_.entries[visible_.front()].key;Select(selected_);if(!library_.warning.empty())status_=library_.warning;}
 void LoadSelected(){const auto* e=Selected();if(!e)throw std::runtime_error("Select a preset first.");if(e->factory==0){Do(2);return;}auto values=e->Read();if(e->factory>=0)factory_(e->factory);else{apply_(values);user_.path=e->path;user_.name=e->name;user_.saved=values;user_.active=true;}status_="Loaded "+e->name;GetUI()->SetAllControlsDirty();}
 void SaveAs(){auto root=UserPresetFolder();if(root.empty())throw std::runtime_error("User preset folder unavailable.");fs::create_directories(root);folder_.Set(root.u8string().c_str());file_.Set((ValidPresetName(user_.name)?user_.name+".sawstar":"My Sound.sawstar").c_str());auto values=current_();
  GetUI()->PromptForFile(file_,folder_,EFileAction::Save,"sawstar",[this,weak=std::weak_ptr<int>(lifetime_),values](const WDL_String& f,const WDL_String&){if(weak.expired()||!f.GetLength())return;try{auto p=fs::u8path(f.Get());if(p.extension().empty())p+=".sawstar";if(p.extension()!=".sawstar"||!ValidPresetName(p.stem().u8string()))throw std::runtime_error("Use a valid name and .sawstar extension.");SaveUserPreset(p,values);
   // Keep a managed library copy when Save As exports to another folder.
   const auto root=UserPresetFolder();if(p.parent_path()!=root){auto report=ImportPresets({p},root,true);if(report.imported==0){status_="Saved externally; library copy skipped (name exists or import failed).";GetUI()->SetAllControlsDirty();return;}p=root/p.filename();}
   user_.path=p;user_.name=p.stem().u8string();user_.saved=values;user_.active=true;selected_="user:"+p.filename().u8string();category_="User";query_.clear();Refresh();status_="Saved "+user_.name;GetUI()->SetAllControlsDirty();
  }catch(const std::exception& e){Error(e);}});
 }
 void Do(int action){try{
  if(action==0)LoadSelected();
  else if(action==1)SaveAs();
  else if(action==2){confirm_->Ask("Initialize Preset?","Initialize the current sound? Any unsaved changes will be lost.","Initialize",[this,weak=std::weak_ptr<int>(lifetime_)]{if(weak.expired())return;factory_(0);Select("factory:init");status_="Init loaded";GetUI()->SetAllControlsDirty();});}
  else if(action==3){clipboard_=current_();status_="Current sound copied.";}
  else if(action==4){if(!clipboard_)throw std::runtime_error("Copy a sound first.");apply_(*clipboard_);user_.active=false;status_="Copied sound applied. Save As to keep it.";GetUI()->SetAllControlsDirty();}
  else if(action==5){auto* e=Selected();if(!e||e->factory>=0)throw std::runtime_error("Select a User preset to rename.");editing_=1;GetUI()->CreateTextEntry(*this,IText(14,Text),Action(5),e->name.c_str());}
  else if(action==6){const auto* e=Selected();if(!e||e->factory>=0)throw std::runtime_error("Factory presets are read-only.");auto path=e->path;
   confirm_->Ask("Delete Preset?","Delete this user preset? A recovery backup will be kept.","Delete",[this,path,weak=std::weak_ptr<int>(lifetime_)]{if(weak.expired())return;try{ArchiveUserPreset(path);if(user_.path==path)user_.active=false;Refresh();status_="Removed; backup retained.";GetUI()->SetAllControlsDirty();}catch(const std::exception& err){Error(err);}});
  }else if(action==7){auto weak=std::weak_ptr<int>(lifetime_);auto files=SelectPresetFiles(GetUI()->GetWindow());if(weak.expired()||files.empty())return;auto result=ImportPresets(files,UserPresetFolder());category_="User";query_.clear();scroll_=0;Refresh();status_=result.Summary();std::string report=status_;for(size_t i=0;i<std::min<size_t>(result.details.size(),12);++i)report+="\n"+result.details[i];if(result.details.size()>12)report+="\nAdditional files skipped or failed: "+std::to_string(result.details.size()-12);GetUI()->ShowMessageBox(report.c_str(),"SAWSTAR Import",kMB_OK);if(!result.duplicates.empty()){auto duplicates=result.duplicates;confirm_->Ask("Import identical settings?",std::to_string(duplicates.size())+" presets match sounds already in your library. Import these copies under their different names?","Import copies",[this,weak,duplicates]{if(weak.expired())return;try{auto retry=ImportPresets(duplicates,UserPresetFolder(),true);Refresh();status_=retry.Summary();GetUI()->SetAllControlsDirty();}catch(const std::exception& e){Error(e);}});}}
  SetDirty(false);
 }catch(const std::exception& e){Error(e);}}
 void TextLine(IGraphics& g,const std::string& s,IRECT r,int size=13,IColor color=Text){g.DrawText(IText(size,color).WithAlign(EAlign::Near),s.c_str(),r);}
 void Info(IGraphics& g){const auto* e=Selected();if(!e)return;TextLine(g,live_?"Current sound":e->name,IRECT(651,140,1038,177),22);
  std::string lesson=live_?"Live settings of the current sound. Save As stores these parameters. Diagrams summarize the sound; they are not audio measurements.":e->lesson;float y=184;while(!lesson.empty()&&y<277){size_t n=lesson.size()<=43?lesson.size():lesson.rfind(' ',43);if(n==0||n==std::string::npos)n=std::min<size_t>(43,lesson.size());TextLine(g,lesson.substr(0,n),IRECT(651,y,1060,y+20),13);lesson.erase(0,n+(n<lesson.size()?1:0));y+=21;}
  TextLine(g,"TAGS",IRECT(651,290,1057,312),12,Blue);TextLine(g,live_?"Live parameter view":e->category+(e->factory>=0?"  /  Factory":"  /  User"),IRECT(651,318,1057,343),14);
  TextLine(g,"HOW IT WORKS - schematic",IRECT(651,354,1057,375),12,Blue);
  for(int i=0;i<2;++i){IRECT b(651+i*203,377,848+i*203,397);g.FillRoundRect(live_==bool(i)?IColor(255,25,65,88):PanelColor,b,2);g.DrawText(IText(11,Text),i?"Current sound":"Saved settings",b);}

  if(!live_&&!preview_){TextLine(g,"Cannot read this preset.",IRECT(651,405,1057,432));return;}const auto v=live_?current_():*preview_;const char* blocks[]={"OSC","MIXER","FILTER","ENV","FX"};
  for(int i=0;i<5;++i){IRECT r(651+i*82,399,723+i*82,450);g.FillRoundRect(IColor(255,10,18,24),r,3);g.DrawRoundRect(Blue,r,3);g.DrawText(IText(11,Text),blocks[i],r.GetFromTop(19));auto plot=IRECT(r.L+7,r.T+23,r.R-7,r.B-6);
   if(i==0||i==2){float px=plot.L,py=plot.MH();for(int j=0;j<=30;++j){float t=j/30.f,y=0;if(i==0){int wave=int(std::lround(v[33]));y=wave==0?2*t-1:wave==1?(t<.5f?1.f:-1.f):wave==2?1-4*std::abs(t-.5f):std::sin(t*6.2831853f);}else{float cutoff=float(Normalize(kParameters[8],v[8]));float low=1/(1+std::exp((t-cutoff)*14));int mode=int(v[32]);y=2*(mode<2?low:mode==2?1-low:4*low*(1-low))-1;}float x=plot.L+t*plot.W(),sy=plot.MH()-y*plot.H()*.45f;if(j)g.DrawLine(Blue,px,py,x,sy);px=x;py=sy;}}
   else if(i==1){for(int j=0;j<4;++j){float x=plot.L+j*plot.W()/4;g.FillRect(Blue,IRECT(x,plot.B-float(v[20+j]/100)*plot.H(),x+6,plot.B));}}
   else if(i==3){float total=float(v[1]+v[2]+v[4])+250.f;float a=float(v[1])/total,d=float(v[2])/total,release=float(v[4])/total;float xs[]={0,a,a+d,1-release,1},ys[]={0,1,float(v[3]),float(v[3]),0};for(int j=1;j<5;++j)g.DrawLine(Blue,plot.L+xs[j-1]*plot.W(),plot.B-ys[j-1]*plot.H(),plot.L+xs[j]*plot.W(),plot.B-ys[j]*plot.H());}
   else{for(int j=0;j<3;++j){int id=j==0?42:j==1?46:54;g.FillCircle(v[id]>.5&&v[id+1]>0?Blue:Border,plot.L+10+j*19,plot.MH(),5);}}
  }
  const char* wave[]={"Saw","Square","Triangle","Sine"};auto w=[&](int id){return wave[std::clamp(int(std::lround(v[id])),0,3)];};
  TextLine(g,std::string("OSC1: ")+w(33)+"  |  OSC2: "+w(34),IRECT(651,463,1063,487),12);
  char line[150];std::snprintf(line,sizeof(line),"MIX: %.0f / %.0f / %.0f / %.0f",v[20],v[21],v[22],v[23]);TextLine(g,line,IRECT(651,490,1063,514),12);
  const char* filter[]={"Low Pass 12","Low Pass 24","High Pass 12","Band Pass 12"};std::snprintf(line,sizeof(line),"FILTER: %s / %.0f Hz",filter[std::clamp(int(std::lround(v[32])),0,3)],v[8]);TextLine(g,line,IRECT(651,517,1063,541),12);
  std::snprintf(line,sizeof(line),"AMP: A %.0f / D %.0f / S %.0f%% / R %.0f ms",v[1],v[2],v[3]*100,v[4]);TextLine(g,line,IRECT(651,544,1063,568),12);
  std::string fx="FX: ";if(v[42]>.5&&v[43]>0)fx+="Chorus ";if(v[46]>.5&&v[47]>0)fx+="Delay ";if(v[54]>.5&&v[55]>0)fx+="Reverb ";if(fx=="FX: ")fx+="Dry";TextLine(g,fx,IRECT(651,571,1063,595),12);
 }
public:
 PresetBrowser(UserPresetSelection& user,std::function<Snapshot()> current,std::function<void(const Snapshot&)> apply,std::function<void(int)> factory,ConfirmAction* confirm):IControl(IRECT(12,82,1268,636)),confirm_(confirm),library_(UserPresetFolder()),user_(user),current_(current),apply_(apply),factory_(factory){if(user_.active)selected_="user:"+user_.path.filename().u8string();else{int i=MatchFactoryPreset(current_());if(i>=0)selected_="factory:"+std::string(FactoryPresets()[i].key);}Refresh();}
 void SyncToSound(){try{library_.Refresh();Filter();if(user_.active)Select("user:"+user_.path.filename().u8string());else{int i=MatchFactoryPreset(current_());if(i>=0)Select("factory:"+std::string(FactoryPresets()[i].key));}}catch(const std::exception& e){Error(e);}}
 void Draw(IGraphics& g)override{
  const IRECT boxes[]={IRECT(12,82,192,636),IRECT(200,82,630,636),IRECT(638,82,1075,636),IRECT(1083,82,1268,636)};const char* names[]={"CATEGORIES","PRESETS","PRESET INFO","PRESET ACTIONS"};for(int i=0;i<4;++i){g.FillRoundRect(PanelColor,boxes[i],3);g.DrawRoundRect(Border,boxes[i],3);TextLine(g,names[i],boxes[i].GetPadded(-12).GetFromTop(24),14,Blue);}
  const char* labels[]={"All","Favorites","Init","Leads","Pads","Plucks","Bass","Sub Pads","Arps","Keys","Sequences","FX","User"};const char* keys[]={"All","Favorites","Init","Lead","Pad","Pluck","Bass","Sub Pad","Arp","Keys","Sequence","FX","User"};
  for(int i=0;i<13;++i){IRECT r(24,137+i*34,180,168+i*34);if(category_==keys[i])g.FillRoundRect(IColor(255,25,65,88),r,3);TextLine(g,labels[i],r.GetHPadded(-8),14);}
  g.FillRoundRect(IColor(255,9,15,18),Search(),3);g.DrawRoundRect(Border,Search(),3);TextLine(g,query_.empty()?"Search presets...":query_,Search().GetHPadded(-8),13);
  for(int i=0;i<Rows&&scroll_+i<int(visible_.size());++i){auto& e=library_.entries[visible_[scroll_+i]];auto r=Row(i);if(e.key==selected_)g.FillRoundRect(IColor(255,25,65,88),r,2);g.DrawText(IText(17,library_.Favorite(e.key)?Blue:Text),library_.Favorite(e.key)?"♥":"♡",r.GetFromLeft(32));TextLine(g,e.name,IRECT(r.L+38,r.T,r.R-85,r.B),14);g.FillCircle(e.factory>=0?IColor(255,190,91,91):IColor(255,87,181,119),r.R-73,r.MH(),3);TextLine(g,e.factory>=0?"Factory":"User",r.GetFromRight(63),11,Text);}
  TextLine(g,std::to_string(visible_.size())+" presets  |  "+std::to_string(library_.entries.size())+" total",IRECT(213,571,615,593),12);
  g.DrawText(IText(12,Text),"< Previous",IRECT(213,600,403,625));g.DrawText(IText(12,Text),"Next >",IRECT(413,600,615,625));
  Info(g);const char* actions[]={"Load","Save As...","Initialize","Copy","Paste","Rename","Delete","Import..."};for(int i=0;i<8;++i){auto r=Action(i);bool disabled=(i==4&&!clipboard_)||(i==6&&(!Selected()||Selected()->factory>=0));g.FillRoundRect(i==2||i==6?IColor(255,38,25,26):IColor(255,10,17,21),r,3);g.DrawRoundRect(disabled?Border:i==2||i==6?IColor(255,125,77,71):Border,r,3);g.DrawText(IText(14,disabled?Border:Text),actions[i],r);}
  g.DrawLine(Border,1091,510,1253,510);
  TextLine(g,status_,IRECT(650,606,1064,630),11);
 }
 void OnMouseDown(float x,float y,const IMouseMod&)override{try{
  if(IRECT(651,377,1051,397).Contains(x,y)){live_=x>=854;SetDirty(false);return;}
  if(Search().Contains(x,y)){editing_=0;GetUI()->CreateTextEntry(*this,IText(13,Text),Search(),query_.c_str());return;}
  if(x<192&&y>=137&&y<137+13*34){const char* keys[]={"All","Favorites","Init","Lead","Pad","Pluck","Bass","Sub Pad","Arp","Keys","Sequence","FX","User"};category_=keys[int((y-137)/34)];scroll_=0;Filter();SetDirty(false);return;}
  if(x>=206&&x<=616&&y>=138&&y<558){int i=int((y-138)/35)+scroll_;if(i<int(visible_.size())){auto e=library_.entries[visible_[i]];if(x<238){library_.ToggleFavorite(e.key);Filter();}else Select(e.key);}SetDirty(false);return;}
  if(x>=206&&x<=616&&y>=600){scroll_+=x<408?-Rows:Rows;Filter();SetDirty(false);return;}
  for(int i=0;i<8;++i)if(Action(i).Contains(x,y)){if(i==6&&(!Selected()||Selected()->factory>=0))return;Do(i);return;}
 }catch(const std::exception& e){Error(e);}}
 void OnMouseWheel(float x,float,const IMouseMod&,float delta)override{if(x>=200&&x<=630){scroll_+=delta>0?-3:3;Filter();SetDirty(false);}}
 void OnTextEntryCompletion(const char* value,int)override{try{if(editing_==0){query_=value;scroll_=0;Filter();}else{auto* e=Selected();if(!e||e->factory>=0)return;auto old=*e;auto path=RenameUserPreset(old.path,value);auto key="user:"+path.filename().u8string();if(user_.path==old.path){user_.path=path;user_.name=path.stem().u8string();}selected_=key;try{library_.MoveFavorite(old.key,key);}catch(const std::exception& err){status_=err.what();}Refresh();status_="Renamed user preset.";GetUI()->SetAllControlsDirty();}SetDirty(false);}catch(const std::exception& e){Error(e);}}
};
}
