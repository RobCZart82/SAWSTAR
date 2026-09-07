// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
void setup(sawstar::Synth& s,float sr){s.Reset(sr);s.SetParameters(0,1,1,.8,10);s.SetFilter(800,0,100);s.SetMixer(70,0,0,30,0,-1,0,0);s.SetPerformance(2,0);}
int main(){for(float sr:{44100.f,48000.f,96000.f}){
 sawstar::Modulation matrix;matrix.Init(sr);
 for(int source=1;source<=5;++source)for(int target=0;target<5;++target){
  matrix.Init(sr);matrix.Set(0,source,target,-50);
  for(int i=0;i<sr/2;++i)matrix.Process();
  sawstar::Modulation::Values inputs{};inputs[source-1]=1;auto out=matrix.Evaluate(inputs);
  for(int t=0;t<5;++t)check(t==target?out[t]<-.49f:out[t]==0,"source/destination isolation and negative amount");
  matrix.Set(0,0,target,100);for(int i=0;i<sr/2;++i)matrix.Process();
  check(std::abs(matrix.Evaluate(inputs)[target])<1.e-5,"Off fades route out");
 }
 // Old/default audio is exactly preserved even when disabled new controls differ.
 sawstar::Synth a,b;setup(a,sr);setup(b,sr);b.SetLfo2(19,0,3,2,true,5,145,true);
 for(int row=0;row<4;++row)b.SetModulation(row,0,row,100);
 a.Midi(0x90,60,100);b.Midi(0x90,60,100);
 for(int i=0;i<4096;++i){auto x=a.ProcessStereo(),y=b.ProcessStereo();check(x.left==y.left&&x.right==y.right,"disabled modulation preserves audio");}
 // Every source and target must affect rendered audio, not merely matrix values.
 for(int source=1;source<=5;++source)for(int target=0;target<5;++target){
  sawstar::Synth plain,mod;setup(plain,sr);setup(mod,sr);
  mod.SetModulation(0,source,target,60);double difference=0;
  for(auto s:{&plain,&mod}){s->SetLfo(4,0,3,0,false,2,120,false);s->SetLfo2(7,0,3,0,false,2,120,false);s->Midi(0xb0,1,100);s->Midi(0xd0,90,0);s->Midi(0x90,60,100);}
  for(int i=0;i<8192;++i){auto x=plain.ProcessStereo(),y=mod.ProcessStereo();difference+=std::abs(x.left-y.left)+std::abs(x.right-y.right);check(std::isfinite(y.left)&&std::abs(y.left)<=.981,"modulated output bounded");}
  check(difference>.001,"each route affects audio");
 }
 // Channel pressure is isolated by MIDI channel, and reset-controllers clears it.
 sawstar::Synth plain,pressure;setup(plain,sr);setup(pressure,sr);pressure.SetModulation(0,5,1,50);
 plain.Midi(0x90,60,100);pressure.Midi(0x90,60,100);pressure.Midi(0xd1,127,0);
 for(int i=0;i<4096;++i)check(plain.Process()==pressure.Process(),"pressure channel isolation");
 pressure.Midi(0xd0,127,0);for(int i=0;i<4096;++i)pressure.Process();pressure.Midi(0xb0,121,0);
 // Two independent direct LFO paths can operate together; stress all four rows.
 sawstar::Synth stress;setup(stress,sr);stress.SetLfo(5,100,2,0,false,2,120,true);stress.SetLfo2(3,100,0,1,true,3,140,true);
 for(int row=0;row<4;++row)stress.SetModulation(row,row+1,row,row%2?-100:100);
 for(int note=48;note<64;++note)stress.Midi(0x90,note,127);
 for(int i=0;i<8192;++i){auto x=stress.ProcessStereo();check(std::isfinite(x.left)&&std::isfinite(x.right)&&std::abs(x.left)<=.981&&std::abs(x.right)<=.981,"combined modulation protected");}
 stress.Midi(0xb0,123,0);for(int i=0;i<sr;++i)stress.Process();check(stress.ActiveVoices()==0,"modulated voices release");
}}
