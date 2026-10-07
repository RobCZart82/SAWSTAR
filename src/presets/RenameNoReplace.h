// SPDX-License-Identifier: MIT
#pragma once
#include <filesystem>
#include <stdexcept>
#include <cerrno>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__APPLE__)
#include <stdio.h>
#endif

namespace sawstar::detail {
// UI/file thread only. source and target must be on the same volume.
// The destination check belongs to the filesystem operation, not a prior exists().
inline void RenameNoReplace(const std::filesystem::path& source,
                            const std::filesystem::path& target) {
  namespace fs = std::filesystem;
#ifdef _WIN32
  // No REPLACE_EXISTING or COPY_ALLOWED: preserve collisions and use a rename,
  // including on FAT/exFAT volumes that cannot create hard links.
  if (!MoveFileExW(source.c_str(), target.c_str(), MOVEFILE_WRITE_THROUGH)) {
    const auto error = GetLastError();
    throw fs::filesystem_error("Cannot rename preset", source, target,
                              std::error_code(error, std::system_category()));
  }
#elif defined(__APPLE__)
  // Never fall back to plain rename: it may overwrite a concurrently created file.
  if (::renamex_np(source.c_str(), target.c_str(), RENAME_EXCL) != 0) {
    const auto error = errno;
    throw fs::filesystem_error("Cannot rename preset", source, target,
                              std::error_code(error, std::generic_category()));
  }
#else
  // Non-release platforms retain the exclusive hard-link implementation.
  fs::create_hard_link(source, target);
  try {
    if (!fs::remove(source)) throw std::runtime_error("Cannot remove old preset name.");
  } catch (...) {
    std::error_code ignored;
    fs::remove(target, ignored);
    throw;
  }
#endif
}
}
