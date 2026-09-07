// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
void prepare(sawstar::Synth& s,float sr){s.Reset(sr);s.SetParameters(0,1,1,1,10);s.SetOutputBoost(18);s.Midi(0x90,60,127);}
int main(){for(float sr:{44100.f,48000.f,96000.f}){
 for(int control=0;control<5;++control){sawstar::Synth changed,reference;prepare(changed,sr);prepare(reference,sr);
  for(int i=0;i<int(sr/2)+17;++i){changed.Process();reference.Process();}
  if(control==0)changed.SetParameters(0,1,1,0,10);
  if(control==1)changed.SetParameters(-60,1,1,1,10);
  if(control==2)changed.SetMixer(0,0,0,0,0,-1,0,0);
  if(control==3)changed.SetWaveforms(3,0);
  if(control==4)changed.SetFilter(100,0,100);
  const float first=changed.Process(),original=reference.Process();
  check(std::abs(first-original)<(control==0?.0002f:.006f),"parameter change first-sample discontinuity");
  double difference=0;
  for(int i=0;i<int(sr/4);++i){auto x=changed.ProcessStereo(),y=reference.ProcessStereo();
    check(std::isfinite(x.left)&&std::abs(x.left)<=.98001f,"parameter transition remains bounded");difference+=std::abs(x.left-y.left);}
  check(difference>1,"parameter still reaches an audible change");
  if(control<3)check(std::abs(changed.Process())<1.e-3f,"level control settles near silence");
 }
 // Live sustain retargeting and rapid envelope/level changes cannot stick notes.
 sawstar::Synth s;prepare(s,sr);
 for(int step=0;step<200;++step){s.SetParameters(step%2?-6:0,1,1,step%2?0:1,10);
  for(int i=0;i<32;++i)check(std::isfinite(s.Process()),"rapid envelope automation");}
 s.Midi(0x80,60,0);for(int i=0;i<int(sr);++i)s.Process();
 check(s.ActiveVoices()==0&&s.Process()==0,"automation note releases");
}}
