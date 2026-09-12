// SPDX-License-Identifier: MIT
#pragma once
#include <cstdio>
#include "gui/Controls/Theme.h"
#include "visual/Scope.h"
namespace sawstar::gui {
class ScopeControl final:public IControl {
public:
  ScopeControl(const IRECT& bounds):IControl(bounds){SetIgnoreMouse(true);}
  void Update(Scope& scope,bool visible){
    // Drain on every idle, even while hidden. Hold only the displayed frame.
    pendingFresh_=scope.Latest(pending_)||pendingFresh_;
    const auto now=DisplayGate::Clock::now();
    const bool stale=now-pending_.time>std::chrono::milliseconds(200);
    const bool reveal=visible&&!visible_;
    if(visible&&gate_.Due(now,std::chrono::milliseconds(90),reveal||stale!=stale_)){
      if(pendingFresh_){
        frame_=pending_;pendingFresh_=false;
        unsigned length=0;const auto start=ScopeStart(frame_,length);
        float peak=0;for(unsigned i=0;i<length;++i)peak=std::max(peak,std::abs(frame_.samples[start+i]));
        peak_=ScopeDisplayPeak(peak_,peak);
      }
      stale_=stale;SetDirty(false);
    }
    visible_=visible;
  }
  void Draw(IGraphics& g) override {
    g.DrawText(IText(11,Text),"WAVEFORM",mRECT.GetFromTop(20));
    const IRECT r(mRECT.L,mRECT.T+26,mRECT.R,mRECT.B-22);
    g.FillRoundRect(IColor(255,3,10,7),r,3);
    const float step=r.W()/6;
    for(float x=r.L+step;x<r.R;x+=step)g.DrawLine(IColor(255,18,39,28),x,r.T,x,r.B);
    for(float y=r.MH()-step;y>r.T;y-=step)g.DrawLine(IColor(255,18,39,28),r.L,y,r.R,y);
    for(float y=r.MH()+step;y<r.B;y+=step)g.DrawLine(IColor(255,18,39,28),r.L,y,r.R,y);
    g.DrawLine(IColor(255,45,78,51),r.L,r.MH(),r.R,r.MH(),nullptr,1.3f);
    unsigned length=0;const auto start=ScopeStart(frame_,length);
    const IColor green(255,104,244,58);
    if(stale_||length<2)g.DrawLine(green,r.L+2,r.MH(),r.R-2,r.MH(),nullptr,1.f);
    else {
      // Display-only scaling, with a silence floor. No gain enters the audio path.
      const float scale=(r.H()*.43f)/std::max(.05f,peak_);
      const int columns=std::max(2,int(r.W()-4));
      float lastX=r.L+2,lastY=r.MH()-frame_.samples[start]*scale;
      for(int col=0;col<columns;++col){
        const unsigned a=unsigned(col)*length/unsigned(columns),b=std::min(length,std::max(a+1,unsigned(col+1)*length/unsigned(columns)));
        float lo=frame_.samples[start+a],hi=lo;
        for(unsigned i=a+1;i<b;++i){lo=std::min(lo,frame_.samples[start+i]);hi=std::max(hi,frame_.samples[start+i]);}
        const float x=r.L+2+col,y=r.MH()-frame_.samples[start+a]*scale;
        g.DrawLine(IColor(55,104,244,58),lastX,lastY,x,y,nullptr,3.f);
        g.DrawLine(green,lastX,lastY,x,y,nullptr,1.f);
        g.DrawLine(green,x,r.MH()-hi*scale,x,r.MH()-lo*scale,nullptr,1.f);
        lastX=x;lastY=y;
      }
    }
    DrawDisplayBezel(g,r);
    char label[64];std::snprintf(label,sizeof(label),"PRE-FX / %.3g ms / AUTO",1000.*ScopeWindowSamples(frame_.rate)/frame_.rate);
    g.DrawText(IText(9,Text),label,mRECT.GetFromBottom(18));
  }
private:
  Scope::Frame frame_{},pending_{};
  DisplayGate gate_;
  float peak_=.05f;
  bool stale_=true,visible_=false,pendingFresh_=false;
};
}
