// SPDX-License-Identifier: MIT
#include "midi/BlockMidiQueue.h"
#include "midi/Arpeggiator.h"
#include "engine/Synth.h"
#include <vector>
#include <memory>
#include <iostream>
#include <cstdlib>
#include <cmath>
using namespace sawstar;
void check(bool ok,const char* text){if(!ok){std::cerr<<text<<'\n';std::exit(1);}}
void boundaries(){
 BlockMidiQueue q;std::vector<int> received;int clock=0,panics=0;
 auto run=[&](int n){q.Process(n,[&](const auto& e){received.push_back(clock*100+e.data1);},[&](int){++clock;},[&]{++panics;});};
 q.Push({8,144,5,100});q.Push({0,144,1,100});q.Push({3,144,2,100});
 q.Push({4,144,3,100});q.Push({4,128,4,0});q.Push({-1,144,6,100});
 run(0);check(q.Size()==6&&received.empty(),"zero block consumes MIDI");
 run(4);check(received==std::vector<int>({1,6,302}),"offset ordering or last sample failed");
 run(1);check(received==std::vector<int>({1,6,302,403,404}),"boundary carry or stable tie failed");
 run(4);check(received.back()==805&&q.Size()==0&&panics==0,"future offset failed");
 q.Push({0,144,1,1});q.Clear();run(1);check(received.size()==6,"reset leaves events");
 for(int i=0;i<1024;++i)q.Push({0,144,i,1});
 received.clear();run(1);check(received.size()==1024&&panics==0,"exact capacity failed");
 for(int i=1;i<1024;++i)check(received[i]-received[i-1]==1,"full queue tie order changed");
 for(int i=0;i<1025;++i)q.Push({0,144,1,1});
 received.clear();run(0);check(panics==1&&q.Size()==0&&received.empty(),"overflow must panic and discard whole burst even on zero block");
 run(1);check(panics==1,"overflow recovery repeats");
 q.Push({0,144,2,1});run(1);check(received.size()==1,"queue does not recover");
}
std::vector<StereoSample> render(int block,float sr,int mode){
 auto s=std::make_unique<Synth>();s->Reset(sr);s->SetParameters(-12,1,10,.7,10);s->SetVoiceMode(mode,0,true);
 Arpeggiator arp;arp.Init(sr);BlockMidiQueue q;
 const BlockMidiEvent events[]={{0,144,60,100},{31,145,67,90},{32,144,64,80},
 {63,176,64,127},{64,128,60,0},{64,144,60,100},{65,128,60,0},
 {127,128,64,0},{128,176,64,0},{129,177,123,0},
 {255,145,67,100},{256,145,67,100},{256,129,67,0},{512,129,67,0},
 {1000,144,72,100},{1001,176,120,0}};
 for(auto e:events)q.Push(e);
 std::vector<StereoSample> result;result.reserve(12000);
 int cycle=0;
 for(int start=0;start<12000;){
  const int sizes[]={1,32,64,512,2048,7};
  const int n=std::min(block?block:sizes[cycle++%6],12000-start);
  q.Process(n,[&](auto e){arp.Midi(e.status,e.data1,e.data2,[&](int a,int b,int c){s->Midi(a,b,c);});},
   [&](int){arp.Process([&](int a,int b,int c){s->Midi(a,b,c);});result.push_back(s->ProcessStereo());},
   [&]{RecoverMidiOverflow(arp,*s);});
  start+=n;
 }
 check(s->ActiveVoices()==0,"timeline leaves stuck voice");return result;
}
void recovery(){
 auto s=std::make_unique<Synth>();s->Reset(48000);s->SetParameters(-12,1,1,1,20);
 Arpeggiator arp;arp.Init(48000);auto send=[&](int a,int b,int c){s->Midi(a,b,c);};
 arp.Set(true,0,2,70,1,0,true,120,true,send);
 for(int ch=0;ch<16;++ch){arp.Midi(144|ch,60,100,send);arp.Midi(176|ch,64,127,send);}
 s->Midi(224,0,96);s->Midi(176,1,90);
 for(int i=0;i<100;++i){arp.Process(send);s->ProcessStereo();}
 BlockMidiQueue q;for(int i=0;i<1025;++i)q.Push({0,144,60,100});
 q.Process(0,[&](auto){check(false,"overflow delivered partial burst");},[](int){},[&]{RecoverMidiOverflow(arp,*s);});
 check(s->PitchBend(0)==12288&&s->ModWheel(0)==90,"overflow unexpectedly resets controllers");
 for(int i=0;i<3000;++i){arp.Process(send);auto x=s->ProcessStereo();check(x.left==0&&x.right==0,"panic or ARP hold leaves sound");}
 check(s->ActiveVoices()==0,"overflow stuck voice");
 for(int n=0;n<128;++n)check(!s->Held(n),"overflow stuck held key");
 // New notes must work after recovery.
 arp.Midi(144,60,100,send);double energy=0;
 for(int i=0;i<1000;++i){arp.Process(send);auto x=s->ProcessStereo();energy+=x.left*x.left;}
 check(energy>0,"overflow prevents new notes");
}
int main(){boundaries();recovery();
 for(float sr:{44100.f,48000.f,96000.f})for(int mode:{0,1,2}){
  auto ref=render(1,sr,mode);
  for(int block:{0,32,64,512,2048}){auto out=render(block,sr,mode);
   for(std::size_t i=0;i<ref.size();++i)check(out[i].left==ref[i].left&&out[i].right==ref[i].right,"block size changes MIDI render");}
 }
 float l[2]{},r[2]{};float* channels[]={l,r};StereoSample v{.5f,-.25f};
 WriteHostOutput(v,channels,1,0);check(l[0]==.125f,"mono routing");
 WriteHostOutput(v,channels,2,1);check(l[1]==.5f&&r[1]==-.25f,"stereo routing");
 WriteHostOutput(v,static_cast<float**>(nullptr),0,0);
 std::cout<<"Production MIDI queue, recovery, routing and block equivalence passed\n";
}
