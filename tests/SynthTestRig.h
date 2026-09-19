// SPDX-License-Identifier: MIT
#pragma once
#include "engine/Synth.h"
#include "midi/Arpeggiator.h"
#include "plugin/State.h"
#include "plugin/EngineControls.h"
using namespace sawstar;
struct Rig{
 Synth synth;Arpeggiator arp;
 void init(float sr){synth.Reset(sr);arp.Init(sr);}
 void midi(int status,int a,int b){arp.Midi(status,a,b,[this](int s,int n,int v){synth.Midi(s,n,v);});}
 void apply(const Snapshot& v){ApplyEngineControls(synth,arp,
  [&](ParameterId id){return v[static_cast<size_t>(id)];},
  [&](ParameterId id){return int(v[static_cast<size_t>(id)]);},137,true);}
 StereoSample process(){arp.Process([this](int s,int n,int v){synth.Midi(s,n,v);});return synth.ProcessStereo();}
 void clear(){arp.Clear([this](int s,int n,int v){synth.Midi(s,n,v);});}
};
