// SPDX-License-Identifier: MIT
#include "presets/FactoryPresets.h"
#include <cmath>
#include <initializer_list>
#include <utility>
namespace sawstar {
namespace {
Snapshot Sound(std::initializer_list<std::pair<ParameterId,double>> edits) {
  auto s=DefaultSnapshot();for(auto p:edits)s[static_cast<size_t>(p.first)]=p.second;return s;
}
}
const std::array<FactoryPreset,kFactoryPresetCount>& FactoryPresets() {
 using P=ParameterId;
 static const std::array<FactoryPreset,kFactoryPresetCount> presets{{
  {"init","Init","Templates","One saw, open signal path. Start shaping your own sound.",DefaultSnapshot(),"Dry / Neutral"},
  {"wide_lead","Wide Saw Lead","Lead","Seven detuned saws add width. MOD opens the low-pass filter.",Sound({{P::OutputGain,-5},{P::SawMix,70},{P::SawDetune,23},{P::SawWidth,90},{P::FilterCutoff,2600},{P::FilterMix,100},{P::FilterResonance,15},{P::AmpRelease,180},{P::ChorusEnabled,1},{P::ChorusMix,12},{P::DelayEnabled,1},{P::DelayMix,10},{P::DelaySync,1},{P::DelayFeedback,20}}),"Wide / Bright / Poly"},
  {"soft_pad","Soft Pad","Pad","Slow amp attack and release soften the wide saw ensemble.",Sound({{P::OutputGain,-7},{P::SawMix,85},{P::SawDetune,16},{P::SawWidth,100},{P::AmpAttack,850},{P::AmpDecay,1500},{P::AmpSustain,.8},{P::AmpRelease,1800},{P::FilterCutoff,1100},{P::FilterMix,100},{P::FilterEnvAmount,12},{P::FilterAttack,1400},{P::FilterDecay,1800},{P::FilterSustain,.5},{P::FilterRelease,1800},{P::ReverbEnabled,1},{P::ReverbMix,18},{P::ReverbDecay,3.2}}),"Slow / Warm / Wide"},
  {"saw_pluck","Saw Pluck","Pluck","A short positive filter envelope makes the attack bright.",Sound({{P::OutputGain,-6},{P::SawMix,45},{P::SawDetune,12},{P::AmpAttack,2},{P::AmpDecay,180},{P::AmpSustain,.12},{P::AmpRelease,170},{P::FilterCutoff,260},{P::FilterMix,100},{P::FilterResonance,25},{P::FilterEnvAmount,48},{P::FilterAttack,2},{P::FilterDecay,140},{P::FilterSustain,0},{P::FilterRelease,140},{P::FilterKeyTrack,70},{P::DelayEnabled,1},{P::DelaySync,1},{P::DelayMix,16},{P::DelayFeedback,25}}),"Short / Bright / Echo"},
  {"deep_bass","Deep Bass","Bass","A centered saw and low cutoff keep the low notes focused.",Sound({{P::OutputGain,-6},{P::SawWidth,0},{P::AmpAttack,3},{P::AmpDecay,180},{P::AmpSustain,.6},{P::AmpRelease,110},{P::FilterCutoff,220},{P::FilterMix,100},{P::FilterResonance,12},{P::FilterEnvAmount,20},{P::FilterAttack,3},{P::FilterDecay,180},{P::FilterSustain,.15},{P::FilterRelease,110},{P::FilterKeyTrack,50},{P::VoiceMode,1},{P::SubLevel,20},{P::FilterDrive,3}}),"Mono / Warm / Dry"},
  {"bright_keys","Bright Keys","Keys","Moderate unison and a falling filter envelope give a key-like shape.",Sound({{P::OutputGain,-6},{P::SawMix,35},{P::SawDetune,8},{P::SawWidth,65},{P::AmpAttack,4},{P::AmpDecay,500},{P::AmpSustain,.25},{P::AmpRelease,450},{P::FilterCutoff,1500},{P::FilterMix,100},{P::FilterEnvAmount,18},{P::FilterAttack,4},{P::FilterDecay,350},{P::FilterSustain,.2},{P::FilterRelease,350},{P::FilterKeyTrack,100},{P::ReverbEnabled,1},{P::ReverbMix,12},{P::ReverbDecay,1.5}}),"Soft / Decay / Space"},
  {"supersaw_one","SuperSaw One","Lead","Two saw ensembles, a sine sub, restrained chorus and tempo delay build a broad lead.",Sound({
    {P::OutputGain,-9},{P::Osc1Level,85},{P::Osc2Level,70},{P::SubLevel,12},
    {P::SawMix,85},{P::SawDetune,21},{P::SawWidth,90},
    {P::Osc2Mix,90},{P::Osc2Detune,31},{P::Osc2Width,100},
    {P::AmpAttack,12},{P::AmpDecay,650},{P::AmpSustain,.85},{P::AmpRelease,550},
    {P::FilterMix,100},{P::FilterCutoff,4200},{P::FilterResonance,10},{P::FilterDrive,2},
    {P::FilterEnvAmount,10},{P::FilterAttack,8},{P::FilterDecay,700},{P::FilterSustain,.35},{P::FilterRelease,500},{P::FilterKeyTrack,40},
    {P::LfoRate,.23},{P::LfoDepth,5},{P::LfoTarget,0},{P::ModDepth,12},{P::ChorusEnabled,1},{P::ChorusMix,12},{P::DelayEnabled,1},{P::DelayMix,14},{P::DelaySync,1},{P::DelayFeedback,24},{P::ReverbEnabled,1},{P::ReverbMix,10},{P::ReverbDecay,1.8}}),"Wide / Trance / Poly"},
  {"soft_pad_one","Soft Pad One","Pad","Saw and triangle ensembles bloom through a slow filter, pink air, chorus and reverb.",Sound({
    {P::OutputGain,-7},{P::Osc1Level,75},{P::Osc2Level,65},{P::SubLevel,8},{P::NoiseLevel,1.5},{P::NoiseSource,3},{P::NoiseColor,-25},
    {P::SawMix,85},{P::SawDetune,14},{P::SawWidth,95},
    {P::Osc2Wave,2},{P::Osc2Mix,80},{P::Osc2Detune,23},{P::Osc2Width,100},
    {P::AmpAttack,1600},{P::AmpDecay,2200},{P::AmpSustain,.85},{P::AmpRelease,3200},
    {P::FilterMix,100},{P::FilterCutoff,950},{P::FilterResonance,8},{P::FilterDrive,1},
    {P::FilterEnvAmount,17},{P::FilterAttack,2400},{P::FilterDecay,3000},{P::FilterSustain,.6},{P::FilterRelease,3200},{P::FilterKeyTrack,35},
    {P::LfoRate,.12},{P::LfoDepth,12},{P::LfoTarget,0},{P::ModDepth,18},{P::ChorusEnabled,1},{P::ChorusMix,18},{P::ChorusRate,.18},{P::ReverbEnabled,1},{P::ReverbMix,22},{P::ReverbDecay,3.6},{P::ReverbDamping,4200}}),"Slow / Air / Wide"} ,
  {"pad_init","Pad-Init","Templates","Dry pad starting point: slow amp envelope and a warm filter. Add OSC2 or effects to taste.",Sound({{P::AmpAttack,900},{P::AmpDecay,1200},{P::AmpSustain,.8},{P::AmpRelease,1800},{P::FilterMix,100},{P::FilterCutoff,1800}}),"Dry / Slow"},
  {"lead_init","Lead-Init","Templates","Dry lead starting point: quick attack, sustained body and short release. MOD opens the filter.",Sound({{P::AmpAttack,5},{P::AmpDecay,250},{P::AmpSustain,.85},{P::AmpRelease,160},{P::FilterMix,100},{P::FilterCutoff,4500}}),"Dry / Sustain"},
  {"pluck_init","Pluck-Init","Templates","Dry pluck starting point: zero sustain, short amp and filter decays. Raise decay for longer notes.",Sound({{P::AmpAttack,2},{P::AmpDecay,280},{P::AmpSustain,0},{P::AmpRelease,160},{P::FilterMix,100},{P::FilterCutoff,500},{P::FilterEnvAmount,36},{P::FilterAttack,2},{P::FilterDecay,200}}),"Dry / Short"},
  {"bass_init","Bass-Init","Templates","Dry mono bass starting point: centered saw, low-pass and a short release. Add SUB in the mixer.",Sound({{P::OutputGain,-6},{P::VoiceMode,1},{P::SawWidth,0},{P::AmpAttack,3},{P::AmpDecay,200},{P::AmpSustain,.7},{P::AmpRelease,90},{P::FilterMix,100},{P::FilterCutoff,700}}),"Dry / Mono"},
  {"silk_lead","Silk Lead","Lead","Triangle plus a quiet saw ensemble. Mono legato and a small glide connect notes; delay adds space.",Sound({{P::OutputGain,-7},{P::Osc1Wave,2},{P::Osc1Level,90},{P::Osc2Level,30},{P::Osc2Mix,45},{P::Osc2Detune,9},{P::VoiceMode,2},{P::GlideTime,65},{P::AmpAttack,8},{P::AmpSustain,.9},{P::AmpRelease,220},{P::FilterMix,100},{P::FilterCutoff,5000},{P::DelayEnabled,1},{P::DelaySync,1},{P::DelayMix,15},{P::DelayFeedback,22}}),"Mono / Glide / Soft"},
  {"warm_horizon","Warm Horizon","Pad","Two softly detuned ensembles and a quiet sine sub. Slow envelopes, gentle chorus and reverb keep the pad steady.",Sound({{P::OutputGain,-6},{P::Osc1Level,80},{P::SawMix,65},{P::SawDetune,10},{P::Osc2Level,50},{P::Osc2Wave,2},{P::Osc2Mix,60},{P::Osc2Detune,17},{P::SubLevel,10},{P::AmpAttack,1200},{P::AmpDecay,1800},{P::AmpSustain,.8},{P::AmpRelease,2600},{P::FilterMix,100},{P::FilterCutoff,1300},{P::FilterKeyTrack,30},{P::ChorusEnabled,1},{P::ChorusMix,15},{P::ReverbEnabled,1},{P::ReverbMix,20},{P::ReverbDecay,3}}),"Warm / Slow / Wide"},
  {"glass_pluck","Glass Pluck","Pluck","Triangle and an octave-up sine add a clear attack. A short envelope and tempo delay leave room between notes.",Sound({{P::Osc1Wave,2},{P::Osc1Level,90},{P::Osc2Wave,3},{P::Osc2Level,45},{P::Osc2Octave,1},{P::AmpAttack,2},{P::AmpDecay,450},{P::AmpSustain,0},{P::AmpRelease,220},{P::FilterMix,100},{P::FilterCutoff,6000},{P::DelayEnabled,1},{P::DelaySync,1},{P::DelayMix,20},{P::DelayFeedback,32},{P::ReverbEnabled,1},{P::ReverbMix,10},{P::ReverbDecay,1.5}}),"Clear / Short / Echo"},
  {"simple_steps","Simple Steps","Arp","Hold a chord for an ascending two-octave pattern. Short filter plucks and restrained tempo delay; HOLD starts off.",Sound({{P::OutputGain,-6},{P::SawMix,35},{P::SawDetune,10},{P::AmpAttack,2},{P::AmpDecay,180},{P::AmpSustain,.15},{P::AmpRelease,100},{P::FilterMix,100},{P::FilterCutoff,650},{P::FilterEnvAmount,30},{P::FilterAttack,2},{P::FilterDecay,140},{P::ArpEnabled,1},{P::ArpOctaves,2},{P::ArpGate,55},{P::DelayEnabled,1},{P::DelaySync,1},{P::DelayMix,12},{P::DelayFeedback,20}}),"Up / Rhythmic / Echo"}
 ,
  {"keys_init","Keys-Init","Templates","Dry triangle keys with a falling amp envelope. Add an octave layer or a little reverb.",Sound({{P::Osc1Wave,2},{P::AmpAttack,3},{P::AmpDecay,650},{P::AmpSustain,.2},{P::AmpRelease,240},{P::FilterMix,100},{P::FilterCutoff,6500}}),"Dry / Decay"},
  {"arp_init","Arp-Init","Templates","Dry short notes ready for the arpeggiator. Enable ARP and hold a chord; HOLD stays off.",Sound({{P::AmpAttack,2},{P::AmpDecay,180},{P::AmpSustain,.1},{P::AmpRelease,90},{P::FilterMix,100},{P::FilterCutoff,1200},{P::ArpGate,55},{P::ArpOctaves,2}}),"Dry / ARP-ready"},
  {"solid_saw","Solid Saw Lead","Lead","Two saw ensembles keep a steady bright body. MOD opens the filter; dotted delay adds space.",Sound({{P::Osc1Level,85},{P::Osc2Level,65},{P::SawMix,75},{P::SawDetune,16},{P::SawWidth,85},{P::Osc2Mix,80},{P::Osc2Detune,24},{P::Osc2Width,95},{P::AmpAttack,6},{P::AmpDecay,450},{P::AmpSustain,.85},{P::AmpRelease,280},{P::FilterMix,100},{P::FilterCutoff,3400},{P::FilterKeyTrack,35},{P::FilterDrive,2},{P::ModDepth,12},{P::DelayEnabled,1},{P::DelaySync,1},{P::DelayDivision,2},{P::DelayMix,12},{P::DelayFeedback,22},{P::WidthEnabled,1},{P::WidthAmount,58}}),"Steady / Wide / Trance"},
  {"velvet_pad","Velvet Pad","Pad","Triangle and saw blend under a slow filter envelope. Quiet dark noise adds texture; reverb softens the tail.",Sound({{P::OutputGain,-7},{P::Osc1Wave,2},{P::Osc1Level,80},{P::SawMix,65},{P::SawDetune,12},{P::Osc2Level,50},{P::Osc2Mix,65},{P::Osc2Detune,18},{P::Osc2Width,95},{P::NoiseSource,2},{P::NoiseLevel,1.5},{P::NoiseColor,-40},{P::AmpAttack,1300},{P::AmpDecay,2000},{P::AmpSustain,.85},{P::AmpRelease,2600},{P::FilterMix,100},{P::FilterCutoff,1000},{P::FilterEnvAmount,10},{P::FilterAttack,1900},{P::FilterDecay,2600},{P::FilterSustain,.65},{P::FilterRelease,2600},{P::FilterKeyTrack,30},{P::ReverbEnabled,1},{P::ReverbMix,18},{P::ReverbDecay,3},{P::ReverbDamping,3800}}),"Warm / Slow / Texture"},
  {"trance_pluck","Trance Pluck","Pluck","Two saws and a fast filter decay make a bright pluck. Zero amp sustain leaves room for dotted echoes.",Sound({{P::OutputGain,-6},{P::Osc1Level,85},{P::SawMix,55},{P::SawDetune,12},{P::Osc2Level,40},{P::Osc2Octave,1},{P::Osc2Mix,40},{P::Osc2Detune,9},{P::AmpAttack,2},{P::AmpDecay,400},{P::AmpSustain,0},{P::AmpRelease,180},{P::FilterMix,100},{P::FilterCutoff,400},{P::FilterEnvAmount,42},{P::FilterAttack,2},{P::FilterDecay,230},{P::FilterSustain,0},{P::FilterRelease,160},{P::FilterKeyTrack,65},{P::DelayEnabled,1},{P::DelaySync,1},{P::DelayDivision,2},{P::DelayMix,17},{P::DelayFeedback,26},{P::ReverbEnabled,1},{P::ReverbMix,8},{P::ReverbDecay,1.3}}),"Bright / Short / Trance"},
  {"round_sub","Round Sub","Bass","A centered sine sub carries the low end. A quiet triangle adds definition; no stereo effects or unison.",Sound({{P::OutputGain,-10},{P::Osc1Wave,2},{P::Osc1Level,20},{P::SubLevel,85},{P::VoiceMode,1},{P::SawWidth,0},{P::AmpAttack,5},{P::AmpDecay,300},{P::AmpSustain,.8},{P::AmpRelease,100},{P::FilterMix,100},{P::FilterCutoff,1200},{P::FilterKeyTrack,50}}),"Mono / Sine / Dry"},
  {"octave_steps","Octave Steps","Arp","An up/down octave pattern with short square and saw notes. A little swing and tempo delay; HOLD is off.",Sound({{P::OutputGain,-10},{P::Osc1Wave,1},{P::Osc1Level,65},{P::Osc2Level,35},{P::Osc2Mix,25},{P::Osc2Detune,8},{P::AmpAttack,2},{P::AmpDecay,220},{P::AmpSustain,.12},{P::AmpRelease,90},{P::FilterMix,100},{P::FilterCutoff,1100},{P::FilterEnvAmount,22},{P::FilterAttack,2},{P::FilterDecay,160},{P::FilterKeyTrack,60},{P::ArpEnabled,1},{P::ArpMode,2},{P::ArpOctaves,2},{P::ArpGate,55},{P::ArpSwing,8},{P::DelayEnabled,1},{P::DelaySync,1},{P::DelayMix,10},{P::DelayFeedback,18}}),"Up-down / Swing / Echo"},
  {"air_keys","Air Keys","Keys","Triangle keys with a quiet octave sine and pink noise. Short reverb gives air without continuous pulsing.",Sound({{P::Osc1Wave,2},{P::Osc1Level,85},{P::Osc2Wave,3},{P::Osc2Level,35},{P::Osc2Octave,1},{P::NoiseSource,3},{P::NoiseLevel,1},{P::NoiseColor,15},{P::AmpAttack,4},{P::AmpDecay,850},{P::AmpSustain,.2},{P::AmpRelease,400},{P::FilterMix,100},{P::FilterCutoff,5500},{P::FilterKeyTrack,50},{P::ReverbEnabled,1},{P::ReverbMix,14},{P::ReverbDecay,1.8}}),"Soft / Air / Decay"}
 }};
 return presets;
}
int MatchFactoryPreset(const Snapshot& s) {
 const auto& presets=FactoryPresets();
 for(size_t p=0;p<presets.size();++p){bool match=true;
  for(size_t i=0;i<s.size();++i)if(!std::isfinite(s[i])||std::abs(s[i]-presets[p].values[i])>1e-7*(1+std::abs(presets[p].values[i]))){match=false;break;}
  if(match)return static_cast<int>(p);
 }return -1;
}
int StepFactoryPreset(int current,int direction){
 const int n=static_cast<int>(FactoryPresets().size());
 if(current<0||current>=n)return direction<0?n-1:0;
 return (current+(direction<0?n-1:1))%n;
}
}
