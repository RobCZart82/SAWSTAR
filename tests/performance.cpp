// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
void prepare(sawstar::Synth& s,float sr,int ch=0){s.Reset(sr);s.SetParameters(0,1,1,1,10);s.Midi(0x90|ch,69,127);}
double frequency(sawstar::Synth& s,float sr){
 for(int i=0;i<sr/2;++i)s.Process();
 int crossings=0;float previous=s.Process();
 for(int i=0;i<sr;++i){float x=s.Process();if(previous<0&&x>=0)++crossings;previous=x;}
 return crossings;
}
int main(){
 for(float sr:{8000.f,44100.f,48000.f,96000.f}){
  sawstar::Synth s;prepare(s,sr);
  check(std::abs(frequency(s,sr)-440)<2,"center pitch");
  s.Midi(0xe0,127,127);check(s.PitchBend(0)==16383,"14-bit maximum");
  check(std::abs(frequency(s,sr)-440*std::exp2(2./12))<2,"positive bend endpoint");
  s.Midi(0xe0,0,0);check(std::abs(frequency(s,sr)-440*std::exp2(-2./12))<2,"negative bend endpoint");
  s.SetPerformance(12,24);check(std::abs(frequency(s,sr)-220)<2,"live bend range change");
  s.Midi(0xb0,1,127);check(s.ModWheel(0)==127,"CC1 state");
  s.Midi(0xb0,121,0);check(s.PitchBend(0)==8192&&s.ModWheel(0)==0,"reset controllers");
  check(std::abs(frequency(s,sr)-440)<2,"reset pitch");
  s.Midi(0xe0,127,127);s.Midi(0xb0,120,0);s.Midi(0x90,69,127);
  check(std::abs(frequency(s,sr)-880)<2,"new note uses current bend");
  s.Reset(sr);check(s.PitchBend(0)==8192&&s.ModWheel(0)==0,"reset clears performance");
  sawstar::Synth a,b;prepare(a,sr,1);prepare(b,sr,1);
  a.Midi(0xe0,0,0);a.Midi(0xb0,1,127);
  for(int i=0;i<sr/4;++i){auto x=a.ProcessStereo(),y=b.ProcessStereo();check(x.left==y.left&&x.right==y.right,"channel isolation");}
  prepare(a,sr);prepare(b,sr);a.SetFilter(200,0,100);b.SetFilter(200,0,100);b.Midi(0xb0,1,127);
  double dry=0,wet=0;
  for(int i=0;i<sr;++i){float x=a.Process(),y=b.Process();if(i>sr/2){dry+=x*x;wet+=y*y;}}
  check(wet>dry*2,"mod wheel opens filter");
  sawstar::FilterModulation f;f.Init(sr);f.Set(200,0,0,10,100,0,100);f.Trigger(true);
  check(f.Process(60,true,24)==800,"wheel depth in semitones");
  for(int block:{1,32,512,2048})for(int step=0;step<20;++step){
   a.SetPerformance(step%2?24:0,48);a.Midi(0xe0,step%2?127:0,step%2?127:0);a.Midi(0xb0,1,step%2?127:0);
   for(int i=0;i<block;++i){auto x=a.ProcessStereo();check(std::isfinite(x.left)&&std::abs(x.left)<=1,"controller sweep bounds");}
  }
  a.Midi(0xb0,120,0);check(a.Process()==0,"panic remains silent");
 }
}
