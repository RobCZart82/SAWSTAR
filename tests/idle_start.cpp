// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <stdexcept>
#include <cmath>
#include <iostream>
int main(){
 sawstar::LowPass filter;filter.Init(48000);filter.Set(100,0,100);filter.SnapToTargets();
 if(std::abs(filter.Process({1,1}).left)>.001f)throw std::runtime_error("Cold filter leaked dry attack");
 sawstar::SevenSaw osc;osc.Init(48000);osc.SetWaveform(3);osc.SetShape(0,0,0);osc.SetFreq(440);osc.SnapToTargets();
 if(std::abs(osc.Process().left)>.0001f)throw std::runtime_error("Sine started as saw");
 sawstar::Synth immediate,waited;immediate.Reset(48000);waited.Reset(48000);
 for(auto* s:{&immediate,&waited}){s->SetParameters(-12,1,50,.7,20);s->SetWaveforms(3,3);s->SetFilter(200,0,100);}
 for(int i=0;i<48000;++i)waited.ProcessStereo();
 immediate.Midi(0x90,60,100);waited.Midi(0x90,60,100);
 for(int i=0;i<480;++i){auto a=immediate.ProcessStereo(),b=waited.ProcessStereo();if(std::abs(a.left-b.left)>1e-6)throw std::runtime_error("Silent wait changed first attack");}
 std::cout<<"Idle filter, waveform and reset attack passed\n";
}
