// SPDX-License-Identifier: MIT
#include "dsp/LowPass.h"
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
void Require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
sawstar::StereoSample Tone(double phase){
  constexpr double pi=3.14159265358979323846;
  return {float(.7*std::sin(2*pi*phase)+.1*std::cos(6*pi*phase)),float(-.8*std::sin(2*pi*phase))};
}
}
int main(){
  // Independent reference: actually settle a filter for four seconds with a
  // known two-harmonic signal. No reuse of the implementation's state formulas.
  double worst=0;
  for(float sr:{44100.f,96000.f,192000.f})for(float hz:{20.f,100.f,3000.f})
  for(int mode:{0,1,2,3})for(float res:{0.f,100.f}){
    constexpr int period=73;std::array<sawstar::StereoSample,period> input{};
    sawstar::StereoSample preview[sawstar::LowPass::kStartPreviewSamples];
    for(int n=0;n<period;++n)input[n]=Tone(double(n)/period);
    for(int n=0;n<64;++n)preview[n]=Tone(double(n)/64);
    sawstar::LowPass prime,reference;
    for(auto* f:{&prime,&reference}){f->Init(sr);f->Set(hz,res,100);f->SetCharacter(0,mode);f->SnapToTargets();}
    Require(prime.PrepareStart(preview,sr/period),"Valid harmonic initialization failed");
    const int rounds=int(std::ceil(sr*4/period));
    for(int r=0;r<rounds;++r)for(auto x:input)reference.Process(x);
    for(int r=0;r<3;++r)for(auto x:input){auto a=prime.Process(x),b=reference.Process(x);
      double error=std::max(std::abs(a.left-b.left),std::abs(a.right-b.right));worst=std::max(worst,error);
      Require(error<2e-5,"Harmonic state differs from independently settled filter");}
  }

  // Fixed-shape rendering must not change a snapped oscillator's samples or
  // leave different continuation history (including its triangle integrator).
  for(int wave=0;wave<4;++wave)for(float mix:{0.f,.6f,1.f}){
    sawstar::SevenSaw normal;normal.Init(96000);normal.SetWaveform(wave);normal.SetFreq(932.3275f);
    normal.SetShape(37,mix,.83f);normal.SetPitchMultiplier(1.3f);normal.SnapToTargets();auto preview=normal;
    Require(normal.CanPreviewStart()==(wave!=2),"Nonstationary triangle was accepted for extrapolation");
    sawstar::StereoSample samples[64];preview.RenderStartPreview(samples,64);
    for(auto x:samples){auto y=normal.Process();Require(x.left==y.left&&x.right==y.right,"Preview changed source samples");}
    for(int n=0;n<128;++n){auto x=preview.Process(),y=normal.Process();Require(x.left==y.left&&x.right==y.right,"Preview changed continuation history");}
  }

  // Do not extrapolate near the cutoff, dry bypass, HP/BP or Nyquist clamps.
  sawstar::LowPass f;f.Init(48000);f.Set(100,60,100);f.SnapToTargets();
  Require(f.WantsPreparedStart(932)&&!f.WantsPreparedStart(120),"Incorrect priming frequency region");
  for(int mode:{2,3}){f.SetCharacter(0,mode);Require(!f.WantsPreparedStart(932),"HP/BP priming must stay disabled");}
  f.SetCharacter(0,1);f.Set(100,60,0);Require(!f.WantsPreparedStart(932),"Dry bypass must not prepare a start");
  sawstar::SevenSaw high;high.Init(8000);high.SetFreq(20000);high.SnapToTargets();
  Require(!high.CanPreviewStart(),"Nyquist-clamped source must not be extrapolated");

  // Invalid estimates are rejected without corrupting existing filter history.
  f.Set(100,60,100);f.SnapToTargets();f.Process({.4f,-.2f});auto unchanged=f;
  sawstar::StereoSample invalid[64]{};invalid[3].left=std::numeric_limits<float>::quiet_NaN();
  Require(!f.PrepareStart(invalid,932),"Non-finite preview accepted");
  for(int n=0;n<100;++n){auto a=f.Process({.1f,.2f}),b=unchanged.Process({.1f,.2f});Require(a.left==b.left&&a.right==b.right,"Rejected preview changed filter state");}
  std::cout<<"Filter start: 72 independent steady-state cases, preview continuity and guards passed; worst error "<<worst<<"\n";
}
