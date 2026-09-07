// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
void check(bool value,const char* why){if(!value){std::cerr<<why<<'\n';std::exit(1);}}
int main(){for(float sr:{44100.f,48000.f,96000.f}){
 for(bool full:{false,true}){
  sawstar::Synth s;s.Reset(sr);s.SetParameters(0,1,1,1,10);s.SetOutputBoost(18);
  s.Midi(0x90,60,127);
  if(full)for(int ch=1;ch<16;++ch)s.Midi(0x90|ch,60,1);
  float previous=0;for(int i=0;i<int(sr/2)+17;++i)previous=s.Process();
  // Same-note velocity retrigger or oldest-voice replacement at full capacity.
  s.Midi(0x90,full?91:60,1);float next=s.Process();
  check(std::abs(next-previous)<.004,"voice reuse must not jump in level");
  check(s.ActiveVoices()==(full?16:1),"voice count remains bounded");
  for(int n=0;n<100;++n){s.Midi(0x90,36+n%60,n%2?1:127);
   for(int i=0;i<7;++i){auto x=s.ProcessStereo();check(std::isfinite(x.left)&&std::abs(x.left)<=.98001f,"dense reuse remains bounded");}}
  for(int ch=0;ch<16;++ch)s.Midi(0xb0|ch,123,0);
  for(int i=0;i<int(sr);++i)s.Process();
  check(s.ActiveVoices()==0&&s.Process()==0,"splice tails release fully");
  s.Midi(0x90,60,127);s.Process();s.Midi(0x90,60,1);s.Process();s.Midi(0xb0,120,0);
  check(s.Process()==0,"panic clears splice immediately");
 }
}}
