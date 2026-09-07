// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
int main(){
  constexpr int rate=48000, samples=48000;
  std::cout<<"scene,voices,realtime_percent,peak_dbfs,rms_dbfs,dc\n";
  for(bool effects:{false,true})for(int count:{1,8,16}){
    auto synth=std::make_unique<sawstar::Synth>();auto& s=*synth;
    s.Reset(rate);s.SetParameters(-6,5,100,.8,100);s.SetOutputBoost(18);
    s.SetSaw(25,70,70);s.SetOsc2(19,60,80);s.SetMixer(70,50,20,5,0,-1,0,0);
    s.SetFilter(1800,30,100);s.SetFilterCharacter(effects?6:0,1);
    if(effects){s.SetChorus(true,20,.3,30);s.SetDelay(true,15,250,30,60,true,false,2,120);s.SetReverb(true,15,50,40,50);}
    for(int n=0;n<count;++n)s.Midi(0x90,48+n,100);
    for(int i=0;i<rate/4;++i)s.ProcessStereo();
    double energy=0,dc=0;float peak=0;
    const auto start=std::chrono::steady_clock::now();
    for(int i=0;i<samples;++i){auto x=s.ProcessStereo();
      if(!std::isfinite(x.left)||!std::isfinite(x.right)||std::abs(x.left)>.98001f||std::abs(x.right)>.98001f)return 1;
      energy+=double(x.left)*x.left+double(x.right)*x.right;dc+=x.left+x.right;
      peak=std::max(peak,std::max(std::abs(x.left),std::abs(x.right)));}
    const double elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
    std::cout<<(effects?"full_fx":"dry_fx")<<','<<count<<','<<std::setprecision(9)<<100*elapsed*rate/samples<<','<<20*std::log10(std::max(peak,1.e-12f))<<','<<10*std::log10(std::max(energy/(samples*2),1.e-24))<<','<<dc/(samples*2)<<'\n';
  }
}
