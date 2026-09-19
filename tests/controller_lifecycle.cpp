// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>

void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
void render(sawstar::Synth& s,int count){for(int i=0;i<count;++i){auto v=s.ProcessStereo();check(std::isfinite(v.left)&&std::isfinite(v.right)&&std::abs(v.left)<=1&&std::abs(v.right)<=1,"invalid lifecycle output");}}
int main(){for(float sr:{44100.f,48000.f,96000.f})for(int mode:{0,1,2}){
 auto s=std::make_unique<sawstar::Synth>();s->Reset(sr);s->SetVoiceMode(mode,0,true);
 s->SetParameters(-12,10,100,.7,20);
 s->Midi(0xe0,127,127);s->Midi(0xb0,1,127);s->Midi(0xd0,100,0);
 s->Midi(0xb0,64,127);s->Midi(0x90,60,100);render(*s,1000);s->Midi(0x80,60,0);
 render(*s,100);check(s->ActiveVoices()>0,"pedal did not hold note");
 s->Midi(0xb0,121,0);
 check(s->PitchBend(0)==8192&&s->ModWheel(0)==0,"CC121 raw controllers not reset");
 render(*s,int(sr));check(s->ActiveVoices()==0,"CC121 left pedal voice stuck");
 // A new phrase remains usable after reset; live A/D/R changes do not strand voices.
 s->Midi(0x90,64,100);
 for(int step=0;step<100;++step){s->SetParameters(-12,step%2?1:1000,step%2?1:1000,.7,step%2?1:1000);render(*s,32);}
 s->SetParameters(-12,10,100,.7,20);s->Midi(0x80,64,0);render(*s,int(sr));
 check(s->ActiveVoices()==0,"envelope automation left note stuck");
 s->Midi(0xb0,1,90);s->Midi(0xe0,0,80);s->Midi(0x90,60,100);render(*s,1000);
 for(int channel=0;channel<16;++channel)s->Midi(0xb0|channel,120,0);
 render(*s,100);check(s->ActiveVoices()==0,"all-channel panic left voice active");
 check(s->PitchBend(0)==10240&&s->ModWheel(0)==90,"CC120 changed controller policy");
 s->Midi(0x90,60,100);render(*s,1000);check(s->ActiveVoices()>0,"panic prevented new note");
}}
