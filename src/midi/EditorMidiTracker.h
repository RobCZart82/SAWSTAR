// SPDX-License-Identifier: MIT
#pragma once

#include <array>
#include <cstddef>
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
    if (kind == 0x9 && velocity > 0) {
      auto& count = heldCounts_[channel][note];
      if (count != std::numeric_limits<std::uint32_t>::max()) ++count;
    } else if (kind == 0x8 || (kind == 0x9 && velocity == 0)) {
      // A later physical release first satisfies the oldest overflow debt.
      auto& pending = recoveryCounts_[channel][note];
      if (pending > 0) --pending;
      else if (heldCounts_[channel][note] > 0) --heldCounts_[channel][note];
    } else if (kind == 0xb && (note == 120 || note == 123)) {
      ClearChannel(channel);
    }
  }

  void ClearChannel(int channel) noexcept {
    if (channel >= 0 && channel < ChannelCount) {
      heldCounts_[channel].fill(0);
      recoveryCounts_[channel].fill(0);
    }
  }

  void Clear() noexcept {
    for (auto& channel : heldCounts_) channel.fill(0);
    for (auto& channel : recoveryCounts_) channel.fill(0);
  }

  // Snapshot the notes known at the end of the overflowing process call.
  // New editor events can be tracked separately while recovery is draining.
  void BeginRecovery() noexcept {
    for (int channel = 0; channel < ChannelCount; ++channel) {
      for (int note = 0; note < NoteCount; ++note) {
        const auto held = heldCounts_[channel][note];
        auto& pending = recoveryCounts_[channel][note];
        pending = held > std::numeric_limits<std::uint32_t>::max() - pending
            ? std::numeric_limits<std::uint32_t>::max() : pending + held;
        heldCounts_[channel][note] = 0;
      }
    }
  }

  // Drain at most `budget` synthetic Note Offs per audio block. The scan is
  // fixed-size (16 x 128); repeated key counts cannot create an unbounded loop
  // in the process callback.
  template <class Release>
  std::size_t ReleaseSome(std::size_t budget, Release&& release) noexcept(noexcept(release(0, 0))) {
    std::size_t sent = 0;
    for (int channel = 0; channel < ChannelCount && sent < budget; ++channel) {
      for (int note = 0; note < NoteCount && sent < budget; ++note) {
        auto& pending = recoveryCounts_[channel][note];
        while (pending > 0 && sent < budget) {
          --pending;
          ++sent;
          release(channel, note);
        }
      }
    }
    return sent;
  }

  std::uint64_t PendingReleaseCount() const noexcept {
    std::uint64_t total = 0;
    for (const auto& channel : recoveryCounts_)
      for (const auto count : channel) total += count;
    return total;
  }

private:
  // Match Synth::downCounts_: repeated same-pitch Note Ons need one Note Off
  // each, including recovery after an editor-to-audio queue overflow.
  std::array<std::array<std::uint32_t, NoteCount>, ChannelCount> heldCounts_{};
  std::array<std::array<std::uint32_t, NoteCount>, ChannelCount> recoveryCounts_{};
};

} // namespace sawstar
