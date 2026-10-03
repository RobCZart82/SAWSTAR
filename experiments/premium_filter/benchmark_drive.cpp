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

template<class Drive> int Benchmark() {
  using Clock = std::chrono::steady_clock;
  constexpr int frames = 8192;
  volatile double checksum = 0;
  std::cout << "rate,voices,drive_db,median_realtime_percent\n";
  for (double rate : {48000., 96000., 192000.}) {
    std::vector<std::array<float, 2>> input(frames);
    for (int i = 0; i < frames; ++i) {
      input[i] = {static_cast<float>(.4 * std::sin(6.283185307179586 * 440 * i / rate)),
                  static_cast<float>(.4 * std::sin(6.283185307179586 * 660 * i / rate))};
    }
    for (int voices : {1, 8, 16}) {
      for (double db : {0., 20., 24.}) {
        std::array<double, 3> times{};
        for (auto& elapsed : times) {
          std::array<Drive, 16> bank;
          for (int v = 0; v < voices; ++v) {
            bank[v].Init(rate); bank[v].Set(db); bank[v].SnapToTargets();
            for (int i = 0; i < 256; ++i) bank[v].Process(input[i]);
          }
          double sum = 0;
          const auto start = Clock::now();
          for (const auto& x : input) for (int v = 0; v < voices; ++v) {
            const auto y = bank[v].Process(x); sum += y[0] + y[1];
          }
          elapsed = std::chrono::duration<double>(Clock::now() - start).count();
          checksum = checksum + sum;
        }
        std::sort(times.begin(), times.end());
        std::cout << rate << ',' << voices << ',' << db << ',' << 100 * times[1] * rate / frames << '\n';
      }
    }
  }
  std::cerr << "Finite checksum: " << std::isfinite(checksum) << '\n';
  return std::isfinite(checksum) ? 0 : 1;
}

int main(int argc, char** argv) {
  if (argc == 1) return Benchmark<sawstar::experimental::PremiumDrive>();
  if (argc == 2 && std::string(argv[1]) == "--reference")
    return Benchmark<sawstar::experimental::ReferencePremiumDrive>();
  std::cerr << "Usage: premium_drive_benchmark [--reference]\n"; return 1;
}
