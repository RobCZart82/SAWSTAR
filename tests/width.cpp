// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <limits>
void check(bool ok,const char* msg){if(!ok){std::cerr<<msg<<'\n';std::exit(1);}}
int main(){for(float sr:{44100.f,48000.f,96000.f}){
 sawstar::Width w;w.Init(sr);float l=.3f,r=-.1f;w.Process(l,r);check(l==.3f&&r==-.1f,"exact initial bypass");
 w.Set(true,100);float previous=.3f;
 for(int i=0;i<int(sr*.03);++i){l=.3f;r=-.1f;w.Process(l,r);check(std::abs((l+r)-.2f)<1e-6,"mono sum preservation");check(std::abs(l-previous)<.001,"smooth enable");previous=l;}
 check(std::abs(l-.5f)<1e-6&&std::abs(r+.3f)<1e-6,"maximum side gain");
 l=r=.2f;w.Process(l,r);check(l==r&&l==.2f,"mono source unchanged");
 w.Set(false,100);for(int i=0;i<int(sr*.03);++i){l=.3f;r=-.1f;w.Process(l,r);check(std::abs(l-previous)<.001,"smooth disable");previous=l;}
 check(l==.3f&&r==-.1f,"exact settled bypass");
 w.Set(true,std::numeric_limits<float>::quiet_NaN());l=.3f;r=-.1f;w.Process(l,r);check(std::isfinite(l),"invalid amount");
 auto s=std::make_unique<sawstar::Synth>();s->Reset(sr);s->SetParameters(0,1,10,1,10);s->SetOutputBoost(24);s->SetSaw(35,100,100);s->SetOsc2(28,100,100);s->SetMixer(100,100,100,100,0,-1,0,0);s->SetChorus(true,60,.4f,60);s->SetDelay(true,40,50,50,6000,true,false,3,120);s->SetReverb(true,40,50,2,6000);s->SetWidth(true,100);
 for(int n=48;n<64;++n)s->Midi(144,n,127);double energy=0;
 for(int i=0;i<int(sr);++i){auto a=s->ProcessStereo();check(std::isfinite(a.left)&&std::isfinite(a.right)&&std::abs(a.left)<=.980001&&std::abs(a.right)<=.980001,"full engine peak guard");energy+=a.left*a.left+a.right*a.right;}
 check(energy>1,"engine emits audio with width");
}std::cout<<"Width: bypass, mono, ramp and engine headroom passed\n";}
