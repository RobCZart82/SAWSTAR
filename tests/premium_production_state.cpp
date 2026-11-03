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
int main(){
 static_assert(std::is_same_v<sawstar::PluginSynth,sawstar::premium_preview_engine::Synth>);
 for(const auto& preset:sawstar::FactoryPresets()){
  auto bytes=sawstar::EncodeHostState(preset.values,1.25f);
  sawstar::Snapshot recalled{};float scale=0;
  if(sawstar::DecodeHostState(bytes.data(),bytes.size(),recalled,scale)!=bytes.size() ||
     recalled!=preset.values||scale!=1.25f)return 1;
  // Legacy sound-state payloads remain readable without a GUI-scale record.
  auto legacy=sawstar::EncodeState(preset.values);sawstar::Snapshot old{};
  if(sawstar::DecodeState(legacy.data(),legacy.size(),old)!=legacy.size()||old!=recalled)return 1;
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
   if(x.left!=y.left||x.right!=y.right||!std::isfinite(x.left)||!std::isfinite(x.right))return 1;
  }
 }
 std::cout<<"All factory presets: sound/host state, GUI scale and recalled render PASS\n";
}
