// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <cstdint>
#include <string_view>

namespace sawstar {
// Persisted IDs / future iPlug2 indices: append only, never reorder or reuse.
enum class ParameterId : std::uint32_t {
  OutputGain = 0, AmpAttack = 1, AmpDecay = 2, AmpSustain = 3, AmpRelease = 4, SawDetune = 5, SawMix = 6, SawWidth = 7, FilterCutoff = 8, FilterResonance = 9, FilterMix = 10, FilterEnvAmount = 11, FilterKeyTrack = 12, FilterAttack = 13, FilterDecay = 14, FilterSustain = 15, FilterRelease = 16, BendRange = 17, ModDepth = 18, OutputBoost = 19, Osc1Level = 20, Osc2Level = 21, SubLevel = 22, NoiseLevel = 23, Osc2Octave = 24, SubOctave = 25, NoiseType = 26, Osc2Detune = 27, Osc2Mix = 28, Osc2Width = 29, Osc1Octave = 30, FilterDrive = 31, FilterMode = 32, Osc1Wave = 33, Osc2Wave = 34, LfoRate = 35, LfoDepth = 36, LfoShape = 37, LfoTarget = 38, LfoSync = 39, LfoDivision = 40, LfoRetrigger = 41, ChorusEnabled = 42, ChorusMix = 43, ChorusRate = 44, ChorusDepth = 45, DelayEnabled = 46, DelayMix = 47, DelayTime = 48, DelayFeedback = 49, DelayTone = 50, DelayMode = 51, DelaySync = 52, DelayDivision = 53
};
enum class Mapping { Linear, Logarithmic };
struct ParameterSpec {
  ParameterId id;
  std::string_view key, name, unit;
  double minimum, maximum, initial;
  Mapping mapping;
};
inline constexpr std::array<ParameterSpec, 54> kParameters{{
  {ParameterId::OutputGain, "output.gain_db", "Output", "dB", -60., 0., -12., Mapping::Linear},
  {ParameterId::AmpAttack, "amp.attack_ms", "Attack", "ms", 1., 10000., 10., Mapping::Logarithmic},
  {ParameterId::AmpDecay, "amp.decay_ms", "Decay", "ms", 1., 10000., 100., Mapping::Logarithmic},
  {ParameterId::AmpSustain, "amp.sustain", "Sustain", "ratio", 0., 1., 0.7, Mapping::Linear},
  {ParameterId::AmpRelease, "amp.release_ms", "Release", "ms", 1., 10000., 250., Mapping::Logarithmic},
  {ParameterId::SawDetune, "saw.detune_cents", "Detune", "cents", 0., 50., 20., Mapping::Linear},
  {ParameterId::SawMix, "saw.mix", "Mix", "%", 0., 100., 0., Mapping::Linear},
  {ParameterId::SawWidth, "saw.width", "Width", "%", 0., 100., 75., Mapping::Linear},
  {ParameterId::FilterCutoff, "filter.cutoff_hz", "Cutoff", "Hz", 20., 20000., 12000., Mapping::Logarithmic},
  {ParameterId::FilterResonance, "filter.resonance", "Resonance", "%", 0., 100., 0., Mapping::Linear},
  {ParameterId::FilterMix, "filter.mix", "Filter Mix", "%", 0., 100., 0., Mapping::Linear},
  {ParameterId::FilterEnvAmount, "filter.env_amount_st", "Env Amount", "st", -96., 96., 0., Mapping::Linear},
  {ParameterId::FilterKeyTrack, "filter.key_track", "Key Track", "%", 0., 100., 0., Mapping::Linear},
  {ParameterId::FilterAttack, "filter.attack_ms", "F Attack", "ms", 1., 10000., 10., Mapping::Logarithmic},
  {ParameterId::FilterDecay, "filter.decay_ms", "F Decay", "ms", 1., 10000., 200., Mapping::Logarithmic},
  {ParameterId::FilterSustain, "filter.sustain", "F Sustain", "ratio", 0., 1., 0., Mapping::Linear},
  {ParameterId::FilterRelease, "filter.release_ms", "F Release", "ms", 1., 10000., 250., Mapping::Logarithmic},
  {ParameterId::BendRange, "performance.bend_range_st", "Bend Range", "st", 0., 24., 2., Mapping::Linear},
  {ParameterId::ModDepth, "performance.mod_depth_st", "Mod Depth", "st", 0., 48., 24., Mapping::Linear},
  {ParameterId::OutputBoost, "output.boost_db", "Level Boost", "dB", 0., 24., 18., Mapping::Linear},
  {ParameterId::Osc1Level, "mixer.osc1", "OSC1", "%", 0., 100., 100., Mapping::Linear},
  {ParameterId::Osc2Level, "mixer.osc2", "OSC2", "%", 0., 100., 0., Mapping::Linear},
  {ParameterId::SubLevel, "mixer.sub", "SUB", "%", 0., 100., 0., Mapping::Linear},
  {ParameterId::NoiseLevel, "mixer.noise", "NOISE", "%", 0., 100., 0., Mapping::Linear},
  {ParameterId::Osc2Octave, "osc2.octave", "OSC2 Oct", "oct", -2., 2., 0., Mapping::Linear},
  {ParameterId::SubOctave, "sub.octave", "SUB Oct", "oct", -2., 0., -1., Mapping::Linear},
  {ParameterId::NoiseType, "noise.type", "Noise Type", "", 0., 1., 0., Mapping::Linear},
  {ParameterId::Osc2Detune, "osc2.detune_cents", "OSC2 Detune", "cents", 0., 50., 20., Mapping::Linear},
  {ParameterId::Osc2Mix, "osc2.mix", "OSC2 Unison", "%", 0., 100., 0., Mapping::Linear},
  {ParameterId::Osc2Width, "osc2.width", "OSC2 Width", "%", 0., 100., 75., Mapping::Linear},
  {ParameterId::Osc1Octave, "osc1.octave", "OSC1 Oct", "oct", -2., 2., 0., Mapping::Linear},
  {ParameterId::FilterDrive, "filter.drive_db", "Drive", "dB", 0., 24., 0., Mapping::Linear},
  {ParameterId::FilterMode, "filter.mode", "Filter Mode", "", 0., 3., 0., Mapping::Linear},
  {ParameterId::Osc1Wave, "osc1.waveform", "OSC1 Wave", "", 0.0, 3.0, 0.0, Mapping::Linear},
  {ParameterId::Osc2Wave, "osc2.waveform", "OSC2 Wave", "", 0.0, 3.0, 0.0, Mapping::Linear},
  {ParameterId::LfoRate, "lfo.rate_hz", "LFO Rate", "Hz", 0.05, 20.0, 1.0, Mapping::Logarithmic},
  {ParameterId::LfoDepth, "lfo.depth", "LFO Amount", "%", 0.0, 100.0, 0.0, Mapping::Linear},
  {ParameterId::LfoShape, "lfo.shape", "LFO Shape", "", 0.0, 3.0, 0.0, Mapping::Linear},
  {ParameterId::LfoTarget, "lfo.target", "LFO Target", "", 0.0, 3.0, 0.0, Mapping::Linear},
  {ParameterId::LfoSync, "lfo.sync", "LFO Sync", "", 0.0, 1.0, 0.0, Mapping::Linear},
  {ParameterId::LfoDivision, "lfo.division", "LFO Division", "", 0.0, 5.0, 2.0, Mapping::Linear},
  {ParameterId::LfoRetrigger, "lfo.retrigger", "LFO Retrigger", "", 0.0, 1.0, 0.0, Mapping::Linear},
  {ParameterId::ChorusEnabled, "chorus.enabled", "Chorus", "", 0., 1., 0., Mapping::Linear},
  {ParameterId::ChorusMix, "chorus.mix", "Chorus Mix", "%", 0., 100., 25., Mapping::Linear},
  {ParameterId::ChorusRate, "chorus.rate_hz", "Chorus Rate", "Hz", .05, 3., .3, Mapping::Logarithmic},
  {ParameterId::ChorusDepth, "chorus.depth", "Chorus Depth", "%", 0., 100., 35., Mapping::Linear},
  {ParameterId::DelayEnabled, "delay.enabled", "Delay", "", 0., 1., 0., Mapping::Linear},
  {ParameterId::DelayMix, "delay.mix", "Delay Mix", "%", 0., 100., 20., Mapping::Linear},
  {ParameterId::DelayTime, "delay.time_ms", "Time", "ms", 1., 2000., 350., Mapping::Logarithmic},
  {ParameterId::DelayFeedback, "delay.feedback", "Feedback", "%", 0., 85., 30., Mapping::Linear},
  {ParameterId::DelayTone, "delay.tone_hz", "Tone", "Hz", 200., 16000., 6000., Mapping::Logarithmic},
  {ParameterId::DelayMode, "delay.mode", "Mode", "", 0., 1., 0., Mapping::Linear},
  {ParameterId::DelaySync, "delay.sync", "Sync", "", 0., 1., 0., Mapping::Linear},
  {ParameterId::DelayDivision, "delay.division", "Division", "", 0., 6., 3., Mapping::Linear}
}};

const ParameterSpec* FindParameter(std::uint32_t id) noexcept;
// Spec must come from kParameters. Non-finite input falls back to its default.
double Sanitize(const ParameterSpec& spec, double physical) noexcept;
double Normalize(const ParameterSpec& spec, double physical) noexcept;
double Denormalize(const ParameterSpec& spec, double normalized) noexcept;
} // namespace sawstar
