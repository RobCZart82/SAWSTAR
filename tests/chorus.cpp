// SPDX-License-Identifier: MIT
#include "dsp/Effects/Chorus.h"
#include "engine/Synth.h"
#include "presets/FactoryPresets.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
using namespace sawstar;
void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
void Configure(Synth& s,float sr){
 const auto& v=FactoryPresets()[7].values;s.Reset(sr);
 s.SetParameters(v[0],v[1],v[2],v[3],v[4]);s.SetSaw(v[5],v[6],v[7]);
 s.SetFilter(v[8],v[9],v[10]);s.SetFilterEnvelope(v[11],v[12],v[13],v[14],v[15],v[16]);
 s.SetPerformance(v[17],v[18]);s.SetOutputBoost(v[19]);s.SetMixer(v[20],v[21],v[22],v[23],v[24],v[25],v[26],v[30]);
 s.SetOsc2(v[27],v[28],v[29]);s.SetFilterCharacter(v[31],v[32]);s.SetWaveforms(v[33],v[34]);
 s.SetLfo(v[35],v[36],v[37],v[38],v[39]!=0,v[40],120,v[41]!=0);
}
namespace sawstar {
struct ChorusReadRegression {
 static void Run(){
  auto storage=std::make_unique<Chorus>();auto& c=*storage;
  c.Init(48000);c.write_=576;c.buffer_[0].fill(.25f);c.buffer_[1].fill(-.75f);
  const float ms=std::nextafter(12.f,std::numeric_limits<float>::infinity());
  const float wrapped=static_cast<float>(c.write_)-ms*.001f*48000+c.kSize;
  check(wrapped==c.kSize,"fixture rounds a negative read position to the ring size");
  check(c.Read(0,ms)==.25f&&c.Read(1,ms)==-.75f,"rounded wrap must read index zero in the same channel");
  // Preserve the exact interpolation at every ordinary write position.
  for(float sr:{44100.f,48000.f,96000.f,192000.f}){
   c.Init(sr);
   for(size_t i=0;i<c.kSize;++i){c.buffer_[0][i]=float(i%113)*.001f;c.buffer_[1][i]=-float(i%127)*.002f;}
   for(size_t w=0;w<c.kSize;++w)for(float milliseconds:{8.f,12.f,17.f,21.f}){
    c.write_=w;const float delay=std::min(milliseconds*.001f*sr,float(c.kSize-2));
    float pos=float(w)-delay;if(pos<0)pos+=c.kSize;
    const auto i=static_cast<size_t>(pos);if(i>=c.kSize)continue;
    const float f=pos-float(i);
    for(size_t ch=0;ch<2;++ch){const float expected=c.buffer_[ch][i]*(1-f)+c.buffer_[ch][(i+1)%c.kSize]*f;
     check(c.Read(ch,milliseconds)==expected,"ordinary chorus interpolation remains bit-identical");}
   }
  }
  // Real Velvet Orbit chorus timeline reaches the boundary around 12.828 s
  // on the observed platforms; channel-constant inputs expose cross-channel reads.
  c.Init(44100);c.Set(true,18,.2f,30);c.buffer_[0].fill(.25f);c.buffer_[1].fill(-.75f);
  for(int i=0;i<44100*13;++i){const auto y=c.Process({.25f,-.75f});
   check(std::isfinite(y.left)&&std::isfinite(y.right)&&std::abs(y.left-.25f)<1.e-6f&&std::abs(y.right+.75f)<1.e-6f,
         "Velvet Orbit chorus wrap retains channel boundaries");}
 }
};
}
int main(){
 ChorusReadRegression::Run();
 for(float sr:{44100.f,48000.f,96000.f,192000.f}){
  Chorus c;c.Init(sr);
  double stereo=0,difference=0;
  for(int i=0;i<sr;++i){float v=.4f*std::sin(i*440.f*6.2831853f/sr);auto y=c.Process({v,-v});check(y.left==v&&y.right==-v,"initial bypass exact");}
  c.Set(true,0,3,100);
  for(int i=0;i<1000;++i){auto y=c.Process({.25f,-.5f});check(y.left==.25f&&y.right==-.5f,"zero mix exact");}
  c.Set(true,35,.3f,45);
  for(int i=0;i<sr*2;++i){float v=.4f*std::sin(i*220.f*6.2831853f/sr);auto y=c.Process({v,v});check(std::isfinite(y.left)&&std::abs(y.left)<=.5&&std::abs(y.right)<=.5,"chorus bounded");stereo+=std::abs(y.left-y.right);difference+=std::abs(y.left-v);}
  check(stereo/sr>.01&&difference/sr>.01,"chorus audible stereo motion");
  // Dry transitions return to exact passthrough; no feedback can sustain a tail.
  c.Set(false,100,3,100);for(int i=0;i<sr;++i)c.Process({0,0});
  auto y=c.Process({.123f,-.456f});check(c.IsDry()&&y.left==.123f&&y.right==-.456f,"bypass settles exactly");
  c.Set(true,100,3,100);for(int i=0;i<sr;++i)c.Process({0,0});
  y=c.Process({0,0});check(y.left==0&&y.right==0,"finite silent tail");
  // Extremes and rapid automation stay within the input peak (convex interpolation).
  for(int i=0;i<sr;++i){if(i%97==0)c.Set(i%2, i%3?100:0,i%2?3:.05f,i%3?100:0);auto z=c.Process({.5f,-.5f});check(std::isfinite(z.left)&&std::abs(z.left)<=.501f&&std::abs(z.right)<=.501f,"automation bounded");}
  c.Init(sr);c.Set(true,50,.3f,35);y=c.Process({0,0});check(y.left==0&&y.right==0,"reset clears old audio");
  c.Set(true,std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),-100);y=c.Process({0,0});check(std::isfinite(y.left),"invalid settings sanitized");
 }
 for(float sr:{44100.f,48000.f,96000.f}){
  Synth dry,off,wet;Configure(dry,sr);Configure(off,sr);Configure(wet,sr);
  off.SetChorus(false,100,3,100);wet.SetChorus(true,25,.3f,35);
  for(int n:{48,55,60,64,67}){dry.Midi(0x90,n,110);off.Midi(0x90,n,110);wet.Midi(0x90,n,110);}
  double diff=0,peak=0;
  for(int i=0;i<sr*5;++i){auto a=dry.ProcessStereo(),b=off.ProcessStereo(),c=wet.ProcessStereo();check(a.left==b.left&&a.right==b.right,"old Soft Pad One unchanged when off");check(std::isfinite(c.left)&&std::isfinite(c.right),"pad chorus finite");diff+=std::abs(a.left-c.left);peak=std::max(peak,static_cast<double>(std::max(std::abs(c.left),std::abs(c.right))));}
  check(diff/sr>.01&&peak<.98,"pad chorus audible with headroom");
  std::cout<<"Soft Pad One chorus @ "<<sr<<" Hz peak "<<20*std::log10(peak)<<" dBFS\n";
 }
}
