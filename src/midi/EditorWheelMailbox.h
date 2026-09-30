// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <atomic>
#include <cstdint>

namespace sawstar {
// GUI wheel gestures are state updates at offset zero, not note edges. Keep the
// latest update outside the finite editor FIFO so its final return cannot drop.
// Only Publish runs on the UI thread; Drain/Clear run on the audio/reset thread.
class EditorWheelMailbox {
public:
  static_assert(std::atomic<std::uint32_t>::is_always_lock_free,
                "Editor wheel delivery must not lock on the audio thread");
  bool Publish(int status, int data1, int data2, int offset = 0) noexcept {
    if (offset != 0 || status < 0x80 || status > 0xef ||
        data1 < 0 || data1 > 127 || data2 < 0 || data2 > 127) return false;
    const int kind = status & 0xf0;
    const int controller = kind == 0xe0 ? 0 : (kind == 0xb0 && data1 == 1 ? 1 : -1);
    if (controller < 0) return false;
    const auto value = static_cast<std::uint32_t>(controller == 0 ? data1 + (data2 << 7) : data2);
    pending_[(status & 15) * 2 + controller].store(value + 1, std::memory_order_release);
    return true;
  }
  template<class Send> void Drain(Send&& send) {
    // Bounded 32 atomic exchanges, no retry loop or heap allocation. A publish
    // after its exchange is delivered next block. Do not replay unchanged state.
    for (int channel = 0; channel < 16; ++channel) {
      auto bend = pending_[channel * 2].exchange(0, std::memory_order_acquire);
      if (bend) { --bend; send(0xe0 | channel, bend & 127, bend >> 7); }
      auto mod = pending_[channel * 2 + 1].exchange(0, std::memory_order_acquire);
      if (mod) send(0xb0 | channel, 1, mod - 1);
    }
  }
  void Clear() noexcept {
    for (auto& value : pending_) value.store(0, std::memory_order_relaxed);
  }
private:
  std::array<std::atomic<std::uint32_t>, 32> pending_{};
};
} // namespace sawstar
