// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <cstdint>
#include <string_view>

namespace sawstar {
// Persisted IDs / future iPlug2 indices: append only, never reorder or reuse.
enum class ParameterId : std::uint32_t {
  OutputGain = 0, AmpAttack = 1, AmpDecay = 2, AmpSustain = 3, AmpRelease = 4, SawDetune = 5, SawMix = 6, SawWidth = 7, FilterCutoff = 8, FilterResonance = 9, FilterMix = 10, FilterEnvAmount = 11, FilterKeyTrack = 12, FilterAttack = 13, FilterDecay = 14, FilterSustain = 15, FilterRelease = 16
};
enum class Mapping { Linear, Logarithmic };
struct ParameterSpec {
  ParameterId id;
  std::string_view key, name, unit;
  double minimum, maximum, initial;
  Mapping mapping;
};
inline constexpr std::array<ParameterSpec, 17> kParameters{{
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
  {ParameterId::FilterRelease, "filter.release_ms", "F Release", "ms", 1., 10000., 250., Mapping::Logarithmic}
}};

const ParameterSpec* FindParameter(std::uint32_t id) noexcept;
// Spec must come from kParameters. Non-finite input falls back to its default.
double Sanitize(const ParameterSpec& spec, double physical) noexcept;
double Normalize(const ParameterSpec& spec, double physical) noexcept;
double Denormalize(const ParameterSpec& spec, double normalized) noexcept;
} // namespace sawstar
