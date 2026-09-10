// SPDX-License-Identifier: MIT
#pragma once
#include "plugin/State.h"
namespace sawstar {
struct FactoryPreset { const char* key; const char* name; const char* category; const char* lesson; Snapshot values; };
inline constexpr size_t kFactoryPresetCount=16;
const std::array<FactoryPreset,kFactoryPresetCount>& FactoryPresets();
// Exact factory sound identification, tolerant of host normalization roundoff.
// -1 means a custom/edited sound; no additional preset identity is serialized.
int MatchFactoryPreset(const Snapshot& values);
int StepFactoryPreset(int current,int direction);
}
