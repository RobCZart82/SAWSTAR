// SPDX-License-Identifier: MIT
#include "dsp/LowPass.h"
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<"\n";std::exit(1);}}
double Energy(float sr,float freq,float cutoff){
 sawstar::LowPass f;f.Init(sr);f.Set(cutoff,0,100);double energy=0;
 for(int i=0;i<static_cast<int>(sr);++i){float x=static_cast<float>(std::sin(6.283185307179586*freq*i/sr));auto y=f.Process({x,x});if(i>sr/2)energy+=y.left*y.left;}
 return energy;
}
int main(){
 for(float sr:{8000.f,44100.f,48000.f,96000.f}){
  sawstar::LowPass f;f.Init(sr);f.Set(200,80,0);
  for(int i=0;i<2000;++i){float x=std::sin(i*.3f);auto y=f.Process({x,-x});check(y.left==x&&y.right==-x,"bypass identity");}
  check(Energy(sr,2000,200)<Energy(sr,50,200)*.001,"high frequency rejection");
  f.Set(1000,0,100);sawstar::StereoSample y;
  for(int i=0;i<sr;++i)y=f.Process({1,0});check(std::abs(y.left-1)<1e-5&&y.right==0,"DC unity/channel isolation");
  for(int block=0;block<200;++block){f.Set(block%2?20:20000,100,100);for(int i=0;i<128;++i){auto v=f.Process({.5f,-.5f});check(std::isfinite(v.left)&&std::abs(v.left)<100,"extreme sweep stability");}}
  f.Clear();y=f.Process({0,0});check(y.left==0&&y.right==0,"clear silence");
  f.Process({std::numeric_limits<float>::quiet_NaN(),0});y=f.Process({0,0});check(std::isfinite(y.left),"invalid input recovery");
  sawstar::Synth s;s.Reset(sr);s.SetParameters(0,1,1,1,10);s.SetSaw(50,100,100);s.SetFilter(1000,100,100);
  for(int n=48;n<64;++n)s.Midi(0x90,n,127);
  for(int i=0;i<sr;++i){auto v=s.ProcessStereo();check(std::isfinite(v.left)&&std::abs(v.left)<=1&&std::abs(v.right)<=1,"full synth bounds");}
  s.Midi(0xb0,120,0);y=s.ProcessStereo();check(y.left==0&&y.right==0,"panic silence");
 }
}
