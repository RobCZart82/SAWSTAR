// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cmath>
#include <iostream>
#include <memory>
#include <cstdlib>
void check(bool ok,const char* message){if(!ok){std::cerr<<message<<"\n";std::exit(1);}}
int main(){
 for(float sr:{44100.f,48000.f,96000.f})for(bool reverse:{false,true}){
  auto s=std::make_unique<sawstar::Synth>();s->Reset(sr);
  s->SetParameters(reverse?-24:0,10,100,1,100);s->SetOutputBoost(reverse?24:0);
  s->SetMixer(10,0,0,0,0,-1,0,0);s->Midi(0x90,60,100);
  for(int i=0;i<int(sr/2);++i)s->ProcessStereo();
  auto reference=std::make_unique<sawstar::Synth>(*s);
  // Equal total gain at both endpoints: changing the split must not swell.
  s->SetParameters(reverse?0:-24,10,100,1,100);s->SetOutputBoost(reverse?0:24);
  double maximum=0;
  for(int i=0;i<int(sr/10);++i){auto a=s->ProcessStereo(),b=reference->ProcessStereo();
   check(std::isfinite(a.left)&&std::isfinite(a.right),"nonfinite gain output");
   if(std::abs(b.left)>1.e-6)maximum=std::max(maximum,double(std::abs(a.left/b.left)));
  }
  std::cout<<sr<<" reverse="<<reverse<<" max ratio="<<maximum<<std::endl;
  check(std::abs(maximum-1)<.0001,"compensated Volume/Boost creates gain excursion");
 }
 // Independent automation must still approach its requested gain smoothly.
 for(float sr:{44100.f,48000.f,96000.f})for(bool boost:{false,true}){
  auto s=std::make_unique<sawstar::Synth>();s->Reset(sr);
  s->SetParameters(-24,10,100,1,100);s->SetOutputBoost(0);
  s->SetMixer(10,0,0,0,0,-1,0,0);s->Midi(0x90,60,100);
  for(int i=0;i<int(sr/2);++i)s->ProcessStereo();
  auto reference=std::make_unique<sawstar::Synth>(*s);
  if(boost)s->SetOutputBoost(12);else s->SetParameters(-12,10,100,1,100);
  const double target=std::pow(10.,12./20.);double previous=1,last=1;
  for(int i=0;i<int(sr/10);++i){auto a=s->ProcessStereo(),b=reference->ProcessStereo();
   if(std::abs(b.left)>1.e-6){last=a.left/b.left;
    check(last>=previous-1.e-5 && last<=target+1.e-4,"single output control overshoots or reverses");previous=last;}
  }
  check(std::abs(last-target)<.001,"single output control fails to settle");
 }

}
