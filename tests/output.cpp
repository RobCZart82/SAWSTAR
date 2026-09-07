// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
int main(){
 for(float sr:{44100.f,48000.f,96000.f}) {
  sawstar::Synth old,fresh;
  for(auto* s:{&old,&fresh}){s->Reset(sr);s->SetParameters(0,1,1,1,20);s->Midi(0x90,69,127);}
  fresh.SetOutputBoost(18);
  double base=0,raised=0;float peak=0;
  for(int i=0;i<sr;++i){auto a=old.ProcessStereo(),b=fresh.ProcessStereo();
   if(i>sr/2){base+=a.left*a.left;raised+=b.left*b.left;peak=std::max(peak,std::abs(b.left));}}
  const double increase=10*std::log10(raised/base);
  check(std::abs(increase-18)<.02,"single note must gain 18 dB without limiting");
  check(peak>.45f&&peak<.51f,"full velocity single note near -6 dBFS at maximum Output");
  std::cout<<sr<<" Hz: single-note gain "<<increase<<" dB, peak "<<20*std::log10(peak)<<" dBFS\n";
  // Deliberately coherent voices on different MIDI channels stress peak protection.
  sawstar::Synth chord;chord.Reset(sr);chord.SetParameters(0,1,1,1,20);chord.SetOutputBoost(24);
  for(int ch=0;ch<16;++ch)chord.Midi(0x90|ch,60,127);
  float chordPeak=0;
  for(int i=0;i<sr;++i){auto x=chord.ProcessStereo();
   check(std::isfinite(x.left)&&std::abs(x.left)<=.98001f&&std::abs(x.right)<=.98001f,"protected 16-voice peak");
   check(x.left==x.right,"linked protection preserves centered image");
   chordPeak=std::max(chordPeak,std::abs(x.left));}
  check(chordPeak>.9f,"stress signal exercises protection");
  std::cout<<"16 coherent voices peak "<<20*std::log10(chordPeak)<<" dBFS\n";
  for(int ch=0;ch<16;++ch)chord.Midi(0xB0|ch,120,0);
  check(chord.Process()==0,"panic remains silent");
  // Matrix-only pan/amp can exceed unity even with dry FX and zero boost.
  for(int target:{2,3}) {
    sawstar::Synth routed;routed.Reset(sr);routed.SetParameters(0,1,1,1,20);
    routed.SetModulation(0,4,target,100);
    for(int ch=0;ch<16;++ch)routed.Midi(0x90|ch,60,127);
    float maximum=0;
    for(int i=0;i<sr/4;++i){auto x=routed.ProcessStereo();
      maximum=std::max(maximum,std::max(std::abs(x.left),std::abs(x.right)));
      check(std::isfinite(x.left)&&std::isfinite(x.right)&&maximum<=.98001f,"matrix-only routing uses peak guard");}
    check(maximum>.9f,"matrix guard test reaches protection threshold");
  }
  // Saturation must not leave a sustained DC offset consuming headroom.
  sawstar::Synth driven;driven.Reset(sr);driven.SetParameters(0,1,1,1,20);
  driven.SetFilter(1800,30,100);driven.SetFilterCharacter(12,1);driven.SetOutputBoost(18);
  driven.Midi(0x90,69,127);double dc=0,power=0;
  for(int i=0;i<static_cast<int>(sr)*2;++i){const auto x=driven.ProcessStereo();if(i>=sr){dc+=x.left;power+=x.left*x.left;}}
  check(std::abs(dc/sr)<.001,"drive DC rejection");
  check(power/sr>.001,"DC test retains audible signal");
  // Stereo unison + resonant filtering + gain automation must stay finite/bounded.
  chord.SetSaw(50,100,100);chord.SetFilter(800,100,100);
  for(int n=48;n<64;++n)chord.Midi(0x90,n,127);
  for(int i=0;i<sr;++i){if(i%512==0)chord.SetOutputBoost((i/512)%2?18:24);
   auto x=chord.ProcessStereo();check(std::isfinite(x.left)&&std::isfinite(x.right)&&std::abs(x.left)<=.98001f&&std::abs(x.right)<=.98001f,"stereo stress and automation");}
  chord.Reset(sr);check(chord.Process()==0,"reset clears output");
 }
}
