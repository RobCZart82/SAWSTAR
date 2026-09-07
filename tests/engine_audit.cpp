// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include "midi/Arpeggiator.h"
#include "plugin/State.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
using namespace sawstar;
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
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
  s.SetNoiseColor(v[63]);s.SetOsc2(v[27],v[28],v[29]);s.SetOutputBoost(v[19]);s.SetSaw(v[5],v[6],v[7]);
  s.SetReverb(v[54]!=0,v[55],v[56],v[57],v[58]);
  s.SetDelay(v[46]!=0,v[47],v[48],v[49],v[50],v[51]!=0,v[52]!=0,int(v[53]),137);
  s.SetChorus(v[42]!=0,v[43],v[44],v[45]);s.SetWaveforms(int(v[33]),int(v[34]));
  s.SetLfo(v[35],v[36],int(v[37]),int(v[38]),v[39]!=0,int(v[40]),137,v[41]!=0);
  s.SetFilterCharacter(v[31],int(v[32]));s.SetFilter(v[8],v[9],v[10]);s.SetFilterEnvelope(v[11],v[12],v[13],v[14],v[15],v[16]);s.SetPerformance(v[17],v[18]);
 }
 StereoSample process(){arp.Process([this](int s,int n,int v){synth.Midi(s,n,v);});return synth.ProcessStereo();}
 void clear(){arp.Clear([this](int s,int n,int v){synth.Midi(s,n,v);});}
};
int main(){
 std::cout<<"scene,sr,peak,rms,mono_rms,dc,render_seconds\n";
 for(float sr:{44100.f,48000.f,96000.f})for(int scene=0;scene<3;++scene){
  auto v=DefaultSnapshot();v[0]=-6;v[1]=scene==1?200:2;v[2]=80;v[3]=scene==2?.2:.8;v[4]=scene==1?2000:100;
  v[19]=18;v[20]=80;v[21]=65;v[22]=20;v[23]=10;v[5]=30;v[6]=80;v[7]=100;v[27]=21;v[28]=75;v[29]=90;
  v[8]=2400;v[9]=65;v[10]=100;v[11]=24;v[31]=9;v[32]=1;v[62]=3;v[63]=30;
  v[35]=3;v[36]=15;v[38]=0;v[64]=.7;v[65]=20;v[67]=3;
  v[71]=1;v[72]=0;v[73]=20;v[74]=2;v[75]=3;v[76]=30;v[77]=3;v[78]=1;v[79]=5;v[80]=5;v[81]=4;v[82]=50;
  v[42]=1;v[43]=20;v[46]=1;v[47]=25;v[49]=60;v[54]=1;v[55]=20;v[57]=3;
  v[83]=scene==2?1:0;v[84]=3;v[85]=3;v[86]=75;v[87]=3;v[88]=45;v[89]=scene==2?1:0;
  auto bytes=EncodeState(v);Snapshot recalled{};check(DecodeState(bytes.data(),bytes.size(),recalled)==bytes.size()&&v==recalled,"complete snapshot recall");
  auto a=std::make_unique<Rig>(),b=std::make_unique<Rig>();a->init(sr);b->init(sr);a->apply(v);b->apply(recalled);
  const int base=scene==0?36:(scene==1?60:84);
  for(int n=0;n<16;++n){a->midi(0x90,base+n,100);b->midi(0x90,base+n,100);}
  double energy=0,mono=0,dc=0;float peak=0;int count=0;
  const auto start=std::chrono::steady_clock::now();
  const int hold=int(sr),total=int(sr*(scene==1?13:3));
  for(int i=0;i<total;++i){
   // Same parameter delivery at 127-sample blocks must not alter the render.
   if(i%127==0)b->apply(recalled);
   if(i==hold/3){a->midi(0xb0,1,100);b->midi(0xb0,1,100);a->midi(0xd0,90,0);b->midi(0xd0,90,0);}
   if(i==hold/2){a->midi(0xe0,0,80);b->midi(0xe0,0,80);}
   if(i==hold){a->clear();b->clear();}
   const auto x=a->process(),y=b->process();
   check(x.left==y.left&&x.right==y.right,"recalled render and block delivery are sample-identical");
   check(std::isfinite(x.left)&&std::isfinite(x.right)&&std::abs(x.left)<=.98001f&&std::abs(x.right)<=.98001f,"combined engine finite and protected");
   peak=std::max(peak,std::max(std::abs(x.left),std::abs(x.right)));
   if(i<hold){energy+=double(x.left)*x.left+double(x.right)*x.right;const double m=(x.left+x.right)*.5;mono+=m*m;dc+=x.left+x.right;++count;}
  }
  check(energy>.001&&mono>.0001,"combined scene audible in stereo and mono");
  check(a->synth.ActiveVoices()==0&&b->synth.ActiveVoices()==0,"long release and held arp clear completely");
  std::cout<<scene<<','<<sr<<','<<peak<<','<<std::sqrt(energy/(2*count))<<','<<std::sqrt(mono/count)<<','<<dc/(2*count)<<','<<std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count()<<'\n';
 }
}
