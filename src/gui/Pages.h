// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <string_view>
namespace sawstar::gui {
enum class Page { Main, Advanced, Presets };
struct PageSpec { Page id; std::string_view title, purpose; };
inline constexpr std::array<PageSpec, 3> kPages{{
  {Page::Main, "MAIN", "Sound design / signal flow"},
  {Page::Advanced, "ADVANCED", "Performance / arpeggiator / modulation"},
  {Page::Presets, "PRESETS", "Library / learning"}
}};
} // namespace sawstar::gui
