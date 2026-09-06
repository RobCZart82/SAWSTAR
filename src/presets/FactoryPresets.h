// SPDX-License-Identifier: MIT
#pragma once
#include "plugin/State.h"
namespace sawstar {
struct FactoryPreset { const char* key; const char* name; const char* category; const char* lesson; Snapshot values; };
const std::array<FactoryPreset,8>& FactoryPresets();
// Exact factory sound identification, tolerant of host normalization roundoff.
// -1 means a custom/edited sound; no additional preset identity is serialized.
int MatchFactoryPreset(const Snapshot& values);
int StepFactoryPreset(int current,int direction);
}
