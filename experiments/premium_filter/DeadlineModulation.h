// SPDX-License-Identifier: MIT
#pragma once
#include <cstdint>

namespace sawstar::experimental {
// Versioned offline workload. API setters are part of the timed sample loop;
// this is not plugin automation or a native host callback.
template<class Synth> void PrepareDeadlineModulation(Synth& s) {
  s.SetLfo(20, 70, 3, 0, false, 0, 120, true);
  s.SetLfo2(17, 50, 2, 1, false, 0, 120, false);
  s.SetModulation(0, 3, 0, 100);
  s.SetModulation(1, 4, 0, -50);
  s.SetModulation(2, 5, 1, 20);
}
template<class Synth> void DeadlineModulationEvent(Synth& s, std::uint64_t sample, int mode) {
  if (sample % 128 == 0) {
    const auto step = sample / 128;
    s.SetFilter(500 + 6000 * float(step % 8) / 7.f, 50, step % 4 == 3 ? 35 : 100);
  }
  if (sample % 512 == 0) {
    const auto step = sample / 512;
    s.SetFilterCharacter(float(step % 3) * 12, mode);
    s.Midi(0xb0, 1, step % 2 ? 127 : 0);
    s.Midi(0xd0, step % 2 ? 127 : 0, 0);
    s.Midi(0xe0, 0, step % 2 ? 80 : 64);
  }
  if (sample % 2048 == 0)
    s.SetModulation(0, 3, (sample / 2048) % 2 ? 1 : 0, (sample / 2048) % 2 ? -30 : 100);
}
}
