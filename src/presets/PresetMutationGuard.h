// SPDX-License-Identifier: MIT
#pragma once
#include <mutex>

namespace sawstar::detail {
// Process-local exclusion shared by preset mutations across plugin instances.
// Internal header also lets the regression hold this real lock deterministically.
class PresetMutationGuard {
  std::unique_lock<std::mutex> lock_;
public:
  explicit PresetMutationGuard(bool wait=false);
};
}
