// SPDX-License-Identifier: MIT
#pragma once

#include <array>
#include <cstdint>
#include <limits>

namespace sawstar {

// Tracks only note events that made it through the editor-to-audio queue.
// If that upstream queue overflows, these editor-held keys need targeted
// Note Offs; host MIDI notes are deliberately not included.
class EditorMidiTracker {
public:
  static constexpr int ChannelCount = 16;
  static constexpr int NoteCount = 128;

  void Observe(std::uint8_t status, int note, int velocity) noexcept {
    if (note < 0 || note >= NoteCount) return;
    const int kind = status >> 4;
    const int channel = status & 0x0f;
    auto& count = heldCounts_[channel][note];
    if (kind == 0x9 && velocity > 0) {
      if (count != std::numeric_limits<std::uint32_t>::max()) ++count;
    } else if (kind == 0x8 || (kind == 0x9 && velocity == 0)) {
      if (count > 0) --count;
    }
  }

  template <class Release>
  void ReleaseAll(Release&& release) noexcept(noexcept(release(0, 0))) {
    for (int channel = 0; channel < ChannelCount; ++channel) {
      for (int note = 0; note < NoteCount; ++note) {
        const auto count = heldCounts_[channel][note];
        heldCounts_[channel][note] = 0;
        for (std::uint32_t i = 0; i < count; ++i) {
          release(channel, note);
        }
      }
    }
  }

private:
  // Match Synth::downCounts_: repeated same-pitch Note Ons need one Note Off
  // each, including recovery after an editor-to-audio queue overflow.
  std::array<std::array<std::uint32_t, NoteCount>, ChannelCount> heldCounts_{};
};

} // namespace sawstar
