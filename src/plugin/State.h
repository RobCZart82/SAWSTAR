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
StateBytes EncodeState(const Snapshot& values);
// Returns consumed bytes or zero; output remains untouched on failure.
// Accepts the VST3 wrapper's optional four-byte bypass trailer.
size_t DecodeState(const uint8_t* data, size_t size, Snapshot& output);
}
