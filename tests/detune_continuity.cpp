// SPDX-License-Identifier: MIT
#include "dsp/SevenSaw.h"
#include <cmath>
#include <iostream>
#include <cstdlib>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<"\n";std::exit(1);}}
int main(){for(float sr:{44100.f,48000.f,96000.f})for(int wave=0;wave<4;++wave){
 sawstar::SevenSaw a;a.Init(sr);a.SetFreq(220);a.SetShape(0,1,1);a.SetWaveform(wave);a.SnapToTargets();
 for(int i=0;i<1000;++i)a.Process();
 auto b=a;a.SetShape(50,1,1);b.SetShape(50,1,1);
 double error=0;
 for(int i=0;i<int(sr*.05);++i){
  // Redundant MIDI pitch updates must not bypass independent detune smoothing.
  b.SetFreq(220);auto x=a.Process(),y=b.Process();
  error=std::max(error,double(std::abs(x.left-y.left)+std::abs(x.right-y.right)));
 }
 std::cout<<sr<<" wave="<<wave<<" error="<<error<<std::endl;
 check(error==0,"SetFreq interrupts detune interpolation");
 // A real octave jump must take effect immediately without resetting detune.
 a.SetShape(0,1,1);b=a;
 a.SetPitchMultiplier(2);b.SetFreq(440);
 for(int i=0;i<500;++i){auto x=a.Process(),y=b.Process();check(x.left==y.left&&x.right==y.right,"pitch update differs from immediate octave multiplier");}
 a.SetPitchMultiplier(1);
 // Idle voice initialization must still be able to snap to the selected shape.
 a.SetShape(12,1,1);a.SnapToTargets();b=a;b.SetFreq(220);
 for(int i=0;i<500;++i){auto x=a.Process(),y=b.Process();check(x.left==y.left&&x.right==y.right,"settled tuning changed");}
}}
