// SPDX-License-Identifier: MIT
#pragma once
#include "engine/Synth.h"
#include "midi/Arpeggiator.h"
#include "plugin/State.h"
using namespace sawstar;
struct Rig{
 Synth synth;Arpeggiator arp;
 void init(float sr){synth.Reset(sr);arp.Init(sr);}
 void midi(int status,int a,int b){arp.Midi(status,a,b,[this](int s,int n,int v){synth.Midi(s,n,v);});}
 void apply(const Snapshot& v){auto& s=synth;
  arp.Set(v[83]!=0,int(v[84]),int(v[85]),v[86],int(v[87]),v[88],v[89]!=0,137,true,[this](int st,int n,int vel){synth.Midi(st,n,vel);});
  s.SetLfo2(v[64],v[65],int(v[66]),int(v[67]),v[68]!=0,int(v[69]),137,v[70]!=0);
  for(int row=0;row<4;++row){int id=71+row*3;s.SetModulation(row,int(v[id]),int(v[id+1]),v[id+2]);}
  s.SetVoiceMode(int(v[59]),v[60],v[61]!=0);s.SetParameters(v[0],v[1],v[2],v[3],v[4]);
  s.SetMixer(v[20],v[21],v[22],v[23],int(v[24]),int(v[25]),v[62]==0?int(v[26]):int(v[62])-1,int(v[30]));
  s.SetNoiseColor(v[63]);s.SetSubWave(int(v[92]));s.SetWidth(v[90]!=0,v[91]);s.SetOsc2(v[27],v[28],v[29]);s.SetOutputBoost(v[19]);s.SetSaw(v[5],v[6],v[7]);
  s.SetReverb(v[54]!=0,v[55],v[56],v[57],v[58]);
  s.SetDelay(v[46]!=0,v[47],v[48],v[49],v[50],v[51]!=0,v[52]!=0,int(v[53]),137);
  s.SetChorus(v[42]!=0,v[43],v[44],v[45]);s.SetWaveforms(int(v[33]),int(v[34]));
  s.SetLfo(v[35],v[36],int(v[37]),int(v[38]),v[39]!=0,int(v[40]),137,v[41]!=0);
  s.SetFilterCharacter(v[31],int(v[32]));s.SetFilter(v[8],v[9],v[10]);s.SetFilterEnvelope(v[11],v[12],v[13],v[14],v[15],v[16]);s.SetPerformance(v[17],v[18]);
 }
 StereoSample process(){arp.Process([this](int s,int n,int v){synth.Midi(s,n,v);});return synth.ProcessStereo();}
 void clear(){arp.Clear([this](int s,int n,int v){synth.Midi(s,n,v);});}
};
