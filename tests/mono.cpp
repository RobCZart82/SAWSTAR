// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
using namespace sawstar;
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
void wait(Synth& s,int frames){for(int i=0;i<frames;++i){auto x=s.ProcessStereo();check(std::isfinite(x.left)&&std::isfinite(x.right)&&std::abs(x.left)<=1&&std::abs(x.right)<=1,"finite output");}}
float frequency(Synth& s,float sr){int crossings=0;float last=s.Process();int frames=static_cast<int>(sr*.1f);for(int i=0;i<frames;++i){float x=s.Process();if(last<=0&&x>0)++crossings;last=x;}return crossings*sr/frames;}
void setup(Synth& s,float sr,int mode=2,float glide=0,bool overlap=true){s.Reset(sr);s.SetParameters(-6,1,10,1,10);s.SetWaveforms(3,3);s.SetVoiceMode(mode,glide,overlap);}
int main(){
 for(float sr:{44100.f,48000.f,96000.f}){
  Synth s;setup(s,sr);s.Midi(0x90,69,127);wait(s,sr*.05f);check(std::abs(frequency(s,sr)-440)<12,"first mono pitch");
  s.Midi(0x90,81,100);wait(s,sr*.02f);check(s.ActiveVoices()==1&&s.Held(69)&&s.Held(81),"one voice and full held keys");check(std::abs(frequency(s,sr)-880)<12,"last key priority");
  s.Midi(0x80,81,0);wait(s,sr*.02f);check(std::abs(frequency(s,sr)-440)<12&&!s.Held(81),"fallback key");
  s.Midi(0x91,81,127);wait(s,sr*.02f);s.Midi(0x80,69,0);check(s.Held(81),"other channel note off isolated");s.Midi(0x81,81,0);wait(s,sr);check(s.ActiveVoices()==0,"last note release");
  setup(s,sr);s.Midi(0x90,69,127);s.Midi(0xb0,64,127);s.Midi(0x90,69,0);wait(s,sr*.1f);check(s.ActiveVoices()==1&&!s.Held(69),"pedal holds without physical key");s.Midi(0xb0,64,0);wait(s,sr);check(s.ActiveVoices()==0,"pedal release");
  s.Midi(0x90,69,127);s.Midi(0x90,69,127);s.Midi(0x80,69,0);wait(s,sr);check(s.ActiveVoices()==1&&s.Held(69),"overlapping note survives first release");s.Midi(0x80,69,0);wait(s,sr);check(s.ActiveVoices()==0&&!s.Held(69),"repeated note releases after matching offs");
  s.Midi(0x90,69,127);s.Midi(0xb0,64,127);s.Midi(0x80,69,0);s.Midi(0xb0,121,0);wait(s,sr);check(s.ActiveVoices()==0,"reset controllers releases pedal");
  s.Midi(0x90,69,127);s.Midi(0x90,81,127);s.Midi(0xb0,123,0);wait(s,sr);check(s.ActiveVoices()==0&&!s.Held(69)&&!s.Held(81),"all notes off clears stack");
  s.Midi(0x90,69,127);s.Midi(0xb0,120,0);check(s.ActiveVoices()==0&&s.Process()==0,"panic silent");
  setup(s,sr,2,1000);s.Midi(0x90,69,127);wait(s,sr*.05f);s.Midi(0x90,81,127);wait(s,sr*.45f);float mid=frequency(s,sr);check(mid>590&&mid<660,"glide passes octave midpoint");wait(s,sr*.6f);check(std::abs(frequency(s,sr)-880)<12,"glide reaches target");
  s.Midi(0x80,81,0);wait(s,sr*1.1f);check(std::abs(frequency(s,sr)-440)<12,"glide fallback target");
  s.Midi(0x80,69,0);wait(s,sr);s.Midi(0x90,81,127);wait(s,sr*.03f);check(std::abs(frequency(s,sr)-880)<12,"overlap-only separated note snaps");
  s.SetVoiceMode(2,1000,false);s.Midi(0x80,81,0);wait(s,sr);s.Midi(0x90,69,127);wait(s,sr*.45f);mid=frequency(s,sr);check(mid>590&&mid<660,"always glide separated note");
  s.SetVoiceMode(2,0,false);wait(s,sr*.02f);check(std::abs(frequency(s,sr)-440)<12,"zero glide completes immediately");
  s.Midi(0xe0,127,127);wait(s,sr*.1f);check(std::abs(frequency(s,sr)-440*std::exp2(2./12))<12,"bend combines with mono");
  setup(s,sr,0);s.Midi(0x90,60,100);s.SetVoiceMode(2,0,true);wait(s,sr);check(s.ActiveVoices()==0,"mode switch before first sample releases");
  // Mode transitions release their old phrase and never revive discarded keys.
  s.SetVoiceMode(0,0,true);wait(s,sr);check(!s.Held(69),"mode transition clears held state");s.Midi(0x90,60,100);s.Midi(0x90,64,100);check(s.ActiveVoices()==2,"poly restored");s.SetVoiceMode(1,100,true);s.Midi(0x90,67,100);check(s.ActiveVoices()==1&&!s.Held(60),"poly to mono single source");s.Midi(0x80,67,0);wait(s,sr);check(s.ActiveVoices()==0,"transition release");
 }
 // Envelope retrigger distinction: attack restarts from sustain in Mono only.
 Synth mono,legato;
 for(auto s:{&mono,&legato}){setup(*s,48000);s->SetParameters(-6,200,100,.1,30);s->Midi(0x90,69,127);wait(*s,96000);}
 mono.SetVoiceMode(1,0,true);mono.Midi(0x90,69,127);wait(mono,96000);
 mono.Midi(0x90,81,127);legato.Midi(0x90,81,127);double m=0,l=0;
 for(int i=0;i<12000;++i){auto a=mono.Process(),b=legato.Process();m+=a*a;l+=b*b;}
 check(m>l*4,"mono attacks again while legato sustains");
 // New controls in Poly must not affect old sound.
 Synth a,b;setup(a,48000,0);setup(b,48000,0,2000,false);a.Midi(0x90,60,100);b.Midi(0x90,60,100);
 for(int i=0;i<48000;++i){auto x=a.ProcessStereo(),y=b.ProcessStereo();check(x.left==y.left&&x.right==y.right,"poly identity");}
}
