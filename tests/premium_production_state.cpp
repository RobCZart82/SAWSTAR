// SPDX-License-Identifier: MIT
#include "plugin/SynthEngine.h"
#include "plugin/EngineControls.h"
#include "plugin/State.h"
#include "presets/FactoryPresets.h"
#include "midi/Arpeggiator.h"
#include <iostream>
#include <cmath>
#include <memory>
#include <type_traits>
#include <string_view>
#include <iomanip>
int main(){
 static_assert(std::is_same_v<sawstar::PluginSynth,sawstar::premium_preview_engine::Synth>);
 for(const auto& preset:sawstar::FactoryPresets()){
  auto bytes=sawstar::EncodeHostState(preset.values,1.25f);
  sawstar::Snapshot recalled{};float scale=0;
  const auto decodedBytes=sawstar::DecodeHostState(bytes.data(),bytes.size(),recalled,scale);
  if(decodedBytes!=bytes.size()){
   std::cerr<<preset.key<<": host-state decode consumed "<<decodedBytes<<" of "<<bytes.size()<<" bytes\n";
   return 1;
  }
  if(recalled!=preset.values){std::cerr<<preset.key<<": host-state round-trip changed preset parameters\n";return 1;}
  if(scale!=1.25f){std::cerr<<preset.key<<": host-state round-trip changed GUI scale to "<<std::hexfloat<<scale<<std::defaultfloat<<"\n";return 1;}
  // Legacy sound-state payloads remain readable without a GUI-scale record.
  auto legacy=sawstar::EncodeState(preset.values);sawstar::Snapshot old{};
  const auto legacyBytes=sawstar::DecodeState(legacy.data(),legacy.size(),old);
  if(legacyBytes!=legacy.size()){std::cerr<<preset.key<<": legacy-state decode consumed "<<legacyBytes<<" of "<<legacy.size()<<" bytes\n";return 1;}
  if(old!=recalled){std::cerr<<preset.key<<": legacy-state round-trip differs from host-state round-trip\n";return 1;}
  auto a=std::make_unique<sawstar::PluginSynth>(),b=std::make_unique<sawstar::PluginSynth>();
  sawstar::Arpeggiator arpa,arpb;
  auto setup=[](auto& synth,auto& arp,const auto& values){
   synth.Reset(48000);arp.Init(48000);
   auto value=[&](sawstar::ParameterId id){return values[static_cast<size_t>(id)];};
   sawstar::ApplyEngineControls(synth,arp,value,[&](auto id){return int(value(id));},136,true);
  };
  setup(*a,arpa,preset.values);setup(*b,arpb,recalled);
  auto sendA=[&](int s,int n,int v){a->Midi(s,n,v);};
  auto sendB=[&](int s,int n,int v){b->Midi(s,n,v);};
  arpa.Midi(0x90,60,100,sendA);arpb.Midi(0x90,60,100,sendB);
  for(int i=0;i<8192;++i){
   if(i==4096){arpa.Midi(0x80,60,0,sendA);arpb.Midi(0x80,60,0,sendB);}
   arpa.Process(sendA);arpb.Process(sendB);
   auto x=a->ProcessStereo(),y=b->ProcessStereo();
   if(x.left!=y.left||x.right!=y.right||!std::isfinite(x.left)||!std::isfinite(x.right)){
    std::cerr<<preset.key<<": recalled render mismatch at sample "<<i
             <<"; direct=("<<std::hexfloat<<x.left<<", "<<x.right<<") recalled=("<<y.left<<", "<<y.right<<")\n";
    return 1;
   }
  }
  if(std::string_view(preset.name).find("(use modwheel)")!=std::string_view::npos){
   setup(*a,arpa,preset.values);setup(*b,arpb,preset.values);
   arpb.Midi(0xb0,1,127,sendB);
   arpa.Midi(0x90,60,110,sendA);arpb.Midi(0x90,60,110,sendB);
   double difference=0,energy=0;
   for(int i=0;i<96000;++i){
    arpa.Process(sendA);arpb.Process(sendB);
    const auto x=a->ProcessStereo(),y=b->ProcessStereo();
    if(!std::isfinite(x.left)||!std::isfinite(y.left)||!std::isfinite(x.right)||!std::isfinite(y.right))return 1;
    difference+=std::abs(x.left-y.left)+std::abs(x.right-y.right);
    energy+=std::abs(x.left)+std::abs(x.right);
   }
   if(energy<.01||difference<energy*.01){std::cerr<<preset.key<<": inaudible wheel response\n";return 1;}
  }

 }
 std::cout<<"All factory presets: sound/host state, GUI scale and recalled render PASS\n";
}
