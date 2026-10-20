// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <cstdint>
namespace sawstar {
// Diagnostic builds only: explicit values, never padding, pointers or raw objects.
struct NumericalTrace {
  static constexpr std::array<const char*,16> VoiceNames{{
    "envelope","bend","routed_pitch","osc1.left","osc1.right",
    "osc2.left","osc2.right","sub","noise","mixed.left","mixed.right",
    "filter.left","filter.right","rendered.left","rendered.right","velocity"}};
  static constexpr std::array<const char*,8> StageNames{{
    "voice_sum","pre_fx","chorus","delay","reverb","width","scaled","guarded"}};
  std::array<int,16> notes{};
  std::array<uint32_t,16> noiseState{};
  std::array<std::array<float,16>,16> voices{};
  std::array<std::array<float,2>,8> stages{};
  // Values that influence the current frame, not a complete DSP-state snapshot.
  std::array<float,6> controls{};
};
}
