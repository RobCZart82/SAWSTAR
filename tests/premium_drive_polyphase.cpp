// SPDX-License-Identifier: MIT
#include "../experiments/premium_filter/PremiumDrive.h"
#include "fixtures/premium_drive_sparse_reference.h"
#include "fixtures/premium_drive_runtime_phase_reference.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace {
using Frame = std::array<float, 2>;
unsigned long long compared = 0;
template<unsigned Factor, template<unsigned> class Oracle = sawstar::test_reference::FixedRatePremiumDrive> void Contract() {
  using Candidate = sawstar::experimental::FixedRatePremiumDrive<Factor>;
  using Reference = Oracle<Factor>;
  static_assert(sizeof(Candidate) + (std::is_same_v<Reference, sawstar::test_reference::FixedRatePremiumDrive<Factor>> ? 4096 : 0)
                <= sizeof(Reference), "Retain compact storage savings; phase dispatch adds no storage");
  static_assert(Candidate::Latency == Reference::Latency, "Latency is unchanged");
  if constexpr (std::is_same_v<Reference, sawstar::phase_reference::FixedRatePremiumDrive<Factor>>) {
    static_assert(sizeof(Candidate) + (Factor == 2 ? 4096 : 0) == sizeof(Reference),
                  "2x decimator saves exactly 4096 bytes; 4x storage is unchanged");
  }
  auto compare = [](Candidate& a, Reference& b, Frame x) {
    const auto actual = a.Process(x), expected = b.Process(x);
    if (std::memcmp(actual.data(), expected.data(), sizeof(actual)) != 0)
      throw std::runtime_error("Compact polyphase changed samples, factor=" + std::to_string(Factor)
        + ", compared=" + std::to_string(compared));
    ++compared;
  };
  for (double rate : {8000., 44100., 48000., 96000., 192000., 384000.}) {
    Candidate a; Reference b; a.Init(rate); b.Init(rate);
    uint32_t random = 1;
    for (int n = 0; n < 65536; ++n) {
      if (n % 101 == 0) { const double db = (n / 101) % 25; a.Set(db); b.Set(db); }
      if (n % 997 == 0) { a.SnapToTargets(); b.SnapToTargets(); }
      if (n % 4093 == 0) { a.Clear(); b.Clear(); }
      Frame x{};
      for (auto& v : x) {
        random = random * 1664525u + 1013904223u;
        v = static_cast<float>((random / 4294967296. - .5) * 1.6);
      }
      if (n % 1031 == 0) x[0] = std::numeric_limits<float>::quiet_NaN();
      if (n % 2053 == 0) x[1] = std::numeric_limits<float>::infinity();
      if (n == 30000) { a.Set(std::numeric_limits<double>::quiet_NaN()); b.Set(std::numeric_limits<double>::quiet_NaN()); }
      if (n == 20000) {
        // Nonzero ring positions must survive copying and repeated use.
        auto copyA = a; auto copyB = b;
        for (int i = 0; i < 512; ++i) compare(copyA, copyB, {.2f, -.3f});
        a = copyA; b = copyB;
      }
      compare(a, b, x);
    }
    for (double db : {0., 20., 24.}) for (int offset : {0, 31, 32, 33, 63, 64, 65, 127, 128, 255, 256}) {
      a.Init(rate); b.Init(rate); a.Set(db); b.Set(db); a.SnapToTargets(); b.SnapToTargets();
      for (int n = 0; n < 512; ++n)
        compare(a, b, {n == offset ? .8f : 0.f, n == offset + 1 ? -.6f : 0.f});
    }
    // Reinitialization at another rate erases both histories and gain targets.
    a.Init(rate == 48000 ? 192000 : 48000); b.Init(rate == 48000 ? 192000 : 48000);
    for (int n = 0; n < 256; ++n) compare(a, b, {0.f, -0.f});
  }
  std::cout << Factor << "x object bytes: " << sizeof(Reference) << " -> " << sizeof(Candidate) << '\n';
}

volatile double checksum = 0;
template<class Drive> double Measure(double rate, int voices, double db, int buffer,
                                     const std::vector<Frame>& input) {
  std::array<Drive, 16> bank;
  for (int v = 0; v < voices; ++v) {
    bank[v].Init(rate); bank[v].Set(db); bank[v].SnapToTargets();
    for (int n = 0; n < 1024; ++n) bank[v].Process(input[n]);
  }
  double sum = 0;
  const auto start = std::chrono::steady_clock::now();
  for (int begin = 0; begin < static_cast<int>(input.size()); begin += buffer)
    for (int n = begin; n < std::min(begin + buffer, static_cast<int>(input.size())); ++n)
      for (int v = 0; v < voices; ++v) {
        const auto y = bank[v].Process(input[n]); sum += y[0] + y[1];
      }
  const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
  checksum = checksum + sum;
  return seconds;
}
template<unsigned Factor, template<unsigned> class Oracle = sawstar::test_reference::FixedRatePremiumDrive> void Benchmark() {
  std::vector<Frame> input(16384);
  for (int n = 0; n < static_cast<int>(input.size()); ++n)
    input[n] = {static_cast<float>(.4 * std::sin(n * .057)), static_cast<float>(.4 * std::cos(n * .083))};
  for (double rate : {48000., 96000., 192000.}) for (int voices : {1, 16})
    for (double db : {0., 20., 24.}) for (int buffer : {32, 256}) for (int pair = 0; pair < 6; ++pair) {
      double before, after;
      // Alternate execution order. Timings are diagnostic, not CI thresholds.
      if (pair % 2) {
        after = Measure<sawstar::experimental::FixedRatePremiumDrive<Factor>>(rate, voices, db, buffer, input);
        before = Measure<Oracle<Factor>>(rate, voices, db, buffer, input);
      } else {
        before = Measure<Oracle<Factor>>(rate, voices, db, buffer, input);
        after = Measure<sawstar::experimental::FixedRatePremiumDrive<Factor>>(rate, voices, db, buffer, input);
      }
      std::cout << Factor << ',' << rate << ',' << voices << ',' << db << ',' << buffer << ',' << pair
        << ',' << before << ',' << after << '\n';
    }
}
}
int main(int argc, char** argv) {
  try {
    if (argc == 1) {
      Contract<2>(); Contract<4>();
      Contract<2, sawstar::phase_reference::FixedRatePremiumDrive>();
      Contract<4, sawstar::phase_reference::FixedRatePremiumDrive>();
      std::cout << compared << " stereo frames bit-identical to sparse 715cd47 and compact runtime-phase d4c7822\n";
    } else if (argc == 2 && std::string(argv[1]) == "--benchmark") {
      std::cout << "factor,rate,voices,drive_db,buffer,pair,sparse_seconds,compact_seconds\n";
      Benchmark<2>(); Benchmark<4>();
      if (!std::isfinite(checksum)) throw std::runtime_error("Nonfinite benchmark checksum");
    } else if (argc == 2 && std::string(argv[1]) == "--phase-benchmark") {
      std::cout << "factor,rate,voices,drive_db,buffer,pair,runtime_phase_seconds,static_phase_seconds\n";
      Benchmark<2, sawstar::phase_reference::FixedRatePremiumDrive>();
      Benchmark<4, sawstar::phase_reference::FixedRatePremiumDrive>();
      if (!std::isfinite(checksum)) throw std::runtime_error("Nonfinite phase benchmark checksum");
    } else throw std::runtime_error("Usage: premium_drive_polyphase [--benchmark|--phase-benchmark]");
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
