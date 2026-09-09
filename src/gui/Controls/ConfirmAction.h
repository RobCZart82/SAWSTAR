// SPDX-License-Identifier: MIT
#pragma once
#include "gui/Controls/Theme.h"
#include <functional>
#include <string>
namespace sawstar::gui {
class ConfirmAction final:public IControl {
 std::string title_,message_,action_;std::function<void()> confirm_;bool actionFocused_=false;
 IRECT Box()const{return mRECT.GetCentredInside(570,230);}
 IRECT Cancel()const{auto b=Box();return IRECT(b.R-294,b.B-63,b.R-164,b.B-23);}
 IRECT Accept()const{auto b=Box();return IRECT(b.R-150,b.B-63,b.R-20,b.B-23);}
 void Finish(bool accept){auto callback=std::move(confirm_);confirm_={};Hide(true);GetUI()->SetAllControlsDirty();if(accept&&callback)callback();}
public:
 explicit ConfirmAction(IRECT r):IControl(r){Hide(true);}
 void Ask(std::string title,std::string message,std::string action,std::function<void()> confirm){title_=std::move(title);message_=std::move(message);action_=std::move(action);confirm_=std::move(confirm);actionFocused_=false;Hide(false);GetUI()->SetAllControlsDirty();}
 void Draw(IGraphics& g)override{g.FillRect(IColor(190,0,0,0),mRECT);auto b=Box();g.FillRoundRect(PanelColor,b,7);g.DrawRoundRect(Border,b,7);g.DrawText(IText(22,Text),title_.c_str(),b.GetFromTop(59));
  std::string text=message_;float y=b.T+66;while(!text.empty()){size_t n=text.size()<=62?text.size():text.rfind(' ',62);if(n==0||n==std::string::npos)n=std::min<size_t>(62,text.size());g.DrawText(IText(14,Text).WithAlign(EAlign::Near),text.substr(0,n).c_str(),IRECT(b.L+24,y,b.R-24,y+23));text.erase(0,n+(n<text.size()?1:0));y+=24;}
  g.FillRoundRect(PanelColor,Cancel(),3);g.DrawRoundRect(!actionFocused_?Blue:Border,Cancel(),3,nullptr,2);g.DrawText(IText(14,Text),"Cancel",Cancel());g.FillRoundRect(IColor(255,56,30,30),Accept(),3);g.DrawRoundRect(actionFocused_?Blue:IColor(255,150,90,80),Accept(),3,nullptr,2);g.DrawText(IText(14,Text),action_.c_str(),Accept());
 }
 void OnMouseDown(float x,float y,const IMouseMod&)override{if(Cancel().Contains(x,y))Finish(false);else if(Accept().Contains(x,y))Finish(true);}
 bool OnKeyDown(float,float,const iplug::IKeyPress& key)override{if(key.VK==27)Finish(false);else if(key.VK==9){actionFocused_=!actionFocused_;SetDirty(false);}else if(key.VK==13)Finish(actionFocused_);return true;}
};
}
