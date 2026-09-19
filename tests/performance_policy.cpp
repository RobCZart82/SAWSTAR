// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
void render(sawstar::Synth& s,int n){for(int i=0;i<n;++i){auto x=s.ProcessStereo();check(std::isfinite(x.left)&&std::isfinite(x.right)&&std::abs(x.left)<=1&&std::abs(x.right)<=1,"invalid mode-change output");}}
int main(){for(float sr:{44100.f,48000.f,96000.f}){
 // Changing mode ends the old phrase and must allow a fresh phrase and release.
 for(int from=0;from<3;++from)for(int to=0;to<3;++to){if(from==to)continue;
  auto s=std::make_unique<sawstar::Synth>();s->Reset(sr);s->SetVoiceMode(from,15,true);s->SetParameters(-12,5,20,.7,20);
  for(int note:{48,60,67})s->Midi(0x90,note,100);render(*s,int(sr/4));
  s->SetVoiceMode(to,15,true);render(*s,int(sr));check(s->ActiveVoices()==0,"mode change left old phrase active");
  // Hosts may still deliver releases for keys held before the mode change.
  for(int note:{48,60,67})s->Midi(0x80,note,0);
  s->Midi(0x90,72,100);render(*s,1000);check(s->ActiveVoices()>0,"mode change prevented new phrase");
  s->Midi(0x80,72,0);render(*s,int(sr));check(s->ActiveVoices()==0,"new phrase did not release");
 }
 // Preserve and explicitly cover the existing first-key LFO event-order policy.
 for(bool retrigger:{false,true}){
  auto a=std::make_unique<sawstar::Synth>(),b=std::make_unique<sawstar::Synth>();
  for(auto* s:{a.get(),b.get()}){s->Reset(sr);s->SetParameters(-12,5,20,1,20);s->SetVoiceMode(1,0,true);s->SetLfo(1,100,0,2,false,0,120,retrigger);s->Midi(0x90,60,100);render(*s,int(sr/4));}
  a->Midi(0x80,60,0);a->Midi(0x90,60,100);
  b->Midi(0x90,60,100);b->Midi(0x80,60,0);
  double diff=0;for(int i=0;i<int(sr/20);++i){auto x=a->ProcessStereo(),y=b->ProcessStereo();check(std::isfinite(x.left)&&std::isfinite(y.left),"invalid LFO output");diff+=std::abs(x.left-y.left)+std::abs(x.right-y.right);}
  check(retrigger?diff>0.01:diff==0,"LFO first-key ordering policy changed");
  for(auto* s:{a.get(),b.get()}){s->Midi(0x80,60,0);render(*s,int(sr));check(s->ActiveVoices()==0,"repeated note stuck");}
 }
}}
