// SPDX-License-Identifier: MIT
#pragma once
#include "IControls.h"
#include "IPopupMenuControl.h"
#include "ITextEntryControl.h"
#include <cmath>
#include <atomic>
#include <cstdio>
#include <string>
namespace sawstar::gui {
using namespace iplug::igraphics;
inline const IColor Text(255,225,234,238),Blue(255,54,170,226),PanelColor(255,27,34,38),Border(255,66,82,90);
inline const IColor DisplayColor(255,12,21,26),Muted(255,149,169,180);
inline constexpr const char* Motto="TRANCE  •  HOUSE  •  AMBIENT  •  BEYOND";
inline void DrawPanel(IGraphics& g,const IRECT& r,float radius=3){
 g.PathRoundRect(r,radius);g.PathFill(IPattern::CreateLinearGradient(r.L,r.T,r.R,r.B,{{IColor(255,32,41,46),0.f},{PanelColor,.45f},{IColor(255,23,30,34),1.f}}));
 g.DrawRoundRect(Border,r,radius);
}
inline void DrawGrid(IGraphics& g,const IRECT& r){
 for(int i=1;i<6;++i){float x=r.L+r.W()*i/6;g.DrawLine(IColor(255,25,41,48),x,r.T,x,r.B);}
 for(int i=1;i<4;++i){float y=r.T+r.H()*i/4;g.DrawLine(IColor(255,25,41,48),r.L,y,r.R,y);}
}
inline IText EntryStyle(int size){return IText(size,Text).WithTEColors(IColor(255,12,19,24),Text);}
inline void StyleEntry(IGraphics* g){if(auto* entry=g->GetTextEntryControl())if(entry->EditInProgress()){auto style=entry->GetText();style.mFGColor=Text;style.mTextEntryBGColor=IColor(255,12,19,24);style.mTextEntryFGColor=Blue;entry->SetText(style);}}
inline void ConfigurePopups(IGraphics* g){g->AttachPopupMenuControl(IText(14,Text));auto* popup=g->GetPopupMenuControl();popup->SetPanelColor(IColor(255,16,23,29));popup->SetCellBackgroundColor(IColor(255,16,23,29));popup->SetItemColor(Text);popup->SetItemMouseoverColor(IColor(255,25,65,88));popup->SetDisabledItemColor(IColor(255,116,135,145));popup->SetSeparatorColor(Border);popup->SetMenuForcedSouth(true);}
// Vector heart avoids missing Unicode glyphs in platform fonts.
inline void DrawFavorite(IGraphics& g,const IRECT& r,bool selected){
 const float x=r.MW(),y=r.MH();g.PathClear();g.PathMoveTo(x,y+6);
 g.PathCubicBezierTo(x-15,y-3,x-5,y-12,x,y-5);
 g.PathCubicBezierTo(x+5,y-12,x+15,y-3,x,y+6);g.PathClose();
 if(selected)g.PathFill(Blue);else g.PathStroke(Text,1.2f);
}
inline IVStyle Style(){return DEFAULT_STYLE.WithColor(kBG,PanelColor).WithColor(kFG,IColor(255,36,45,50)).WithColor(kFR,Border).WithColor(kHL,Blue).WithColor(kX1,Blue).WithColor(kX2,Text).WithColor(kX3,Blue).WithDrawShadows(false).WithRoundness(.12f).WithLabelText(IText(11,Text)).WithValueText(EntryStyle(10).WithVAlign(EVAlign::Bottom));}
// A recessed value field and chevron identify choices, not on/off actions.
inline void DrawChoice(IGraphics& g,const IRECT& r,const char* value,int size=11){
 g.FillRoundRect(DisplayColor,r,3);g.DrawRoundRect(Border,r,3);
 g.DrawText(IText(size,Text).WithAlign(EAlign::Near),value,IRECT(r.L+9,r.T,r.R-25,r.B));
 const float x=r.R-13,y=r.MH();g.DrawLine(Blue,x-4,y-2,x,y+2,nullptr,1.5f);g.DrawLine(Blue,x,y+2,x+4,y-2,nullptr,1.5f);
}
class Dropdown final:public IControl{
 const char* label_;
 IRECT Field()const{const auto area=*label_?mRECT.GetReducedFromTop(17):mRECT;return area.GetCentredInside(area.W(),std::min(32.f,area.H()));}
public:
 Dropdown(IRECT r,int id,const char* label):IControl(r,id),label_(label){mDisablePrompt=false;}
 void Draw(IGraphics& g)override{if(*label_)g.DrawText(IText(11,Text).WithAlign(EAlign::Near),label_,mRECT.GetFromTop(15));WDL_String display;GetParam()->GetDisplay(display);DrawChoice(g,Field(),display.Get());if(GetMouseIsOver())g.DrawRoundRect(Blue,Field(),3);}
 void OnMouseDown(float,float,const IMouseMod&)override{PromptUserInput(Field());StyleEntry(GetUI());}
};
class BrandWordmark final:public IControl{
 bool font_;
public:BrandWordmark(IRECT r,bool font):IControl(r),font_(font){SetIgnoreMouse(true);}
 void Draw(IGraphics& g)override{auto style=IText(38,Text).WithFont(font_?"SAWSTAR-Orbitron":"Roboto-Regular").WithAlign(EAlign::Near);float x=mRECT.L;for(char c:std::string("SAWSTAR")){char letter[]={c,0};IRECT measured;g.MeasureText(style,letter,measured);g.DrawText(style,letter,IRECT(x,mRECT.T,x+measured.W()+2,mRECT.B));x+=measured.W()+1.6f;}}
};
class Section final:public IControl{
 const char* title_;IColor color_;bool tint_;
public: Section(IRECT r,const char* title,IColor color=Blue,bool tint=false):IControl(r),title_(title),color_(color),tint_(tint){SetIgnoreMouse(true);}
 void Draw(IGraphics& g)override{DrawPanel(g,mRECT);g.DrawRoundRect(tint_?IColor(135,color_.R,color_.G,color_.B):Border,mRECT,3);
 if(tint_)g.FillRect(IColor(30,color_.R,color_.G,color_.B),mRECT.GetFromTop(35));
 if(*title_)g.DrawLine(tint_?IColor(85,color_.R,color_.G,color_.B):Border,mRECT.L+8,mRECT.T+35,mRECT.R-8,mRECT.T+35);
 g.DrawText(IText(15,color_).WithFont("SAWSTAR-Bold").WithAlign(EAlign::Near),title_,IRECT(mRECT.L+12,mRECT.T,mRECT.R-8,mRECT.T+35));}
};
// Decorative separators never capture input.
class Divider final:public IControl{
public:explicit Divider(IRECT r):IControl(r){SetIgnoreMouse(true);}
 void Draw(IGraphics& g)override{g.DrawLine(Border,mRECT.L,mRECT.MH(),mRECT.R,mRECT.MH());}
};
class Knob final:public IVKnobControl{
public:Knob(IRECT r,int id,const char* title):IVKnobControl(r,id,title,Style(),true){}
 void DrawValue(IGraphics& g,bool)override{char value[32];const double v=GetParam()->Value();std::snprintf(value,sizeof(value),std::abs(v)>=100?"%.0f":"%.1f",v);g.DrawText(mStyle.valueText,value,mValueBounds);}
 void DrawWidget(IGraphics& g)override{const float radius=std::max(1.f,std::min(18.f,GetRadius()-5.f)),cx=mWidgetBounds.MW(),cy=mWidgetBounds.MH();
 const float a=-135.f+270.f*GetValue();
 // Reserve space inside the widget so the 270-degree dotted scale cannot touch labels.
 for(int i=0;i<=24;++i){const float angle=(-225.f+i*270.f/24.f)*.01745329252f;g.FillCircle(IColor(180,107,134,147),cx+std::cos(angle)*(radius+4.f),cy+std::sin(angle)*(radius+4.f),i%6==0?1.f:.7f);}
 g.FillCircle(IColor(100,0,0,0),cx,cy+2,radius+1);
 g.FillCircle(GetMouseIsOver()?IColor(255,25,37,44):IColor(255,17,25,30),cx,cy,radius);g.DrawCircle(IColor(255,69,89,99),cx,cy,radius,nullptr,1.5f);
 g.DrawArc(IColor(35,54,170,226),cx,cy,radius,-135,a,nullptr,6);
 g.DrawArc(Blue,cx,cy,radius,-135,a,nullptr,2);
 const float rad=(a-90)*.01745329252f;g.DrawLine(Text,cx+std::cos(rad)*radius*.55f,cy+std::sin(rad)*radius*.55f,cx+std::cos(rad)*radius*.8f,cy+std::sin(rad)*radius*.8f,nullptr,2);}
};
class Fader final:public IVSliderControl{
public:Fader(IRECT r,int id,const char* title,EDirection dir=EDirection::Vertical):IVSliderControl(r,id,title,Style(),true,dir,DEFAULT_GEARING,8,3,true){}
 void DrawValue(IGraphics& g,bool)override{char value[32];const double v=GetParam()->Value();std::snprintf(value,sizeof(value),"%.1f",v);g.DrawText(mStyle.valueText,value,mValueBounds);}
 void DrawHandle(IGraphics& g,const IRECT& r)override{auto b=r.GetCentredInside(mDirection==EDirection::Vertical?17:10,mDirection==EDirection::Vertical?10:17);g.PathRoundRect(b,2);g.PathFill(IPattern::CreateLinearGradient(b.MW(),b.T,b.MW(),b.B,{{IColor(255,91,197,240),0.f},{Blue,.5f},{IColor(255,36,129,189),1.f}}));g.DrawRoundRect(IColor(255,110,205,242),b,2);}
 IRECT GetTrackBounds()const{auto r=IVSliderControl::GetTrackBounds();return mDirection==EDirection::Vertical?IRECT(r.MW()-3,r.T,r.MW()+3,r.B):IRECT(r.L,r.MH()-3,r.R,r.MH()+3);}
 void DrawTrack(IGraphics& g,const IRECT&)override{const auto r=GetTrackBounds();
 g.FillRoundRect(IColor(255,17,46,65),r,1);g.DrawRoundRect(IColor(255,48,81,99),r,1);
 if(GetValue()>0){g.PathRect(r.FracRect(mDirection,float(GetValue())));g.PathFill(IPattern::CreateLinearGradient(r.L,r.B,r.R,r.T,{{IColor(255,28,94,134),0.f},{Blue,1.f}}));}
 for(int i=0;i<=8;++i){auto color=IColor(255,66,86,97);if(mDirection==EDirection::Vertical){float y=r.T+r.H()*i/8;g.DrawLine(color,r.L-6,y,r.L-2,y);g.DrawLine(color,r.R+2,y,r.R+6,y);}else{float x=r.L+r.W()*i/8;g.DrawLine(color,x,r.T-5,x,r.T-2);g.DrawLine(color,x,r.B+2,x,r.B+5);}}
 }

};
// Parameter-linked schematic, not a measured frequency response or oscilloscope.
class Curve final:public IControl{
 bool envelope_;
public:Curve(IRECT r,std::initializer_list<int> ids,bool envelope):IControl(r,ids),envelope_(envelope){SetIgnoreMouse(true);}
 void Draw(IGraphics& g)override{g.FillRoundRect(DisplayColor,mRECT,3);g.DrawRoundRect(Border,mRECT,3);DrawGrid(g,mRECT.GetPadded(-1));const auto r=mRECT.GetPadded(-10);
 float px=r.L,py=r.B;
 if(envelope_){float a=.08f+.25f*GetValue(0),d=.1f+.25f*GetValue(1),s=GetValue(2),rel=.08f+.25f*GetValue(3);float scale=1/(a+d+.25f+rel);float xs[]={0,a*scale,(a+d)*scale,(a+d+.25f)*scale,1};float ys[]={0,1,s,s,0};for(int i=1;i<5;++i){float x=r.L+xs[i]*r.W(),y=r.B-ys[i]*r.H();g.DrawLine(Blue,px,py,x,y,nullptr,1.5f);g.FillCircle(Blue,x,y,2);px=x;py=y;}}
 else{float cutoff=GetValue(0);int mode=std::lround(GetValue(1)*3);for(int i=0;i<=80;++i){float x=i/80.f;float low=1/(1+std::exp((x-cutoff)*14));float y=mode<2?low:mode==2?1-low:4*low*(1-low);float sy=r.B-y*r.H()*.8f;if(i)g.DrawLine(Blue,px,py,r.L+x*r.W(),sy,nullptr,1.5f);px=r.L+x*r.W();py=sy;}}
 }
};
class Toggle final:public IControl{
 const char* label_;
public:Toggle(IRECT r,int id,const char* label):IControl(r,id),label_(label){}
 void Draw(IGraphics& g)override{const bool on=GetValue()>.5;g.FillRoundRect(on?IColor(255,22,58,80):IColor(255,13,19,22),mRECT,3);g.DrawRoundRect(on||GetMouseIsOver()?Blue:IColor(255,67,87,99),mRECT,3,nullptr,on?1.5f:1.f);char text[80];std::snprintf(text,sizeof(text),"%s%s%s",label_,*label_?"  ":"",on?"ON":"OFF");g.DrawText(IText(11,on?Text:IColor(255,130,150,158)),text,mRECT);}
 void OnMouseDown(float,float,const IMouseMod&)override{SetValue(GetValue()>.5?0:1);SetDirty(true);}
};
class Meter final:public IControl{
 const std::atomic<float>& left_;const std::atomic<float>& right_;const Fader* fader_;
public:Meter(IRECT r,const std::atomic<float>& l,const std::atomic<float>& rt,const Fader* fader):IControl(r),left_(l),right_(rt),fader_(fader){SetIgnoreMouse(true);}
 void Draw(IGraphics& g)override{
 for(int i=0;i<2;++i){const float value=i?right_.load():left_.load();const float fill=std::clamp((20*std::log10(std::max(value,1.e-6f))+60)/60,0.f,1.f);
 const auto track=fader_->GetTrackBounds();const float lane=(mRECT.W()-4.f)/2.f;
 const IRECT r(mRECT.L+i*(lane+4),track.T,mRECT.L+i*(lane+4)+lane,track.B);
 g.FillRect(DisplayColor,r);
 // Twenty equal-height cells: 14 green, 4 yellow, 2 red. No audio changes.
 constexpr int cells=20;const float height=r.H()/cells;
 for(int cell=0;cell<cells;++cell){
  const bool lit=fill>float(cell)/cells;
  const IColor color=cell<14?IColor(255,75,207,127):cell<18?IColor(255,246,211,83):IColor(255,239,89,75);
  const IRECT segment(r.L+1,r.B-(cell+1)*height+1,r.R-1,r.B-cell*height-1);
  g.FillRect(lit?color:IColor(255,color.R/5,color.G/5,color.B/5),segment);
 }
 g.DrawRect(Border,r);
 g.DrawText(IText(9,Muted),i?"R":"L",IRECT(r.L-2,r.B+3,r.R+2,r.B+17));
 }
 }
};
class Status final:public IControl{
 const std::atomic<float>& cpu_;const std::atomic<int>& rate_;const std::atomic<int>& voices_;
public:Status(IRECT r,const std::atomic<float>& c,const std::atomic<int>& sr,const std::atomic<int>& v):IControl(r),cpu_(c),rate_(sr),voices_(v){SetIgnoreMouse(true);}
 void Draw(IGraphics& g)override{char text[160];std::snprintf(text,sizeof(text),"CPU %.1f%%   |   %.1f kHz   |   %d / 16 VOICES   |   KEYBOARD / WHEELS: MIDI CH 1",cpu_.load(),rate_.load()/1000.,voices_.load());g.DrawText(IText(11,Text).WithAlign(EAlign::Near),text,mRECT);}
};

}
