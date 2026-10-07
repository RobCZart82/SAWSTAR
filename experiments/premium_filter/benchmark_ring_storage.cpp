// SPDX-License-Identifier: MIT
// Offline ring-capacity study. Wall time is descriptive and never gates CI.
#include "PremiumDrive.h"
#include "../../tests/fixtures/premium_drive_large_ring_reference.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace {
using Frame = std::array<float, 2>;
unsigned long long compared = 0;
volatile double checksum = 0;
template<unsigned Factor, bool Vector> using Before = sawstar::ring_reference::FixedRatePremiumDrive<Factor, true, false, Vector>;
template<unsigned Factor, bool Vector> using After = sawstar::experimental::FixedRatePremiumDrive<Factor, true, false, Vector>;

template<unsigned Factor, bool Vector> void Contract() {
  using A = After<Factor, Vector>; using B = Before<Factor, Vector>;
  static_assert(sizeof(A) + (Factor == 2 ? 4096 : 0) == sizeof(B));
  static_assert(A::Latency == 32 && B::Latency == 32);
  for (double rate : {8000., 44100., 48000., 96000., 192000., 384000.}) {
    A a; B b; a.Init(rate); b.Init(rate);
    auto equal = [](A& x, B& y, Frame input) {
      const auto actual = x.Process(input), expected = y.Process(input);
      if (!std::isfinite(actual[0]) || !std::isfinite(actual[1]) ||
          std::memcmp(actual.data(), expected.data(), sizeof(actual)))
        throw std::runtime_error("Ring capacity changed output");
      ++compared;
    };
    uint32_t random = 9;
    for (int n = 0; n < 16384; ++n) {
      if (n % 79 == 0) { const double db = (n / 79) % 25; a.Set(db); b.Set(db); }
      if (n % 997 == 0) { a.SnapToTargets(); b.SnapToTargets(); }
      if (n % 4093 == 0) { a.Clear(); b.Clear(); }
      Frame x{};
      for (auto& v : x) {
        random = random * 1664525u + 1013904223u;
        v = float((random / 4294967296. - .5) * 3.);
      }
      if (n % 1031 == 0) x[0] = std::numeric_limits<float>::quiet_NaN();
      if (n % 2053 == 0) x[1] = std::numeric_limits<float>::infinity();
      if (n == 4000) {
        auto copyA = a; auto copyB = b;
        for (int i = 0; i < 257; ++i) equal(copyA, copyB, {.2f, -.3f});
        a = copyA; b = copyB;
      }
      equal(a, b, x);
    }
    for (int offset : {0, 31, 32, 63, 64, 65, 127, 128, 129}) {
      a.Init(rate); b.Init(rate);
      for (int n = 0; n < 512; ++n)
        equal(a, b, {n == offset ? .8f : 0.f, n == offset + 1 ? -.6f : 0.f});
    }
    a.Init(rate); b.Init(rate);
    float peak = 0; int peakAt = -1;
    for (int n = 0; n < 128; ++n) {
      const Frame input{n == 0 ? 1.f : 0.f, 0};
      const auto actual = a.Process(input), expected = b.Process(input);
      if (std::memcmp(actual.data(), expected.data(), sizeof(actual)) || actual[1] != 0)
        throw std::runtime_error("Ring impulse or channel isolation");
      if (std::abs(actual[0]) > peak) { peak = std::abs(actual[0]); peakAt = n; }
      ++compared;
    }
    if (peakAt != 32) throw std::runtime_error("Ring latency changed");
  }
}

struct Timing { double seconds, sum; };
template<class Drive> Timing Measure(const std::vector<Frame>& input, double rate) {
  auto bank = std::make_unique<std::array<Drive, 16>>();
  for (auto& drive : *bank) {
    drive.Init(rate); drive.Set(20); drive.SnapToTargets();
    for (int n = 0; n < 1024; ++n) drive.Process(input[n]);
  }
  double sum = 0;
  const auto start = std::chrono::steady_clock::now();
  for (auto x : input) for (auto& drive : *bank) {
    const auto y = drive.Process(x); sum += y[0] + y[1];
  }
  const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
  if (!std::isfinite(seconds) || seconds <= 0 || !std::isfinite(sum))
    throw std::runtime_error("Invalid ring timing/checksum");
  checksum = checksum + sum;
  return {seconds, sum};
}

template<unsigned Factor, bool Vector, bool Normalization = false> void Benchmark() {
  using A = sawstar::experimental::FixedRatePremiumDrive<Factor, true, false, Vector, Normalization>;
  using B = std::conditional_t<Normalization, After<Factor, Vector>, Before<Factor, Vector>>;
  constexpr int frames = 16384;
  for (double rate : {48000., 96000., 192000.}) {
    std::vector<Frame> input(frames);
    for (int n = 0; n < frames; ++n)
      input[n] = {float(.4 * std::sin(6.283185307179586 * 440 * n / rate)),
                  float(.4 * std::cos(6.283185307179586 * 660 * n / rate))};
    for (int pair = 0; pair < 8; ++pair) {
      Timing before{}, after{};
      if (pair % 2) { after = Measure<A>(input, rate); before = Measure<B>(input, rate); }
      else { before = Measure<B>(input, rate); after = Measure<A>(input, rate); }
      if constexpr (!Normalization) {
        if (before.sum != after.sum) throw std::runtime_error("Paired checksum differs");
      }
      std::cout << Factor << ',' << (Vector ? "simd" : "scalar") << ','
                << rate << ",16,20," << pair << ',' << sizeof(B) << ',' << sizeof(A)
                << ',' << before.seconds << ',' << after.seconds << '\n';
    }
  }
}
}
int main(int argc, char** argv) {
  try {
    if (argc == 1) {
      Contract<2, false>(); Contract<2, true>(); Contract<4, false>(); Contract<4, true>();
      std::cout << compared << " stereo frames bit-identical to c5f5d635 large-ring fixture; backend "
                << sawstar::experimental::detail::FirLanes4::Backend << '\n';
    } else if (argc == 2 && (std::string(argv[1]) == "--benchmark" || std::string(argv[1]) == "--normalization-benchmark")) {
      std::cerr << "vector_backend=" << sawstar::experimental::detail::FirLanes4::Backend << '\n';
      std::cout << std::setprecision(17)
                << "factor,implementation,rate,voices,drive_db,pair,reference_bytes,candidate_bytes,reference_seconds,candidate_seconds\n";
      if (std::string(argv[1]) == "--normalization-benchmark") {
        Benchmark<2, false, true>(); Benchmark<2, true, true>();
        Benchmark<4, false, true>(); Benchmark<4, true, true>();
      } else {
        Benchmark<2, false>(); Benchmark<2, true>(); Benchmark<4, false>(); Benchmark<4, true>();
      }
      if (!std::isfinite(checksum)) throw std::runtime_error("Nonfinite total checksum");
    } else throw std::runtime_error("Usage: premium_ring_study [--benchmark|--normalization-benchmark]");
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
