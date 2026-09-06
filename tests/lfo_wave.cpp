// SPDX-License-Identifier: MIT
#include "dsp/Lfo.h"
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
int main(){for(float sr:{44100.f,48000.f,96000.f}){
 for(int w=0;w<4;++w){sawstar::SevenSaw osc;osc.Init(sr);osc.SetFreq(220);osc.SetShape(25,.8f,1);osc.SetWaveform(w);double e=0;
  for(int i=0;i<sr/2;++i){auto x=osc.Process();check(std::isfinite(x.left)&&std::isfinite(x.right)&&std::abs(x.left)<2.1f&&std::abs(x.right)<2.1f,"waveform finite bounded");e+=x.left*x.left;}
  check(e>1,"each waveform audible");
  for(int i=0;i<4096;++i){if(i%256==0)osc.SetWaveform((i/256)%4);auto x=osc.Process();check(std::isfinite(x.left)&&std::abs(x.left)<2.1,"switching stable");}
 }
 sawstar::Lfo l;l.Init(sr);l.Set(2,100,0,0,false,2,120,true);
 for(int i=0;i<sr;++i)l.Process();l.Trigger();for(int i=0;i<sr/8;++i)l.Process();
 check(std::abs(l.Phase()-.25)<.001,"free LFO 2 Hz phase");
 l.Set(1,100,0,0,true,3,120,true);for(int i=0;i<sr;++i)l.Process();l.Trigger();
 for(int i=0;i<sr/8;++i)l.Process();check(std::abs(l.Phase()-.5)<.001,"tempo LFO 1/8 at 120 BPM = 4 Hz");
 l.Set(1,0,0,0,false,2,120,false);const auto before=l.Phase();l.Trigger();check(l.Phase()==before,"free phase ignores note trigger");
 for(int target=0;target<4;++target)for(int shape=0;shape<4;++shape){l.Set(20,100,shape,target,false,2,120,false);
  for(int i=0;i<sr/4;++i){auto m=l.Process();check(std::isfinite(m.cutoff)&&std::abs(m.cutoff)<=24.001&&std::abs(m.pitch)<=1.001&&m.amp>=-.001&&m.amp<=1.001&&std::abs(m.pan)<=1.001,"modulation ranges");}}
 // Default/off must be sample-identical to the pre-LFO path.
 sawstar::Synth a,b;for(auto s:{&a,&b}){s->Reset(sr);s->SetParameters(-12,1,1,1,20);s->Midi(0x90,60,100);}
 b.SetWaveforms(0,0);b.SetLfo(12,0,3,3,true,5,145,true);
 for(int i=0;i<4096;++i)check(a.Process()==b.Process(),"LFO amount zero preserves audio");
 for(int target=0;target<4;++target){sawstar::Synth plain,mod;
  for(auto s:{&plain,&mod}){s->Reset(sr);s->SetParameters(0,1,1,1,20);s->SetOutputBoost(18);s->SetFilter(600,0,100);s->Midi(0x90,60,127);}
  mod.SetLfo(3,100,0,target,false,2,120,false);double difference=0;
  for(int i=0;i<sr;++i){auto p=plain.ProcessStereo(),m=mod.ProcessStereo();difference+=std::abs(m.left-p.left)+std::abs(m.right-p.right);check(std::isfinite(m.left)&&std::abs(m.left)<=1,"modulated voice bounds");}
  check(difference>1,"every LFO target changes rendered audio");
 }
 // Independent oscillator selection; only OSC2 is audible here.
 sawstar::Synth c,d;for(auto s:{&c,&d}){s->Reset(sr);s->SetMixer(0,100,0,0,0,-1,0,0);s->SetParameters(0,1,1,1,20);s->Midi(0x90,60,127);}
 c.SetWaveforms(1,3);d.SetWaveforms(2,3);
 for(int i=0;i<sr/4;++i){auto x=c.Process(),y=d.Process();check(std::abs(x-y)<1e-4||i<sr*.15,"silent OSC1 shape does not affect OSC2");}
}}
