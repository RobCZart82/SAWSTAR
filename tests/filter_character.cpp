// SPDX-License-Identifier: MIT
#include "dsp/LowPass.h"
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
double energy(float sr,int mode,float freq){
 sawstar::LowPass f;f.Init(sr);f.Set(1000,0,100);f.SetCharacter(0,mode);double e=0;
 for(int i=0;i<sr;++i){float x=std::sin(6.283185307179586*freq*i/sr);auto y=f.Process({x,0});
  check(y.right==0,"filter channel isolation");if(i>sr/2)e+=y.left*y.left;}
 return e;
}
double third(float sr,float drive){
 sawstar::LowPass f;f.Init(sr);f.Set(12000,0,100);f.SetCharacter(drive,0);double re=0,im=0;
 for(int i=0;i<sr;++i){double phase=6.283185307179586*100*i/sr;auto y=f.Process({static_cast<float>(.2*std::sin(phase)),0});
  if(i>=sr/2){re+=y.left*std::cos(3*phase);im+=y.left*std::sin(3*phase);}}
 return std::hypot(re,im)/(sr/2);
}
int main(){for(float sr:{44100.f,48000.f,96000.f}){
 check(energy(sr,1,4000)<energy(sr,0,4000)*.1,"LP24 steeper than LP12");
 check(energy(sr,2,100)<energy(sr,2,4000)*.001,"HP rejects bass");
 const double band=energy(sr,3,1000);
 check(energy(sr,3,50)<band*.03&&energy(sr,3,10000)<band*.04,"BP rejects both sides");
 check(third(sr,18)>.005&&third(sr,0)<1.e-5,"drive adds measurable third harmonic");
 sawstar::LowPass f;f.Init(sr);f.Set(1000,100,0);
 for(int mode=0;mode<4;++mode){f.SetCharacter(24,mode);
  for(int i=0;i<1024;++i){float x=std::sin(i*.17f);auto y=f.Process({x,-x});check(y.left==x&&y.right==-x,"dry bypass ignores drive and mode");}}
 f.Set(1000,100,100);
 for(int block=0;block<200;++block){f.SetCharacter(block%2?0:24,block%4);f.Set(block%2?20:20000,100,100);
  for(int i=0;i<128;++i){auto y=f.Process({.7f,-.7f});check(std::isfinite(y.left)&&std::abs(y.left)<100&&y.left==-y.right,"mode/drive sweep stability");}}
 f.Clear();auto y=f.Process({0,0});check(y.left==0&&y.right==0,"clear all filter stages");
 for(int mode=0;mode<4;++mode){sawstar::Synth s;s.Reset(sr);s.SetParameters(0,1,1,1,20);s.SetOutputBoost(24);
  s.SetMixer(100,100,100,100,0,-1,0,0);s.SetSaw(50,100,100);s.SetOsc2(40,100,100);
  s.SetFilter(1000,100,100);s.SetFilterCharacter(24,mode);
  for(int n=48;n<64;++n)s.Midi(0x90,n,127);
  for(int i=0;i<sr/4;++i){auto z=s.ProcessStereo();check(std::isfinite(z.left)&&std::isfinite(z.right)&&std::abs(z.left)<=.98001f&&std::abs(z.right)<=.98001f,"driven mixer chord bounds");}
 }
}}
