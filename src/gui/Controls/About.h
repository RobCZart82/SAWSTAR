// SPDX-License-Identifier: MIT
#pragma once
#include "gui/Controls/Theme.h"
#include "CreatorLogo.h"
#include "config.h"
namespace sawstar::gui {
// Full-editor overlay intercepts clicks and wheel events until OK is pressed.
class AboutWindow final:public IControl {
 IBitmap logo_;
 IRECT Box()const{return mRECT.GetCentredInside(480,580);}
 IRECT OK()const{auto b=Box();return IRECT(b.MW()-60,b.B-57,b.MW()+60,b.B-21);}
public:
 explicit AboutWindow(IRECT r):IControl(r){Hide(true);}
 void OnInit()override{logo_=GetUI()->LoadBitmap("GYR-Logo1.png",CreatorLogo,sizeof(CreatorLogo));}
 void Open(){Hide(false);GetUI()->SetAllControlsDirty();}
 void Close(){Hide(true);GetUI()->SetAllControlsDirty();}
 void Draw(IGraphics& g)override{
  g.FillRect(IColor(190,0,0,0),mRECT);const auto b=Box();
  g.FillRoundRect(PanelColor,b,8);g.DrawRoundRect(Border,b,8);
  g.DrawText(IText(27,Text),"SAWSTAR",b.GetFromTop(62));
  g.DrawText(IText(14,Text),"Simple Synth - Big Sound",IRECT(b.L,b.T+59,b.R,b.T+83));
  const IRECT plaque(b.MW()-145,b.T+100,b.MW()+145,b.T+402);
  g.FillRoundRect(IColor(255,245,245,242),plaque,5);
  g.DrawFittedBitmap(logo_,plaque.GetPadded(-10));
  g.DrawText(IText(18,Text),"Gyuricza Róbert",IRECT(b.L,b.T+418,b.R,b.T+447));
  g.DrawText(IText(13,Text),"Version " PLUG_VERSION_STR " / Development",IRECT(b.L,b.T+453,b.R,b.T+476));
  g.DrawText(IText(12,Blue),"github.com/RobCZart82/SAWSTAR",IRECT(b.L,b.T+481,b.R,b.T+503));
  g.FillRoundRect(IColor(255,22,58,80),OK(),3);g.DrawRoundRect(Blue,OK(),3);g.DrawText(IText(15,Text),"OK",OK());
 }
 void OnMouseDown(float x,float y,const IMouseMod&)override{if(OK().Contains(x,y))Close();}
 bool OnKeyDown(float,float,const IKeyPress& key)override{if(key.VK==27||key.VK==13)Close();return true;}
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
