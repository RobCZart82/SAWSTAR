// SPDX-License-Identifier: MIT
// Standalone Release profiling tool, not a CI pass/fail timing gate.
#include "PremiumDrive.h"
#include "ReferencePremiumDrive.h"
#include <string>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>

template<class Drive> int Benchmark(bool smallBlocks = false) {
  using Clock = std::chrono::steady_clock;
  constexpr int frames = 8192;
  volatile double checksum = 0;
  std::cout << (smallBlocks ? "rate,voices,drive_db,buffer,median_realtime_percent,worst_block_percent\n"
                          : "rate,voices,drive_db,median_realtime_percent\n");
  for (double rate : {48000., 96000., 192000.}) {
    std::vector<std::array<float, 2>> input(frames);
    for (int i = 0; i < frames; ++i) {
      input[i] = {static_cast<float>(.4 * std::sin(6.283185307179586 * 440 * i / rate)),
                  static_cast<float>(.4 * std::sin(6.283185307179586 * 660 * i / rate))};
    }
    for (int voices : {1, 8, 16}) {
      if (smallBlocks && voices != 16) continue;
      for (double db : {0., 20., 24.}) {
        if (smallBlocks && db != 20) continue;
        for (int buffer : {32, 64, 128, 256, frames}) {
          if (smallBlocks ? buffer == frames : buffer != frames) continue;
          double worstBlock = 0;
        std::array<double, 3> times{};
        for (auto& elapsed : times) {
          std::array<Drive, 16> bank;
          for (int v = 0; v < voices; ++v) {
            bank[v].Init(rate); bank[v].Set(db); bank[v].SnapToTargets();
            for (int i = 0; i < 256; ++i) bank[v].Process(input[i]);
          }
          double sum = 0;
          const auto start = Clock::now();
          for (int begin = 0; begin < frames; begin += buffer) {
            const auto blockStart = Clock::now();
            for (int i = begin; i < begin + buffer; ++i) for (int v = 0; v < voices; ++v) {
              const auto y = bank[v].Process(input[i]); sum += y[0] + y[1];
            }
            const double blockTime = std::chrono::duration<double>(Clock::now() - blockStart).count();
            worstBlock = std::max(worstBlock, 100 * blockTime * rate / buffer);
          }
          elapsed = std::chrono::duration<double>(Clock::now() - start).count();
          checksum = checksum + sum;
        }
        std::sort(times.begin(), times.end());
        std::cout << rate << ',' << voices << ',' << db << ',';
        if (smallBlocks) std::cout << buffer << ',';
        std::cout << 100 * times[1] * rate / frames;
        if (smallBlocks) std::cout << ',' << worstBlock;
        std::cout << '\n';
        }
      }
    }
  }
  std::cerr << "Finite checksum: " << std::isfinite(checksum) << '\n';
  return std::isfinite(checksum) ? 0 : 1;
}

int main(int argc, char** argv) {
  bool reference = false, smallBlocks = false;
  for (int i = 1; i < argc; ++i) {
    const std::string option = argv[i];
    if (option == "--reference" && !reference) reference = true;
    else if (option == "--small-blocks" && !smallBlocks) smallBlocks = true;
    else { std::cerr << "Usage: premium_drive_benchmark [--reference] [--small-blocks]\n"; return 1; }
  }
  if (reference) return Benchmark<sawstar::experimental::ReferencePremiumDrive>(smallBlocks);
  return Benchmark<sawstar::experimental::PremiumDrive>(smallBlocks);
}
