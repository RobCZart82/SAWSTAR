// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace sawstar {
// Output-only attenuation measured with the +3 dB wet-preview calibration.
// 24 factory patches; 44.1/48 kHz, 1/8/16 held roots, velocity 127, FX/ARP
// enabled as saved. Aim below -6.5 dBFS before the output safety guard.
inline constexpr std::array<std::pair<std::string_view,double>,24> kPremiumPreviewFactoryTrims{{
  {"init",8},{"wide_lead",11},{"soft_pad",7},{"saw_pluck",8},
  {"deep_bass",0},{"bright_keys",9},{"supersaw_one",7},{"soft_pad_one",7},
  {"pad_init",12},{"lead_init",12},{"pluck_init",11},{"bass_init",0},
  {"silk_lead",0},{"warm_horizon",7},{"glass_pluck",15},{"simple_steps",0},
  {"keys_init",13},{"arp_init",13},{"solid_saw",6},{"velvet_pad",8},
  {"trance_pluck",8},{"round_sub",0},{"octave_steps",0},{"air_keys",12}
}};
inline double PremiumPreviewFactoryTrim(std::string_view key) {
  for(const auto& trim:kPremiumPreviewFactoryTrims)if(trim.first==key)return trim.second;
  throw std::logic_error("Uncalibrated premium preview factory preset");
}
}
