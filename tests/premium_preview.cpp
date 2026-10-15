// SPDX-License-Identifier: MIT
#include "PremiumPreviewSynth.h"
#include "engine/Synth.h"
#include "plugin/config.h"
#include "presets/UserPresets.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string_view>

int main() {
  try {
    static_assert(PLUG_LATENCY == 32, "Preview must report its adapter latency");
    static_assert(PLUG_UNIQUE_ID != 'SwSt', "Preview must not replace the release");
    if(std::string_view(PLUG_NAME)!="SAWSTAR Premium Preview" ||
       std::string_view(BUNDLE_NAME)!="SAWSTARPremiumPreview" ||
       std::string_view(sawstar::kPresetProductFolder)!="SAWSTAR Premium Preview")
      throw std::runtime_error("Preview identity or preset isolation is missing");
    // Delayed dry and oversampled wet paths share the advertised latency.
    for(float rate:{44100.f,48000.f,96000.f,192000.f}) {
      sawstar::experimental::RateScaledLookupEnginePremiumFilter filter;
      filter.Init(rate);filter.Set(2000,40,0);filter.SnapToTargets();
      for(int i=0;i<40;++i) {
        auto y=filter.Process({i==0?1.f:0.f,0});
        if(y.left!=(i==32?1.f:0.f)||y.right!=0)
          throw std::runtime_error("Preview dry-path latency mismatch");
      }
    }
    // Exercise the actual preview Synth, all modes, MIDI and rate reset.
    double difference=0,energy=0;
    for(float rate:{44100.f,48000.f,96000.f}) for(int mode=0;mode<4;++mode) {
      sawstar::premium_preview_engine::Synth preview;
      sawstar::Synth classic;
      auto setup=[&](auto& synth){
        synth.Reset(rate);synth.SetParameters(-6,1,20,.6,20);
        synth.SetSaw(12,75,60);synth.SetFilterCharacter(12,mode);
        synth.SetFilter(2000,40,100);synth.Midi(0x90,60,100);
      };
      setup(preview);setup(classic);
      for(int i=0;i<4096;++i) {
        if(i==2048){preview.Midi(0x80,60,0);classic.Midi(0x80,60,0);}
        auto y=preview.ProcessStereo(),x=classic.ProcessStereo();
        if(!std::isfinite(y.left)||!std::isfinite(y.right)||
           std::abs(y.left)>1||std::abs(y.right)>1)
          throw std::runtime_error("Invalid preview output");
        energy+=double(y.left)*y.left+double(y.right)*y.right;
        difference+=std::abs(double(y.left)-x.left)+std::abs(double(y.right)-x.right);
      }
      preview.Midi(0xb0,120,0);
      if(preview.ActiveVoices()!=0)throw std::runtime_error("Preview panic left a voice active");
    }
    if(energy<=0||difference<=0)throw std::runtime_error("Preview is silent or still uses Classic");
    std::cout<<"Premium preview identity, 32-sample dry latency and four-mode engine smoke PASS\n";
    return 0;
  } catch(const std::exception& error) { std::cerr<<error.what()<<'\n';return 1; }
}
