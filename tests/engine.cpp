// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cmath>
#include <iostream>
#include <cstdlib>
void check(bool ok, const char* what) { if (!ok) { std::cerr << what << "\n"; std::exit(1); } }
int main() {
 for (double rate : {44100., 48000., 96000.}) {
  sawstar::Synth s; s.Reset(rate); s.SetParameters(-12, 10, 100, .7, 20);
  check(s.Process() == 0, "initial silence");
  s.Midi(0x90, 69, 100); double energy = 0;
  for (int i=0; i<rate; ++i) { float x=s.Process(); check(std::isfinite(x) && std::abs(x)<=1, "finite bounded output"); energy += x*x; }
  check(energy > .001 && s.Held(69), "audible note");
  s.Midi(0xB0,64,127); s.Midi(0x90,69,0);
  check(!s.Held(69), "velocity zero releases key");
  for(int i=0;i<rate;++i) s.Process(); check(s.ActiveVoices()==1, "sustain pedal");
  s.Midi(0xB0,64,0); for(int i=0;i<rate;++i) s.Process(); check(s.ActiveVoices()==0, "release completes");
  for(int n=20;n<50;++n) s.Midi(0x90,n,127);
  check(s.ActiveVoices()==16, "bounded polyphony");
  s.Midi(0xB0,120,0); check(s.Process()==0 && s.ActiveVoices()==0,"all sound off");
  s.Midi(0x91,60,127); s.Midi(0x80,60,0); check(s.Held(60),"channel isolation");
  s.Reset(rate); check(s.Process()==0 && !s.Held(60),"reset clears voices");
 }
}
