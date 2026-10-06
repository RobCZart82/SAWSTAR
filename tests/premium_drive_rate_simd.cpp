// SPDX-License-Identifier: MIT
#include "../experiments/premium_filter/RateScaledPremiumDrive.h"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
using namespace sawstar::experimental;
using Frame = std::array<float, 2>;
unsigned long long compared = 0;
void Require(bool ok, const char* message) {
  if (!ok) throw std::runtime_error(message);
}
void Equal(Frame a, Frame b) {
  Require(std::isfinite(a[0]) && std::isfinite(a[1]) &&
          std::isfinite(b[0]) && std::isfinite(b[1]), "nonfinite routed output");
  Require(std::memcmp(a.data(), b.data(), sizeof(a)) == 0, "routed samples differ");
}
template<unsigned Factor> void Case(double rate) {
  RateScaledPremiumDrive scalar;
  RateScaledSimdPremiumDrive simd;
  FixedRatePremiumDrive<Factor> oracle;
  scalar.Init(rate); simd.Init(rate); oracle.Init(rate);
  Require(scalar.Factor() == Factor && simd.Factor() == Factor, "normalized rate routing");
  auto compare = [&](Frame x) {
    const auto expected = oracle.Process(x);
    Equal(scalar.Process(x), expected);
    Equal(simd.Process(x), expected);
    ++compared;
  };
  uint32_t random = 7;
  for (int n = 0; n < 4096; ++n) {
    if (n % 79 == 0) {
      const double db = (n / 79) % 25;
      scalar.Set(db); simd.Set(db); oracle.Set(db);
    }
    if (n % 97 == 0) { scalar.SnapToTargets(); simd.SnapToTargets(); oracle.SnapToTargets(); }
    if (n % 521 == 0) { scalar.Clear(); simd.Clear(); oracle.Clear(); }
    Frame x{};
    for (auto& v : x) {
      random = random * 1664525u + 1013904223u;
      v = static_cast<float>((random / 4294967296. - .5) * 3.);
    }
    if (n == 777) x[0] = std::numeric_limits<float>::quiet_NaN();
    if (n == 999) x[1] = std::numeric_limits<float>::infinity();
    if (n == 1200) {
      scalar.Set(std::numeric_limits<double>::infinity());
      simd.Set(std::numeric_limits<double>::infinity());
      oracle.Set(std::numeric_limits<double>::infinity());
    }
    compare(x);
    if (n == 2000) {
      auto copyScalar = scalar; auto copySimd = simd; auto copyOracle = oracle;
      for (int k = 0; k < 129; ++k) {
        const Frame input{.2f, -.3f};
        const auto expected = copyOracle.Process(input);
        Equal(copyScalar.Process(input), expected); Equal(copySimd.Process(input), expected);
        ++compared;
      }
      // Processing copies must not advance the original histories or gain.
    }
  }
  scalar.Clear(); simd.Clear(); oracle.Clear();
  for (int n = 0; n < 64; ++n) {
    Equal(scalar.Process({0, 0}), {0, 0});
    Equal(simd.Process({0, 0}), {0, 0});
    Equal(oracle.Process({0, 0}), {0, 0});
  }
  scalar.Init(rate); simd.Init(rate);
  const auto impulse = [&](auto& drive) {
    int position = -1; float peak = 0;
    for (int n = 0; n < 128; ++n) {
      const auto y = drive.Process({n == 0 ? 1.f : 0.f, 0});
      Require(y[1] == 0, "routed impulse stereo isolation");
      if (std::abs(y[0]) > peak) { peak = std::abs(y[0]); position = n; }
    }
    Require(position == 32, "routed latency");
  };
  impulse(scalar); impulse(simd);
}
void Reinitialize() {
  RateScaledPremiumDrive scalar;
  RateScaledSimdPremiumDrive simd;
  scalar.Init(48000); simd.Init(48000); scalar.Set(24); simd.Set(24);
  for (int n = 0; n < 257; ++n) { scalar.Process({.7f, -.5f}); simd.Process({.7f, -.5f}); }
  // Both directions, repeated live-state reinitialization, never live rate switching.
  for (double rate : {192000., 48000., 176400., 176399., 384000., 8000.}) {
    scalar.Init(rate); simd.Init(rate);
    RateScaledPremiumDrive fresh; fresh.Init(rate);
    for (int n = 0; n < 512; ++n) {
      const Frame x{float(.3 * std::sin(n * .13)), float(.4 * std::cos(n * .19))};
      const auto expected = fresh.Process(x);
      Equal(scalar.Process(x), expected); Equal(simd.Process(x), expected);
      ++compared;
    }
  }
}
}
int main() {
  try {
    static_assert(sizeof(RateScaledPremiumDrive) == sizeof(RateScaledSimdPremiumDrive));
    static_assert(RateScaledPremiumDrive::Latency == RateScaledSimdPremiumDrive::Latency);
    Case<4>(std::numeric_limits<double>::quiet_NaN());
    Case<4>(std::numeric_limits<double>::infinity());
    Case<4>(-std::numeric_limits<double>::infinity());
    Case<4>(-1); Case<4>(7999); Case<4>(8000);
    Case<4>(175999); Case<4>(176399);
    // SafeSampleRate normalizes to float: this rounds to exactly 176400.
    Case<2>(176399.999); Case<2>(176400); Case<2>(176401);
    Case<2>(192000); Case<2>(384000); Case<2>(384001); Case<2>(1e9);
    Reinitialize();
    std::cout << compared << " routed stereo frames bit-identical; backend "
              << sawstar::experimental::detail::FirLanes4::Backend << '\n';
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
