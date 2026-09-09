// SPDX-License-Identifier: MIT
#pragma once
#include "IControls.h"
#include <cmath>
#include <atomic>
#include <cstdio>
namespace sawstar::gui {
using namespace iplug::igraphics;
inline const IColor Text(255,225,234,238),Blue(255,54,170,226),PanelColor(255,20,26,29),Border(255,48,61,67);
inline IVStyle Style(){return DEFAULT_STYLE.WithColor(kBG,PanelColor).WithColor(kFG,IColor(255,36,45,50)).WithColor(kFR,Border).WithColor(kHL,Blue).WithColor(kX1,Blue).WithColor(kX2,Text).WithColor(kX3,Blue).WithDrawShadows(false).WithRoundness(.12f).WithLabelText(IText(11,Text)).WithValueText(IText(10,Text).WithVAlign(EVAlign::Bottom));}
// A recessed value field and chevron identify choices, not on/off actions.
inline void DrawChoice(IGraphics& g,const IRECT& r,const char* value,int size=11){
 g.FillRoundRect(IColor(255,10,16,20),r,3);g.DrawRoundRect(Border,r,3);
 g.DrawText(IText(size,Text).WithAlign(EAlign::Near),value,IRECT(r.L+9,r.T,r.R-25,r.B));
 const float x=r.R-13,y=r.MH();g.DrawLine(Blue,x-4,y-2,x,y+2,nullptr,1.5f);g.DrawLine(Blue,x,y+2,x+4,y-2,nullptr,1.5f);
}
class Dropdown final:public IControl{
 const char* label_;
 IRECT Field()const{const auto area=*label_?mRECT.GetReducedFromTop(17):mRECT;return area.GetCentredInside(area.W(),std::min(32.f,area.H()));}
public:
 Dropdown(IRECT r,int id,const char* label):IControl(r,id),label_(label){mDisablePrompt=false;}
 void Draw(IGraphics& g)override{if(*label_)g.DrawText(IText(11,Text).WithAlign(EAlign::Near),label_,mRECT.GetFromTop(15));WDL_String display;GetParam()->GetDisplay(display);DrawChoice(g,Field(),display.Get());}
 void OnMouseDown(float,float,const IMouseMod&)override{PromptUserInput(Field());}
};
class Section final:public IControl{
 const char* title_;IColor color_;bool tint_;
public: Section(IRECT r,const char* title,IColor color=Blue,bool tint=false):IControl(r),title_(title),color_(color),tint_(tint){SetIgnoreMouse(true);}
 void Draw(IGraphics& g)override{g.FillRoundRect(PanelColor,mRECT,3);g.DrawRoundRect(Border,mRECT,3);
 if(tint_)g.FillRect(IColor(35,color_.R,color_.G,color_.B),mRECT.GetFromTop(35));
 g.DrawText(IText(15,color_).WithAlign(EAlign::Near),title_,IRECT(mRECT.L+12,mRECT.T,mRECT.R-8,mRECT.T+35));}
};
class Knob final:public IVKnobControl{
public:Knob(IRECT r,int id,const char* title):IVKnobControl(r,id,title,Style(),true){}
 void DrawValue(IGraphics& g,bool)override{char value[32];const double v=GetParam()->Value();std::snprintf(value,sizeof(value),std::abs(v)>=100?"%.0f":"%.1f",v);g.DrawText(mStyle.valueText,value,mValueBounds);}
 void DrawWidget(IGraphics& g)override{const float radius=std::min(18.f,GetRadius()),cx=mWidgetBounds.MW(),cy=mWidgetBounds.MH();
 const float a=-135.f+270.f*GetValue();
 g.FillCircle(IColor(255,10,14,17),cx,cy,radius);g.DrawCircle(Border,cx,cy,radius,nullptr,2);
 g.DrawArc(IColor(35,54,170,226),cx,cy,radius,-135,a,nullptr,6);
 g.DrawArc(Blue,cx,cy,radius,-135,a,nullptr,2);
 const float rad=(a-90)*.01745329252f;g.DrawLine(Text,cx+std::cos(rad)*radius*.55f,cy+std::sin(rad)*radius*.55f,cx+std::cos(rad)*radius*.8f,cy+std::sin(rad)*radius*.8f,nullptr,2);}
};
class Fader final:public IVSliderControl{
public:Fader(IRECT r,int id,const char* title,EDirection dir=EDirection::Vertical):IVSliderControl(r,id,title,Style(),true,dir,DEFAULT_GEARING,8,3,true){}
 void DrawValue(IGraphics& g,bool)override{char value[32];const double v=GetParam()->Value();std::snprintf(value,sizeof(value),"%.1f",v);g.DrawText(mStyle.valueText,value,mValueBounds);}
 void DrawHandle(IGraphics& g,const IRECT& r)override{auto b=r.GetCentredInside(mDirection==EDirection::Vertical?17:10,mDirection==EDirection::Vertical?10:17);g.FillRoundRect(Blue,b,2);g.DrawRoundRect(IColor(255,110,205,242),b,2);}
 void DrawTrack(IGraphics& g,const IRECT& filled)override{const auto r=GetTrackBounds();
 for(int i=0;i<=8;++i){if(mDirection==EDirection::Vertical){float y=r.T+r.H()*i/8;g.DrawLine(Border,r.MW()-7,y,r.MW()+7,y);}else{float x=r.L+r.W()*i/8;g.DrawLine(Border,x,r.MH()-6,x,r.MH()+6);}}
 IVSliderControl::DrawTrack(g,filled);}
};
// Parameter-linked schematic, not a measured frequency response or oscilloscope.
class Curve final:public IControl{
 bool envelope_;
public:Curve(IRECT r,std::initializer_list<int> ids,bool envelope):IControl(r,ids),envelope_(envelope){SetIgnoreMouse(true);}
 void Draw(IGraphics& g)override{g.FillRoundRect(IColor(255,9,15,18),mRECT,3);g.DrawRoundRect(Border,mRECT,3);const auto r=mRECT.GetPadded(-10);
 float px=r.L,py=r.B;
 if(envelope_){float a=.08f+.25f*GetValue(0),d=.1f+.25f*GetValue(1),s=GetValue(2),rel=.08f+.25f*GetValue(3);float scale=1/(a+d+.25f+rel);float xs[]={0,a*scale,(a+d)*scale,(a+d+.25f)*scale,1};float ys[]={0,1,s,s,0};for(int i=1;i<5;++i){float x=r.L+xs[i]*r.W(),y=r.B-ys[i]*r.H();g.DrawLine(Blue,px,py,x,y,nullptr,1.5f);g.FillCircle(Blue,x,y,2);px=x;py=y;}}
 else{float cutoff=GetValue(0);int mode=std::lround(GetValue(1)*3);for(int i=0;i<=80;++i){float x=i/80.f;float low=1/(1+std::exp((x-cutoff)*14));float y=mode<2?low:mode==2?1-low:4*low*(1-low);float sy=r.B-y*r.H()*.8f;if(i)g.DrawLine(Blue,px,py,r.L+x*r.W(),sy,nullptr,1.5f);px=r.L+x*r.W();py=sy;}}
 }
};
class Toggle final:public IControl{
 const char* label_;
public:Toggle(IRECT r,int id,const char* label):IControl(r,id),label_(label){}
 void Draw(IGraphics& g)override{const bool on=GetValue()>.5;g.FillRoundRect(on?IColor(255,22,58,80):IColor(255,13,19,22),mRECT,3);g.DrawRoundRect(on?Blue:Border,mRECT,3,nullptr,on?1.5f:1.f);char text[80];std::snprintf(text,sizeof(text),"%s%s%s",label_,*label_?"  ":"",on?"ON":"OFF");g.DrawText(IText(11,on?Text:IColor(255,130,150,158)),text,mRECT);}
 void OnMouseDown(float,float,const IMouseMod&)override{SetValue(GetValue()>.5?0:1);SetDirty(true);}
};
class Meter final:public IControl{
 const std::atomic<float>& left_;const std::atomic<float>& right_;
public:Meter(IRECT r,const std::atomic<float>& l,const std::atomic<float>& rt):IControl(r),left_(l),right_(rt){SetIgnoreMouse(true);}
 void Draw(IGraphics& g)override{
 for(int i=0;i<2;++i){const float value=i?right_.load():left_.load();const float fill=std::clamp((20*std::log10(std::max(value,1.e-6f))+60)/60,0.f,1.f);
 const IRECT r(mRECT.L+i*10,mRECT.T,mRECT.L+i*10+6,mRECT.B);g.FillRect(IColor(255,7,11,13),r);
 // Fixed display-height zones: ~70% green, ~20% orange, ~10% red.
 // These are peak-meter colors, not a change to audio gain or meter ballistics.
 if(fill>0){const auto gradient=IPattern::CreateLinearGradient(r.MW(),r.B,r.MW(),r.T,{
 {IColor(255,40,145,88),0.f},{IColor(255,80,190,108),.68f},
 {IColor(255,222,157,62),.72f},{IColor(255,235,145,54),.88f},
 {IColor(255,221,72,62),.92f},{IColor(255,235,81,69),1.f}});
 g.PathRect(r.GetFromBottom(r.H()*fill));g.PathFill(gradient);}
 g.DrawRect(Border,r);
 }
 }
};
class Status final:public IControl{
 const std::atomic<float>& cpu_;const std::atomic<int>& rate_;const std::atomic<int>& voices_;
public:Status(IRECT r,const std::atomic<float>& c,const std::atomic<int>& sr,const std::atomic<int>& v):IControl(r),cpu_(c),rate_(sr),voices_(v){SetIgnoreMouse(true);}
 void Draw(IGraphics& g)override{char text[160];std::snprintf(text,sizeof(text),"CPU %.1f%%   |   %.1f kHz   |   %d / 16 VOICES   |   KEYBOARD / WHEELS: MIDI CH 1",cpu_.load(),rate_.load()/1000.,voices_.load());g.DrawText(IText(11,Text).WithAlign(EAlign::Near),text,mRECT);}
};

}
