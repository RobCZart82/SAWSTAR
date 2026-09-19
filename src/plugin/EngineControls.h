// SPDX-License-Identifier: MIT
#pragma once
#include "plugin/Parameters.h"
namespace sawstar {
// Shared production mapping; caller owns parameter locking and mode/reset policy.
// Separate integer access preserves host stepped-parameter conversion semantics.
template<class SynthType,class ArpType,class Value,class Integer>
void ApplyEngineControls(SynthType& synth,ArpType& arp,Value value,Integer integer,double tempo,bool running){
  auto send=[&](int s,int n,int v){synth.Midi(s,n,v);};
  arp.Set(integer(ParameterId::ArpEnabled)!=0,integer(ParameterId::ArpMode),integer(ParameterId::ArpRate),value(ParameterId::ArpGate),integer(ParameterId::ArpOctaves),value(ParameterId::ArpSwing),integer(ParameterId::ArpHold)!=0,tempo,running,send);
  synth.SetLfo2(value(ParameterId::Lfo2Rate),value(ParameterId::Lfo2Depth),integer(ParameterId::Lfo2Shape),integer(ParameterId::Lfo2Target),integer(ParameterId::Lfo2Sync)!=0,integer(ParameterId::Lfo2Division),tempo,integer(ParameterId::Lfo2Retrigger)!=0);
  for(int row=0;row<4;++row){const int id=71+row*3;synth.SetModulation(row,integer(static_cast<ParameterId>(id)),integer(static_cast<ParameterId>(id+1)),value(static_cast<ParameterId>(id+2)));}
  synth.SetVoiceMode(integer(ParameterId::VoiceMode),value(ParameterId::GlideTime),integer(ParameterId::GlideMode)!=0);
  synth.SetParameters(value(ParameterId::OutputGain), value(ParameterId::AmpAttack), value(ParameterId::AmpDecay),
                       value(ParameterId::AmpSustain), value(ParameterId::AmpRelease));
  synth.SetMixer(value(ParameterId::Osc1Level),value(ParameterId::Osc2Level),value(ParameterId::SubLevel),value(ParameterId::NoiseLevel),
    integer(ParameterId::Osc2Octave),integer(ParameterId::SubOctave),integer(ParameterId::NoiseSource)==0?integer(ParameterId::NoiseType):integer(ParameterId::NoiseSource)-1,integer(ParameterId::Osc1Octave));
  synth.SetNoiseColor(value(ParameterId::NoiseColor));
  synth.SetSubWave(integer(ParameterId::SubWave));
  synth.SetWidth(integer(ParameterId::WidthEnabled)!=0,value(ParameterId::WidthAmount));
  synth.SetOsc2(value(ParameterId::Osc2Detune),value(ParameterId::Osc2Mix),value(ParameterId::Osc2Width));
  synth.SetOutputBoost(static_cast<float>(value(ParameterId::OutputBoost)));
  synth.SetSaw(static_cast<float>(value(ParameterId::SawDetune)), static_cast<float>(value(ParameterId::SawMix)),
                static_cast<float>(value(ParameterId::SawWidth)));
  synth.SetReverb(integer(ParameterId::ReverbEnabled)!=0,value(ParameterId::ReverbMix),value(ParameterId::ReverbSize),value(ParameterId::ReverbDecay),value(ParameterId::ReverbDamping));
  synth.SetDelay(integer(ParameterId::DelayEnabled)!=0,value(ParameterId::DelayMix),value(ParameterId::DelayTime),value(ParameterId::DelayFeedback),value(ParameterId::DelayTone),integer(ParameterId::DelayMode)!=0,integer(ParameterId::DelaySync)!=0,integer(ParameterId::DelayDivision),tempo);
  synth.SetChorus(integer(ParameterId::ChorusEnabled)!=0,value(ParameterId::ChorusMix),value(ParameterId::ChorusRate),value(ParameterId::ChorusDepth));
  synth.SetWaveforms(integer(ParameterId::Osc1Wave),integer(ParameterId::Osc2Wave));
  synth.SetLfo(value(ParameterId::LfoRate),value(ParameterId::LfoDepth),integer(ParameterId::LfoShape),integer(ParameterId::LfoTarget),
    integer(ParameterId::LfoSync)!=0,integer(ParameterId::LfoDivision),tempo,integer(ParameterId::LfoRetrigger)!=0);
  synth.SetFilterCharacter(static_cast<float>(value(ParameterId::FilterDrive)),integer(ParameterId::FilterMode));
  synth.SetFilter(static_cast<float>(value(ParameterId::FilterCutoff)), static_cast<float>(value(ParameterId::FilterResonance)),
                   static_cast<float>(value(ParameterId::FilterMix)),false);
  synth.SetFilterEnvelope(static_cast<float>(value(ParameterId::FilterEnvAmount)), static_cast<float>(value(ParameterId::FilterKeyTrack)),
    static_cast<float>(value(ParameterId::FilterAttack)), static_cast<float>(value(ParameterId::FilterDecay)),
    static_cast<float>(value(ParameterId::FilterSustain)), static_cast<float>(value(ParameterId::FilterRelease)));
  synth.SetPerformance(static_cast<float>(value(ParameterId::BendRange)),static_cast<float>(value(ParameterId::ModDepth)));
}
}
