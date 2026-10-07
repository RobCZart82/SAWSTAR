// SPDX-License-Identifier: MIT
#pragma once
#include "presets/RenameNoReplace.h"
#include <string>
#include <limits>

namespace sawstar::detail {
// Try each backup name with filesystem exclusion. A separate exists() check
// cannot protect a backup created by a writer outside our mutation lock.
template<typename Rename>
std::filesystem::path ArchiveNoReplace(const std::filesystem::path& source, Rename&& rename) {
  for (unsigned long long suffix = 0; ; ++suffix) {
    auto target = source;
    target += suffix == 0 ? ".deleted" : ".deleted-" + std::to_string(suffix);
    try {
      rename(source, target);
      return target;
    } catch (const std::filesystem::filesystem_error& error) {
      // Permissions, missing sources and unsupported filesystems are not
      // collisions. Preserve the source and report those failures immediately.
      if (error.code() != std::errc::file_exists ||
          suffix == std::numeric_limits<unsigned long long>::max()) throw;
    }
  }
}
inline std::filesystem::path ArchiveNoReplace(const std::filesystem::path& source) {
  return ArchiveNoReplace(source, RenameNoReplace);
}
}
