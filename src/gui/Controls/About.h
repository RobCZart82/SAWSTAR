// SPDX-License-Identifier: MIT
#pragma once
#include "gui/Controls/Theme.h"
#include "AboutFont.h"
#include "gui/Controls/DeveloperMark.h"
#include "config.h"
#include "BuildVersion.h"
namespace sawstar::gui {
// Full-editor overlay intercepts clicks and wheel events until OK is pressed.
class AboutWindow final:public IControl {
 bool fontLoaded_=false;
 IRECT Box()const{return mRECT.GetCentredInside(620,460);}
 IRECT OK()const{auto b=Box();return IRECT(b.MW()-60,b.B-57,b.MW()+60,b.B-21);}
public:
 explicit AboutWindow(IRECT r,bool fontLoaded):IControl(r),fontLoaded_(fontLoaded){Hide(true);}
 void Open(){Hide(false);GetUI()->SetAllControlsDirty();}
 void Close(){Hide(true);GetUI()->SetAllControlsDirty();}
 void Draw(IGraphics& g)override{
  g.FillRect(IColor(190,0,0,0),mRECT);const auto b=Box();
  DrawPanel(g,b,8);
  g.DrawText(IText(54,Text).WithFont(fontLoaded_?"SAWSTAR-Orbitron":"Roboto-Regular"),"SAWSTAR",IRECT(b.L+25,b.T+30,b.R-25,b.T+112));
  g.DrawText(IText(16,Text),"Simple Synth - Big Sound",IRECT(b.L,b.T+114,b.R,b.T+143));
  g.DrawText(IText(13,Text),Motto,IRECT(b.L+20,b.T+146,b.R-20,b.T+171));
  g.DrawLine(Blue,b.L+85,b.T+184,b.R-85,b.T+184);
  DrawDeveloperMark(g,IRECT(b.L+140,b.T+202,b.L+183,b.T+271));
  g.DrawText(IText(13,Muted),"Developed by",IRECT(b.L,b.T+200,b.R,b.T+222));
  g.DrawText(IText(20,Text),"Gyuricza Róbert",IRECT(b.L,b.T+225,b.R,b.T+254));
  g.DrawText(IText(11,Text),"Version " SAWSTAR_DISPLAY_VERSION " / " SAWSTAR_BUILD_ID,IRECT(b.L,b.T+262,b.R,b.T+284));
  g.DrawText(IText(12,Blue),"github.com/RobCZart82/SAWSTAR",IRECT(b.L,b.T+291,b.R,b.T+316));
  g.DrawLine(Blue,b.L+85,b.T+321,b.R-85,b.T+321);
  g.DrawText(IText(12,Text),"iPlug2 / DaisySP   |   SAWSTAR code: MIT",IRECT(b.L,b.T+327,b.R,b.T+349));
  g.DrawText(IText(12,Text),"Orbitron: Matt McInerney / SIL OFL 1.1",IRECT(b.L,b.T+351,b.R,b.T+373));
  g.FillRoundRect(IColor(255,22,58,80),OK(),3);g.DrawRoundRect(Blue,OK(),3);g.DrawText(IText(15,Text),"OK",OK());
 }
 void OnMouseDown(float x,float y,const IMouseMod&)override{if(OK().Contains(x,y))Close();}
 bool OnKeyDown(float,float,const iplug::IKeyPress& key)override{if(key.VK==27||(key.VK&0x7fff)==13)Close();return true;}
};
class SettingsMenu final:public IControl {
 AboutWindow* about_;float& scale_;IPopupMenu menu_;
public:
 SettingsMenu(IRECT r,AboutWindow* about,float& scale):IControl(r),about_(about),scale_(scale){menu_.AddItem("GUI Scale: 75%");menu_.AddItem("GUI Scale: 100%");menu_.AddItem("GUI Scale: 125%");menu_.AddSeparator();menu_.AddItem("Updates / Downloads...");menu_.AddSeparator();menu_.AddItem("About SAWSTAR...");SetTooltip("GUI scale / Downloads / About");}
 void Draw(IGraphics& g)override{
  const float x=mRECT.MW(),y=mRECT.MH();
  for(int i=0;i<8;++i){float a=i*.785398163f;g.DrawLine(Blue,x+std::cos(a)*9,y+std::sin(a)*9,x+std::cos(a)*14,y+std::sin(a)*14,nullptr,4);}
  g.DrawCircle(Blue,x,y,9,nullptr,4);g.FillCircle(PanelColor,x,y,5);
 }
 void OnMouseDown(float,float,const IMouseMod&)override{for(int i=0;i<3;++i)menu_.GetItem(i)->SetChecked(std::abs(scale_-(.75f+i*.25f))<.01f);GetUI()->CreatePopupMenu(*this,menu_,mRECT);}
 void OnPopupMenuSelection(IPopupMenu* menu,int)override{if(!menu)return;int i=menu->GetChosenItemIdx();if(i>=0&&i<3){scale_=.75f+i*.25f;GetUI()->Resize(PLUG_WIDTH,PLUG_HEIGHT,scale_);}else if(i==4)GetUI()->OpenURL("https://github.com/RobCZart82/SAWSTAR/releases");else if(i==6)about_->Open();}
};
}
