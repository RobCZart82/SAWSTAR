// SPDX-License-Identifier: MIT
#include "midi/EditorMidiTracker.h"

#include <iostream>
#include <utility>
#include <vector>

int main() {
  sawstar::EditorMidiTracker tracker;
  auto drain = [&](auto&& release) {
    tracker.BeginRecovery();
    while (tracker.PendingReleaseCount() != 0)
      tracker.ReleaseSome(64, release);
  };
  tracker.Observe(0x90, 60, 100); // Delivered editor Note On.
  tracker.Observe(0x90, 60, 100); // Retrigger/overlap on the same pitch.
  tracker.Observe(0x80, 60, 0);   // One matching release leaves one press held.
  tracker.Observe(0x91, 127, 64); // A second channel remains independent.
  tracker.Observe(0x80, 61, 0);   // Unmatched Note Off is harmless.

  // Model a Note Off dropped by the bounded queue: no Observe call occurs.
  std::vector<std::pair<int, int>> released;
  drain([&](int channel, int note) {
    released.emplace_back(channel, note);
  });
  if (released.size() != 2 || released[0] != std::pair<int, int>{0, 60} ||
      released[1] != std::pair<int, int>{1, 127}) {
    std::cerr << "Editor MIDI overflow recovery did not release the outstanding presses.\n";
    return 1;
  }

  tracker.Observe(0x92, 0, 100);
  tracker.Observe(0x92, 0, 100); // Both Note Offs are lost during overflow.
  int repeatedReleases = 0;
  bool wrongRepeatedRelease = false;
  drain([&](int channel, int note) {
    wrongRepeatedRelease |= channel != 2 || note != 0;
    ++repeatedReleases;
  });
  if (repeatedReleases != 2 || wrongRepeatedRelease) {
    std::cerr << "Recovery did not emit one Note Off per repeated Note On.\n";
    return 1;
  }

  tracker.Observe(0x92, 0, 100);
  tracker.Observe(0x92, 0, 0); // Note On with velocity zero is a release.
  int extraReleases = 0;
  drain([&](int, int) { ++extraReleases; });
  if (extraReleases != 0) {
    std::cerr << "A released key was emitted again during recovery.\n";
    return 1;
  }

  tracker.Observe(0x90, 64, 100); // Old editor press.
  tracker.Observe(0x90, 64, 100);
  tracker.Observe(0xb0, 123, 0); // Host All Notes Off invalidates old channel state.
  // A fresh host press after the reset is intentionally not observed as GUI input.
  int staleReleases = 0;
  drain([&](int, int) { ++staleReleases; });
  if (staleReleases != 0) {
    std::cerr << "A channel reset left stale editor ownership for later recovery.\n";
    return 1;
  }

  tracker.Observe(0x90, 65, 100);
  tracker.Clear(); // Internal ARP/mode reset invalidates every tracked channel.
  drain([&](int, int) { ++staleReleases; });
  if (staleReleases != 0) {
    std::cerr << "A global reset left stale editor ownership for later recovery.\n";
    return 1;
  }

  for (int i = 0; i < 2049; ++i) tracker.Observe(0x90, 67, 100);
  tracker.BeginRecovery();
  if (tracker.PendingReleaseCount() != 2049) {
    std::cerr << "Recovery did not snapshot the accepted editor presses.\n";
    return 1;
  }
  const auto budgeted = tracker.ReleaseSome(16, [&](int, int) { ++staleReleases; });
  if (budgeted != 16 || tracker.PendingReleaseCount() != 2033) {
    std::cerr << "Recovery work was not bounded to the requested per-block budget.\n";
    return 1;
  }
  tracker.ClearChannel(0);
  if (tracker.PendingReleaseCount() != 0) {
    std::cerr << "Channel reset did not clear pending recovery presses.\n";
    return 1;
  }

  tracker.Observe(0x90, 68, 100);
  tracker.BeginRecovery();
  tracker.Observe(0x90, 68, 100); // A new press is tracked in the next epoch.
  tracker.Observe(0x80, 68, 0);   // Its off resolves the older debt first.
  if (tracker.PendingReleaseCount() != 0) {
    std::cerr << "A delivered Note Off did not resolve the oldest recovery debt.\n";
    return 1;
  }
  tracker.BeginRecovery();
  int epochReleases = 0;
  tracker.ReleaseSome(16, [&](int, int note) { epochReleases += note == 68; });
  if (epochReleases != 1 || tracker.PendingReleaseCount() != 0) {
    std::cerr << "Recovery did not preserve the new press after resolving the old one.\n";
    return 1;
  }
  return 0;
}
