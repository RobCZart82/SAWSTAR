// SPDX-License-Identifier: MIT
#pragma once
#include "plugin/Parameters.h"
#include <array>
#include <cstddef>
#include <cstdint>
namespace sawstar {
using Snapshot = std::array<double, kParameters.size()>;
using StateBytes = std::array<uint8_t, 16 + 12 * kParameters.size()>;
Snapshot DefaultSnapshot();
// Ignore floating-point round-trip noise, without hiding a control's smallest step.
bool SnapshotsMatch(const Snapshot& a,const Snapshot& b);
StateBytes EncodeState(const Snapshot& values);
// Returns consumed bytes or zero; output remains untouched on failure.
// Accepts the VST3 wrapper's optional four-byte bypass trailer.
size_t DecodeState(const uint8_t* data, size_t size, Snapshot& output);
// Standalone .sawstar files require a non-empty versioned payload containing
// at least one known parameter, with no host trailer. Failure leaves output intact.
// Old versioned partial files remain supported; headerless legacy is host-only.
size_t DecodePresetFile(const uint8_t* data, size_t size, Snapshot& output);
}
