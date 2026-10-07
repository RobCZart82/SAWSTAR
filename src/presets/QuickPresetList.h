// SPDX-License-Identifier: MIT
#pragma once
#include "presets/FactoryPresets.h"
#include "presets/UserPresets.h"
#include <tuple>

namespace sawstar {
struct QuickPresetEntry {
  std::string name, category;
  int factory = -1;
  fs::path path;
  std::string Label() const { return name + " [" + category + "]"; }
};

// UI/file thread only. Sorting changes display order, never factory/state IDs.
inline std::vector<QuickPresetEntry> BuildQuickPresetList(const std::vector<fs::path>& files) {
  struct Sortable { QuickPresetEntry entry; std::string foldedName, foldedCategory; };
  std::vector<Sortable> sorted;
  const auto add = [&](QuickPresetEntry entry) {
    auto name = Fold(entry.name), category = Fold(entry.category);
    sorted.push_back({std::move(entry), std::move(name), std::move(category)});
  };
  const auto& factory = FactoryPresets();
  // Init remains an explicit confirmed action, outside arrow navigation.
  for (int i = 1; i < static_cast<int>(factory.size()); ++i)
    add({factory[i].name, factory[i].category, i, {}});
  for (const auto& file : files) add({file.stem().u8string(), "User", -1, file});
  std::sort(sorted.begin(), sorted.end(), [](const Sortable& a, const Sortable& b) {
    return std::tie(a.foldedName, a.foldedCategory, a.entry.name, a.entry.factory, a.entry.path)
         < std::tie(b.foldedName, b.foldedCategory, b.entry.name, b.entry.factory, b.entry.path);
  });
  std::vector<QuickPresetEntry> result;
  result.reserve(sorted.size());
  for (auto& item : sorted) result.push_back(std::move(item.entry));
  return result;
}

inline int FindQuickPreset(const std::vector<QuickPresetEntry>& entries, int factory,
                           bool userActive, const fs::path& userPath) {
  for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
    const auto& entry = entries[i];
    if (userActive ? entry.factory < 0 && entry.path == userPath : entry.factory >= 0 && entry.factory == factory)
      return i;
  }
  return -1;
}

inline int StepQuickPreset(int position, int direction, int count) {
  if (count <= 0) return -1;
  if (position < 0 || position >= count) return direction < 0 ? count - 1 : 0;
  return (position + (direction < 0 ? count - 1 : 1)) % count;
}
}
