// SPDX-License-Identifier: MIT
#include "dsp/Effects/Delay.h"
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
using namespace sawstar;
void check(bool ok,const char* message){if(!ok){std::cerr<<message<<'\n';std::exit(1);}}
int main(){
 for(float sr:{44100.f,48000.f,96000.f,192000.f}){
  Delay d;d.Init(sr);
  for(int i=0;i<1000;++i){auto a=d.Process({.2f,-.3f});check(a.left==.2f&&a.right==-.3f,"default bypass exact");}
  d.Set(true,100,100,50,16000,true,false,3,120);
  for(int i=0;i<sr*2;++i)d.Process({0,0});
  double firstL=0,firstR=0,secondL=0,secondR=0;
  for(int i=0;i<sr*.35f;++i){auto a=d.Process(i==0?StereoSample{1,1}:StereoSample{});
   check(std::isfinite(a.left)&&std::isfinite(a.right),"ping pong finite");
   if(i>sr*.095f&&i<sr*.11f){firstL+=std::abs(a.left);firstR+=std::abs(a.right);}
   if(i>sr*.195f&&i<sr*.21f){secondL+=std::abs(a.left);secondR+=std::abs(a.right);}
  }
  check(firstL>.8&&firstR<.001&&secondR>.3&&secondL<.001,"100 ms ping pong impulse alternates");
  d.Init(sr);d.Set(true,100,250,0,16000,false,false,3,120);for(int i=0;i<sr*2;++i)d.Process({0,0});
  int first=-1;double right=0,echo=0;
  for(int i=0;i<sr*.4f;++i){auto a=d.Process(i==0?StereoSample{1,0}:StereoSample{});if(first<0&&a.left>.001)first=i;right+=std::abs(a.right);echo+=std::abs(a.left);}
  check(std::abs(first-sr*.25f)<4&&echo>.9&&right==0,"free time and stereo isolation");
  d.Set(true,20,350,30,6000,false,true,2,120);check(std::abs(d.TimeSeconds()-.375f)<1.e-5,"dotted eighth timing");
  d.Set(true,20,350,30,6000,false,true,3,60);check(std::abs(d.TimeSeconds()-1)<1.e-5,"tempo quarter follows BPM");
  d.Set(true,20,350,30,6000,false,true,6,20);check(d.TimeSeconds()==2,"long sync clamps to two seconds");
  d.Set(true,20,350,30,6000,false,true,3,0);check(std::abs(d.TimeSeconds()-.5f)<1.e-5,"missing BPM fallback");
  // Continuous automation and maximum feedback remain bounded without input growth.
  for(int i=0;i<sr*3;++i){if(i%701==0)d.Set(true,100,i%2?1:2000,85,i%3?200:16000,i%2,false,0,120);
   auto a=d.Process({.1f,-.1f});check(std::isfinite(a.left)&&std::abs(a.left)<2&&std::abs(a.right)<2,"automation stable");}
  d.Set(false,100,100,85,6000,true,false,0,120);for(int i=0;i<sr;++i)d.Process({0,0});
  auto a=d.Process({.2f,-.3f});check(d.IsDry()&&a.left==.2f&&a.right==-.3f,"disable exact after fade");
  d.Set(true,100,100,0,6000,false,false,0,120);for(int i=0;i<sr*.3f;++i){auto z=d.Process({0,0});check(z.left==0&&z.right==0,"disabled history cannot reappear");}
  d.Init(sr);a=d.Process({0,0});check(a.left==0&&a.right==0,"reset silent");
  d.Set(true,0,100,85,6000,false,false,0,120);a=d.Process({.2f,-.3f});check(a.left==.2f&&a.right==-.3f,"zero mix exact");
  d.Set(true,NAN,NAN,NAN,NAN,true,true,99,std::numeric_limits<double>::infinity());a=d.Process({0,0});check(std::isfinite(a.left)&&std::isfinite(a.right),"invalid values sanitized");
 }
 // Low-pass tone removes high-frequency energy from the wet signal.
 Delay bright,dark;bright.Init(48000);dark.Init(48000);
 bright.Set(true,100,20,0,16000,false,false,0,120);dark.Set(true,100,20,0,200,false,false,0,120);
 double b=0,d=0;
 for(int i=0;i<96000;++i){float x=.2f*std::sin(i*6.2831853f*8000/48000);auto y=bright.Process({x,x}),z=dark.Process({x,x});if(i>48000){b+=y.left*y.left;d+=z.left*z.left;}}
 check(d<b*.02,"tone darkens repeats");
 Synth dry,off,wet;for(auto s:{&dry,&off,&wet}){s->Reset(48000);s->SetParameters(-12,1,100,.7,50);s->Midi(0x90,60,100);}
 off.SetDelay(false,100,100,85,6000,true,false,0,120);wet.SetChorus(true,25,.3f,35);wet.SetDelay(true,25,150,35,4500,true,false,0,120);
 double diff=0;for(int i=0;i<96000;++i){auto a=dry.ProcessStereo(),b=off.ProcessStereo(),c=wet.ProcessStereo();check(a.left==b.left&&a.right==b.right,"synth dry identity");check(std::isfinite(c.left)&&std::abs(c.left)<=1&&std::abs(c.right)<=1,"effects chain bounded");diff+=std::abs(a.left-c.left);}
 check(diff>.01,"effects chain audible");
}
