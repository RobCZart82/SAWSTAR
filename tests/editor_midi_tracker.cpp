// SPDX-License-Identifier: MIT
#include "midi/EditorMidiTracker.h"

#include <iostream>
#include <utility>
#include <vector>

int main() {
  sawstar::EditorMidiTracker tracker;
  tracker.Observe(0x90, 60, 100); // Delivered editor Note On.
  tracker.Observe(0x91, 127, 64); // A second channel remains independent.
  tracker.Observe(0x80, 61, 0);   // Unmatched Note Off is harmless.

  // Model a Note Off dropped by the bounded queue: no Observe call occurs.
  std::vector<std::pair<int, int>> released;
  tracker.ReleaseAll([&](int channel, int note) {
    released.emplace_back(channel, note);
  });
  if (released.size() != 2 || released[0] != std::pair<int, int>{0, 60} ||
      released[1] != std::pair<int, int>{1, 127}) {
    std::cerr << "Editor MIDI overflow recovery did not release the tracked keys.\n";
    return 1;
  }

  tracker.Observe(0x92, 0, 100);
  tracker.Observe(0x92, 0, 0); // Note On with velocity zero is a release.
  int extraReleases = 0;
  tracker.ReleaseAll([&](int, int) { ++extraReleases; });
  if (extraReleases != 0) {
    std::cerr << "A released key was emitted again during recovery.\n";
    return 1;
  }
  return 0;
}
