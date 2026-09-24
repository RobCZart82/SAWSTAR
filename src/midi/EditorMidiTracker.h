// SPDX-License-Identifier: MIT
#pragma once

#include <array>
#include <cstdint>

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
    if (kind == 0x9) held_[channel][note] = velocity > 0;
    else if (kind == 0x8) held_[channel][note] = false;
  }

  template <class Release>
  void ReleaseAll(Release&& release) noexcept(noexcept(release(0, 0))) {
    for (int channel = 0; channel < ChannelCount; ++channel) {
      for (int note = 0; note < NoteCount; ++note) {
        if (held_[channel][note]) {
          held_[channel][note] = false;
          release(channel, note);
        }
      }
    }
  }

private:
  std::array<std::array<bool, NoteCount>, ChannelCount> held_{};
};

} // namespace sawstar
