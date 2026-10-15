// SPDX-License-Identifier: MIT
#include "PremiumPreviewSynth.h"
#include "UncalibratedPreviewSynth.h"
#include "MeteredPreviewSynth.h"
#include "plugin/EngineControls.h"
#include "presets/FactoryPresets.h"
#include "midi/Arpeggiator.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
using P=sawstar::ParameterId;
constexpr int notes[]{48,52,55,57,59,62,64,67,69,71,74,76,79,81,83,86};
template<class Engine> void Controls(Engine& synth,sawstar::Arpeggiator& arp,const sawstar::Snapshot& state) {
  const auto value=[&](P id){return state[static_cast<size_t>(id)];};
  sawstar::ApplyEngineControls(synth,arp,value,[&](P id){return static_cast<int>(value(id));},136,true);
}
double Db(double peak){return 20*std::log10(std::max(peak,1.e-20));}
void GainContract() {
  // At safe levels every sample must remain a scalar copy of the original
  // listening engine, with identical filter/Drive/FX histories and latency.
  for(float rate:{44100.f,48000.f,96000.f,192000.f}) for(int mode=0;mode<4;++mode)
    for(float mix:{0.f,50.f,100.f}) {
      sawstar::premium_preview_engine::Synth candidate;
      sawstar::premium_preview_reference::Synth reference;
      auto setup=[&](auto& synth){
        synth.Reset(rate);synth.SetParameters(-40,1,100,.7,40);synth.SetOutputBoost(0);
        synth.SetSaw(23,70,90);synth.SetFilter(2000,25,mix);
        synth.SetFilterCharacter(6,mode);synth.SetChorus(true,12,.3,35);
        synth.SetDelay(true,10,350,20,6000,false,true,3,136);synth.Midi(0x90,60,96);
      };
      setup(candidate);setup(reference);
      const float gain=1.f+(sawstar::PremiumPreviewLevel::WetGain-1.f)*mix*.01f;
      for(int i=0;i<4096;++i){
        if(i==2048){candidate.Midi(0x80,60,0);reference.Midi(0x80,60,0);}
        const auto a=candidate.ProcessStereo(),b=reference.ProcessStereo();
        if(std::abs(a.left-b.left*gain)>2.e-7f||std::abs(a.right-b.right*gain)>2.e-7f)
          throw std::runtime_error("Output correction changed the accepted signal beyond scalar gain");
        if(mix==0&&(a.left!=b.left||a.right!=b.right))
          throw std::runtime_error("Dry path changed");
      }
    }
  // An abrupt mix gesture must not make an instantaneous output-gain jump.
  sawstar::PremiumPreviewLevel level;level.Reset(48000);level.Snap(0);
  const float first=level.Process(100);
  if(first<=1||first>=1.01f)throw std::runtime_error("Unsmoothed gain change");
  level.Reset(48000);if(level.Process(0)!=1)throw std::runtime_error("Reset did not restore unity");
}
void GuardContract() {
  for(float rate:{44100.f,48000.f,96000.f,192000.f}){
    sawstar::premium_preview_engine::Synth synth;
    synth.Reset(rate);synth.SetParameters(0,1,100,1,10);synth.SetOutputBoost(24);
    synth.SetSaw(10,0,0);synth.SetFilter(20000,0,100);synth.SetFilterCharacter(0,0);
    for(int n:notes)synth.Midi(0x90,n,127);
    bool guarded=false;
    for(int i=0;i<4096;++i){auto y=synth.ProcessStereo();
      const float peak=std::max(std::abs(y.left),std::abs(y.right));
      if(!std::isfinite(peak)||peak>.98001f)throw std::runtime_error("Output guard failed");
      guarded=guarded||peak>.97f;
    }
    if(!guarded)throw std::runtime_error("Guard stress did not reach the guard");
  }
}
}
int main(int argc,char** argv){try{
  const bool measure=argc==2&&std::string_view(argv[1])=="--measure";
  if(argc>1&&!measure)throw std::runtime_error("Usage: premium_preview_levels [--measure]");
  GainContract();GuardContract();
  std::cout<<"preset,sample_rate,held_roots,velocity,pre_guard_peak_dbfs,rms_dbfs\n"<<std::setprecision(9);
  double worst=-400;int cells=0;
  for(float rate:{44100.f,48000.f})for(const auto& preset:sawstar::FactoryPresets())for(int count:{1,8,16}){
    // This copy has the SAME signal path and gain but no output limiter: a
    // passing factory test cannot hide clipping behind the safety guard.
    sawstar::premium_preview_meter::Synth synth;sawstar::Arpeggiator arp;
    synth.Reset(rate);arp.Init(rate);Controls(synth,arp,preset.values);
    auto send=[&](int a,int b,int c){synth.Midi(a,b,c);};
    for(int i=0;i<count;++i)arp.Midi(0x90,notes[i],127,send);
    double peak=0,energy=0;const int frames=static_cast<int>(rate*6);
    for(int i=0;i<frames;++i){
      if(i==static_cast<int>(rate*4))for(int n=0;n<count;++n)arp.Midi(0x80,notes[n],0,send);
      arp.Process(send);const auto y=synth.ProcessStereo();
      if(!std::isfinite(y.left)||!std::isfinite(y.right))throw std::runtime_error("Non-finite factory output");
      peak=std::max({peak,std::abs(double(y.left)),std::abs(double(y.right))});
      energy+=double(y.left)*y.left+double(y.right)*y.right;
    }
    const double db=Db(peak);worst=std::max(worst,db);++cells;
    std::cout<<preset.key<<','<<rate<<','<<count<<",127,"<<db<<','<<10*std::log10(std::max(energy/(frames*2),1.e-40))<<std::endl;
    if(!measure&&db>-6.0)throw std::runtime_error(std::string("Factory headroom failed: ")+preset.key);
  }
  std::cerr<<"Preview level contracts PASS; factory cells="<<cells<<"; worst pre-guard peak="<<worst<<" dBFS\n";
  return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
