// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
double source(float sr,int source,int type=0,int octave=-1){
 sawstar::Synth s;s.Reset(sr);s.SetParameters(0,1,1,1,20);s.SetOutputBoost(18);
 s.SetMixer(source==0?100:0,source==1?100:0,source==2?100:0,source==3?100:0,0,octave,type,0);
 s.Midi(0x90,69,127);double energy=0;int crossings=0;float last=0;
 for(int i=0;i<sr;++i){auto x=s.ProcessStereo();check(std::isfinite(x.left)&&std::abs(x.left)<=.981f,"source bounds");
  if(i>sr/2){energy+=x.left*x.left;if(last<=0&&x.left>0)++crossings;}last=x.left;}
 check(energy>.01,"isolated source audible");
 if(source==2)check(std::abs(crossings-220*std::exp2(octave))<=2,"sub octave frequency");
 s.Midi(0x80,69,0);for(int i=0;i<sr;++i)s.Process();check(s.ActiveVoices()==0&&s.Process()==0,"source releases including noise");
 return energy;
}
int main(){for(float sr:{44100.f,48000.f,96000.f}){
 for(int i=0;i<4;++i)source(sr,i);
 source(sr,2,0,-2);source(sr,2,0,0);
 check(source(sr,3,1)<source(sr,3,0)*.3,"Dark Noise attenuates high frequency energy");
 sawstar::Synth s;s.Reset(sr);s.SetParameters(0,1,1,1,20);s.SetOutputBoost(24);
 s.SetMixer(100,100,100,100,1,-1,0,-1);s.SetOsc2(30,100,100);s.SetSaw(25,100,100);
 for(int n=48;n<64;++n)s.Midi(0x90,n,127);
 for(int i=0;i<sr;++i){auto x=s.ProcessStereo();check(std::isfinite(x.left)&&std::isfinite(x.right)&&std::abs(x.left)<=.98001f&&std::abs(x.right)<=.98001f,"full mixer chord protected");}
 s.SetOutputBoost(0);
 for(int i=0;i<sr;++i){auto x=s.ProcessStereo();check(std::abs(x.left)<=.98001f&&std::abs(x.right)<=.98001f,"new sources protected even at zero boost");}
 s.SetMixer(0,0,0,0,0,-1,0,0);for(int i=0;i<sr;++i)s.Process();
 check(std::abs(s.Process())<1.e-6,"all mixer faders down mute output");
 // Default mixer must leave the original oscillator unchanged.
 sawstar::Synth a,b;a.Reset(sr);b.Reset(sr);a.SetParameters(0,1,1,1,20);b.SetParameters(0,1,1,1,20);
 b.SetMixer(100,0,0,0,0,-1,0,0);a.Midi(0x90,60,127);b.Midi(0x90,60,127);
 for(int i=0;i<4096;++i)check(a.Process()==b.Process(),"default mixer equivalence");
}}
