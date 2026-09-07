// SPDX-License-Identifier: MIT
#include "dsp/Effects/Reverb.h"
#include "engine/Synth.h"
#include "presets/FactoryPresets.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
using namespace sawstar;
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
int main(){
 for(float sr:{44100.f,48000.f,96000.f,192000.f}){
  Reverb dry,shortRoom,longRoom;dry.Init(sr);shortRoom.Init(sr);longRoom.Init(sr);
  shortRoom.Set(true,100,50,.3f,6000);longRoom.Set(true,100,50,3,6000);
  for(int i=0;i<sr;++i){shortRoom.Process({0,0});longRoom.Process({0,0});}
  double lateShort=0,lateLong=0,stereo=0,energy=0;
  for(int i=0;i<sr*2;++i){const StereoSample in=i==0?StereoSample{1,0}:StereoSample{};
   auto a=dry.Process(in),b=shortRoom.Process(in),c=longRoom.Process(in);
   check(a.left==in.left&&a.right==in.right,"default bypass exact");
   check(std::isfinite(c.left)&&std::isfinite(c.right)&&std::abs(c.left)<2&&std::abs(c.right)<2,"impulse finite bounded");
   energy+=c.left*c.left+c.right*c.right;stereo+=std::abs(c.left-c.right);
   if(i>sr*.7){lateShort+=b.left*b.left+b.right*b.right;lateLong+=c.left*c.left+c.right*c.right;}
  }
  check(energy>.01&&stereo>.01,"stereo reverb responds");check(lateLong>lateShort*100&&lateLong>1.e-6,"decay controls tail duration");
  longRoom.Set(false,100,100,10,500);for(int i=0;i<sr;++i)longRoom.Process({0,0});
  auto a=longRoom.Process({.2f,-.3f});check(longRoom.IsDry()&&a.left==.2f&&a.right==-.3f,"disable settles dry");
  longRoom.Set(true,100,100,10,500);for(int i=0;i<sr*.2;++i){auto b=longRoom.Process({0,0});check(b.left==0&&b.right==0,"old tail cannot return");}
  longRoom.Init(sr);longRoom.Set(true,0,100,10,500);a=longRoom.Process({.2f,-.3f});check(a.left==.2f&&a.right==-.3f,"zero mix dry");
  for(int i=0;i<sr;++i){if(i%997==0)longRoom.Set(true,100,i%2?0:100,i%3?.2f:10,i%2?500:16000);
   const float x=.1f*std::sin(i*.1f);a=longRoom.Process({x,-x});check(std::isfinite(a.left)&&std::isfinite(a.right)&&std::abs(a.left)<4&&std::abs(a.right)<4,"automation bounded");}
  longRoom.Init(sr);a=longRoom.Process({0,0});check(a.left==0&&a.right==0,"reset clears memory");
  longRoom.Set(true,NAN,NAN,NAN,NAN);a=longRoom.Process({0,0});check(std::isfinite(a.left),"settings sanitized");
 }
 Reverb bright,dark,small,large;for(auto p:{&bright,&dark,&small,&large})p->Init(48000);
 bright.Set(true,100,50,2,16000);dark.Set(true,100,50,2,500);small.Set(true,100,0,2,6000);large.Set(true,100,100,2,6000);
 for(int i=0;i<48000;++i)for(auto p:{&bright,&dark,&small,&large})p->Process({0,0});
 double b=0,d=0;int firstSmall=-1,firstLarge=-1;
 for(int i=0;i<96000;++i){float x=.1f*std::sin(i*6.2831853f*8000/48000);auto y=bright.Process({x,x}),z=dark.Process({x,x});
  auto s=small.Process(i==0?StereoSample{1,0}:StereoSample{}),l=large.Process(i==0?StereoSample{1,0}:StereoSample{});
  if(i>48000){b+=y.left*y.left+y.right*y.right;d+=z.left*z.left+z.right*z.right;}
  if(i>1&&firstSmall<0&&std::abs(s.left)>.001)firstSmall=i;if(i>1&&firstLarge<0&&std::abs(l.left)>.001)firstLarge=i;
 }
 check(d<b*.05,"damping darkens tail");check(firstSmall>0&&firstLarge>firstSmall*2,"size changes reflection timing");
 Synth drySynth,off,wet;
 const auto& v=FactoryPresets()[7].values;
 for(auto s:{&drySynth,&off,&wet}){s->Reset(48000);s->SetParameters(v[0],v[1],v[2],v[3],v[4]);s->SetSaw(v[5],v[6],v[7]);s->SetFilter(v[8],v[9],v[10]);s->SetFilterEnvelope(v[11],v[12],v[13],v[14],v[15],v[16]);s->SetOutputBoost(v[19]);s->SetMixer(v[20],v[21],v[22],v[23],v[24],v[25],v[26],v[30]);s->SetOsc2(v[27],v[28],v[29]);s->SetFilterCharacter(v[31],v[32]);s->SetWaveforms(v[33],v[34]);s->SetLfo(v[35],v[36],v[37],v[38],false,2,120,false);for(int n:{48,55,60,64,67})s->Midi(0x90,n,100);}
 off.SetReverb(false,100,100,10,500);wet.SetReverb(true,20,50,2.5f,6000);
 double difference=0;for(int i=0;i<240000;++i){auto a=drySynth.ProcessStereo(),b=off.ProcessStereo(),c=wet.ProcessStereo();check(a.left==b.left&&a.right==b.right,"Soft Pad One dry identity");check(std::isfinite(c.left)&&std::abs(c.left)<=1&&std::abs(c.right)<=1,"wet pad bounded");difference+=std::abs(a.left-c.left);}
 check(difference>1,"pad reverb audible");
}
