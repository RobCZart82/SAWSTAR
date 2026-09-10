// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include "dsp/Safety.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
void check(bool ok,const char* reason){if(!ok){std::cerr<<reason<<'\n';std::exit(1);}}
int main(){
  for(int type=0;type<3;++type)for(float color:{-100.f,0.f,100.f}){
    sawstar::Synth a,b;
    a.Reset(48000);a.SetNoiseColor(color);a.Reset(48000);
    b.Reset(48000);b.SetNoiseColor(color);
    for(auto* s:{&a,&b}){s->SetParameters(0,1,1,1,20);s->SetMixer(0,0,0,100,0,-1,type,0);s->Midi(0x90,60,127);}
    double energy=0;
    for(int i=0;i<8192;++i){float x=a.Process(),y=b.Process();check(x==y,"Reset must retain Noise Color");energy+=x*x;}
    check(energy>0,"reset comparison must be audible");
  }
  for(float bad:{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()}){
    check(sawstar::SafeSampleRate(bad)==44100,"invalid rate fallback");
    sawstar::SubOscillator sub;sub.Init(bad);sub.SetFreq(bad);
    sawstar::SevenSaw osc;osc.Init(bad);osc.SetPitchMultiplier(bad);
    sawstar::Synth synth;synth.Reset(bad);synth.SetParameters(bad,bad,bad,bad,bad);synth.SetNoiseColor(bad);synth.SetFilter(bad,bad,bad);synth.Midi(0x90,60,100);
    for(int i=0;i<1024;++i){auto x=osc.Process();check(std::isfinite(x.left)&&std::isfinite(x.right)&&std::isfinite(sub.Process())&&std::isfinite(synth.Process()),"invalid input poisons DSP");}
    sub.SetFreq(220);osc.SetFreq(220);osc.SetPitchMultiplier(1);synth.SetParameters(0,1,1,1,20);
    double energy=0;for(int i=0;i<4096;++i){float x=synth.Process();check(std::isfinite(x),"recovery remains finite");energy+=x*x;}
    check(energy>0,"valid input recovers audible output");
  }
  check(sawstar::SafeSampleRate(1.e300)==384000,"extreme finite rate bounded");
}
