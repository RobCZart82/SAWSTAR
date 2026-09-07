// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <array>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
// Compare averaged spectral power at frequencies two octaves apart.
double pinkSlope(float sr){
 sawstar::PinkNoise n;n.Reset(1234567);
 // DFT bins accumulated in short blocks: averaging avoids random-bin variance.
 constexpr int size=2048;double low=0,high=0;
 for(int block=0;block<256;++block){
  std::array<double,2> re{},im{};
  for(int i=0;i<size;++i){double x=n.Process();
   for(int k=0;k<2;++k){double phase=6.283185307179586*(k?64:16)*i/size;
    double window=.5-.5*std::cos(6.283185307179586*i/size);
    re[k]+=x*window*std::cos(phase);im[k]-=x*window*std::sin(phase);}}
  low+=re[0]*re[0]+im[0]*im[0];high+=re[1]*re[1]+im[1]*im[1];
 }
 (void)sr;return low/high;
}
struct Energy {double total=0,difference=0;};
Energy render(float sr,int type,float color){
 sawstar::Synth s;s.Reset(sr);s.SetParameters(0,1,1,1,20);s.SetMixer(0,0,0,100,0,-1,type,0);s.SetNoiseColor(color);s.Midi(0x90,60,127);
 Energy e;float last=0;
 for(int i=0;i<int(sr)*2;++i){float x=s.Process();check(std::isfinite(x)&&std::abs(x)<=.981f,"noise finite and bounded");
  if(i>sr){e.total+=x*x;e.difference+=(x-last)*(x-last);}last=x;}
 check(e.total>.001,"noise audible");s.Midi(0x80,60,0);
 for(int i=0;i<sr;++i)s.Process();check(s.ActiveVoices()==0,"noise releases");return e;
}
int main(){
 const double slope=pinkSlope(48000);check(slope>2.5&&slope<6.5,"pink power falls approximately 3 dB/octave");
 for(float sr:{44100.f,48000.f,96000.f}){
  for(int type=0;type<3;++type){auto neutral=render(sr,type,0),dark=render(sr,type,-100),bright=render(sr,type,100);
   check(dark.difference/dark.total<neutral.difference/neutral.total*.7,"negative color darkens spectrum");
   check(bright.difference/bright.total>neutral.difference/neutral.total*1.03,"positive color brightens spectrum");}
  // A/B renders share the same random sequences. Compare switched output to
  // an unchanged reference: switching itself must add only a small first step.
  for(int from=0;from<3;++from)for(int to=0;to<3;++to)if(from!=to){
    sawstar::Synth moving,steady,destination;
    for(auto* s:{&moving,&steady,&destination}){s->Reset(sr);s->SetParameters(0,1,1,1,20);s->SetOutputBoost(18);s->SetMixer(0,0,0,100,0,-1,from,0);s->Midi(0x90,60,127);}
    destination.SetMixer(0,0,0,100,0,-1,to,0);
    for(int i=0;i<int(sr)/2;++i){moving.Process();steady.Process();destination.Process();}
    moving.SetMixer(0,0,0,100,0,-1,to,0);
    double changed=0;
    for(int i=0;i<int(sr)/5;++i){float x=moving.Process(),y=steady.Process(),z=destination.Process();
      if(i==0)check(std::abs(x-y)<.01f,"noise selection has a smooth first sample");
      if(i>sr*.15f)check(std::abs(x-z)<1.e-5f,"noise selection settles to destination");
      changed+=std::abs(x-y);
    }
    check(changed>1,"noise type still changes the sound");
  }
  sawstar::Synth a,b;a.Reset(sr);b.Reset(sr);b.SetNoiseColor(100);a.Midi(0x90,60,127);b.Midi(0x90,60,127);
  for(int i=0;i<8192;++i)check(a.Process()==b.Process(),"noise color leaves oscillators unchanged");
 }
}
