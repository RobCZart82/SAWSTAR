// SPDX-License-Identifier: MIT
#include "dsp/SevenSaw.h"
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
void check(bool ok,const char* what){if(!ok){std::cerr<<what<<"\n";std::exit(1);}}
int main(){
 for(float sr:{44100.f,48000.f,96000.f}) {
  sawstar::SevenSaw s;s.Init(sr);s.SetShape(20,0,1);s.SetFreq(440);
  daisysp::Oscillator reference;reference.Init(sr);reference.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_SAW);reference.SetAmp(1);reference.SetFreq(440);
  for(int i=0;i<10000;++i){auto v=s.Process();check(v.left==v.right && std::abs(v.left-reference.Process())<1e-6f,"single saw compatibility");}
  s.SetShape(50,1,0);for(int i=0;i<sr;++i){auto v=s.Process();check(std::isfinite(v.left)&&std::abs(v.left)<=1.001,"bounds");}
  s.Init(sr);s.SetShape(25,1,0);s.SetFreq(440);
  for(int i=0;i<10000;++i){auto v=s.Process();check(v.left==v.right,"width zero mono");}
  s.SetShape(25,1,1);double side=0,mid=0;
  for(int i=0;i<sr;++i){auto v=s.Process();side+=(v.left-v.right)*(v.left-v.right);mid+=(v.left+v.right)*(v.left+v.right);}
  check(side>1 && mid>1,"stereo and mono-fold energy");
  for(float hz:{8.1758f,440.f,12543.85f}){s.SetFreq(hz);s.SetShape(50,1,1);for(int i=0;i<1000;++i){auto v=s.Process();check(std::isfinite(v.left)&&std::isfinite(v.right)&&std::abs(v.left)<=1.001&&std::abs(v.right)<=1.001,"pitch extremes");}}
  sawstar::Synth synth;synth.Reset(sr);synth.SetParameters(0,1,10,1,20);synth.SetSaw(50,100,100);
  for(int n=48;n<64;++n)synth.Midi(0x90,n,127);
  for(int i=0;i<sr;++i){auto v=synth.ProcessStereo();check(std::isfinite(v.left)&&std::abs(v.left)<=1&&std::abs(v.right)<=1,"112 saw stress");}
  for(int ch=0;ch<16;++ch)synth.Midi(0xb0|ch,120,0);
  auto silent=synth.ProcessStereo();check(silent.left==0&&silent.right==0,"panic silence");
 }
}
