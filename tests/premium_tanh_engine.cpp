// SPDX-License-Identifier: MIT
#include "PremiumSynth.h"
#include "TanhPremiumSynth.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
using Reference = sawstar::experimental_engine::Synth;
using Study = sawstar::experimental_tanh_engine::Synth;
template<class S> void Setup(S& s,double rate,int mode){
  s.Reset(rate);s.SetParameters(-6,5,100,.8,100);s.SetOutputBoost(18);
  s.SetSaw(25,70,70);s.SetOsc2(19,60,80);s.SetMixer(70,50,20,5,0,-1,0,0);
  s.SetFilter(1800,50,100);s.SetFilterCharacter(20,mode);
  s.SetChorus(true,20,.3,30);s.SetDelay(true,15,250,30,60,true,false,2,120);
  s.SetReverb(true,15,50,40,50);
  for(int n=0;n<16;++n)s.Midi(0x90,48+n,100);
}
void Contract(){
  double maximum=0;
  for(double rate:{48000.,96000.,192000.})for(int mode=0;mode<4;++mode){
    auto a=std::make_unique<Reference>();auto b=std::make_unique<Study>();Setup(*a,rate,mode);Setup(*b,rate,mode);
    for(int n=0;n<8192;++n){
      if(n%1024==0){const float db=(n/1024)%3*12;a->SetFilterCharacter(db,mode);b->SetFilterCharacter(db,mode);}
      const auto x=a->ProcessStereo(),y=b->ProcessStereo();
      for(double d:{std::abs(double(x.left)-y.left),std::abs(double(x.right)-y.right)}){
        if(!std::isfinite(d)||d>2e-6)throw std::runtime_error("Full engine study deviation");maximum=std::max(maximum,d);
      }
    }
    a->Reset(rate);b->Reset(rate);
    for(int n=0;n<64;++n){const auto x=a->ProcessStereo(),y=b->ProcessStereo();
      if(x.left!=0||x.right!=0||y.left!=0||y.right!=0)throw std::runtime_error("Study engine reset silence");}
  }
  std::cout<<"Full engine maximum absolute output difference: "<<maximum<<'\n';
}
volatile double checksum=0;
template<class S>double Measure(double rate,int mode){
  auto s=std::make_unique<S>();Setup(*s,rate,mode);for(int n=0;n<2048;++n)s->ProcessStereo();
  double sum=0;const auto start=std::chrono::steady_clock::now();
  for(int n=0;n<4096;++n){const auto y=s->ProcessStereo();sum+=y.left+y.right;}
  const double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();checksum=checksum+sum;return seconds;
}
int main(int argc,char** argv){try{
  if(argc==1)Contract();
  else if(argc==2&&std::string(argv[1])=="--benchmark"){
    std::cout<<"rate,mode,voices,drive_db,fx,pair,reference_seconds,study_seconds\n";
    for(double rate:{48000.,96000.,192000.})for(int mode=0;mode<4;++mode)for(int pair=0;pair<4;++pair){
      double a,b;if(pair%2){b=Measure<Study>(rate,mode);a=Measure<Reference>(rate,mode);}else{a=Measure<Reference>(rate,mode);b=Measure<Study>(rate,mode);}
      if(!std::isfinite(a)||!std::isfinite(b)||a<=0||b<=0)throw std::runtime_error("Invalid engine time");
      std::cout<<rate<<','<<mode<<",16,20,1,"<<pair<<','<<a<<','<<b<<'\n';
    }
    if(!std::isfinite(checksum))throw std::runtime_error("Invalid checksum");
  }else throw std::runtime_error("Usage: premium_tanh_engine [--benchmark]");
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
