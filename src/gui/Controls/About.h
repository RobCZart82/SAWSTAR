// SPDX-License-Identifier: MIT
#pragma once
#include "gui/Controls/Theme.h"
#include "AboutFont.h"
#include "config.h"
namespace sawstar::gui {
// Full-editor overlay intercepts clicks and wheel events until OK is pressed.
class AboutWindow final:public IControl {
 bool fontLoaded_=false;
 IRECT Box()const{return mRECT.GetCentredInside(580,410);}
 IRECT OK()const{auto b=Box();return IRECT(b.MW()-60,b.B-57,b.MW()+60,b.B-21);}
public:
 explicit AboutWindow(IRECT r,bool fontLoaded):IControl(r),fontLoaded_(fontLoaded){Hide(true);}
 void Open(){Hide(false);GetUI()->SetAllControlsDirty();}
 void Close(){Hide(true);GetUI()->SetAllControlsDirty();}
 void Draw(IGraphics& g)override{
  g.FillRect(IColor(190,0,0,0),mRECT);const auto b=Box();
  g.FillRoundRect(PanelColor,b,8);g.DrawRoundRect(Border,b,8);
  g.DrawText(IText(54,Text).WithFont(fontLoaded_?"SAWSTAR-Orbitron":"Roboto-Regular"),"SAWSTAR",IRECT(b.L+25,b.T+30,b.R-25,b.T+112));
  g.DrawText(IText(16,Text),"Simple Synth - Big Sound",IRECT(b.L,b.T+114,b.R,b.T+143));
  g.DrawLine(Blue,b.L+85,b.T+162,b.R-85,b.T+162);
  g.DrawText(IText(20,Text),"Gyuricza Róbert",IRECT(b.L,b.T+176,b.R,b.T+209));
  g.DrawText(IText(13,Text),"Version " PLUG_VERSION_STR " / Development",IRECT(b.L,b.T+216,b.R,b.T+242));
  g.DrawText(IText(12,Blue),"github.com/RobCZart82/SAWSTAR",IRECT(b.L,b.T+248,b.R,b.T+273));
  g.DrawText(IText(12,Text),"iPlug2 / DaisySP   |   SAWSTAR code: MIT",IRECT(b.L,b.T+280,b.R,b.T+302));
  g.DrawText(IText(12,Text),"Orbitron: Matt McInerney / SIL OFL 1.1",IRECT(b.L,b.T+304,b.R,b.T+326));
  g.FillRoundRect(IColor(255,22,58,80),OK(),3);g.DrawRoundRect(Blue,OK(),3);g.DrawText(IText(15,Text),"OK",OK());
 }
 void OnMouseDown(float x,float y,const IMouseMod&)override{if(OK().Contains(x,y))Close();}
 bool OnKeyDown(float,float,const iplug::IKeyPress& key)override{if(key.VK==27||key.VK==13)Close();return true;}
};
class SettingsMenu final:public IControl {
 AboutWindow* about_;IPopupMenu menu_;
public:
 SettingsMenu(IRECT r,AboutWindow* about):IControl(r),about_(about){menu_.AddItem("About SAWSTAR...");SetTooltip("Settings / About");}
 void Draw(IGraphics& g)override{
  const float x=mRECT.MW(),y=mRECT.MH();
  for(int i=0;i<8;++i){float a=i*.785398163f;g.DrawLine(Blue,x+std::cos(a)*9,y+std::sin(a)*9,x+std::cos(a)*14,y+std::sin(a)*14,nullptr,4);}
  g.DrawCircle(Blue,x,y,9,nullptr,4);g.FillCircle(PanelColor,x,y,5);
 }
 void OnMouseDown(float,float,const IMouseMod&)override{GetUI()->CreatePopupMenu(*this,menu_,mRECT.L,mRECT.B);}
 void OnPopupMenuSelection(IPopupMenu* menu,int)override{if(menu&&menu->GetChosenItemIdx()==0)about_->Open();}
};
}
