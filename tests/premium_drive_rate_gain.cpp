// SPDX-License-Identifier: MIT
#include "../experiments/premium_filter/RateScaledPremiumDrive.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
using namespace sawstar::experimental;
using Frame = std::array<float, 2>;
#ifdef SAWSTAR_RATE_LOOKUP_STUDY
using Candidate = RateScaledLookupPremiumDrive;
using Control = RateScaledGainPremiumDrive;
constexpr bool Lookup = true;
#else
using Candidate = RateScaledGainPremiumDrive;
using Control = RateScaledSimdPremiumDrive;
constexpr bool Lookup = false;
#endif
unsigned long long compared = 0;
void Require(bool ok, const char* why) { if (!ok) throw std::runtime_error(why); }
struct Error {
  double squared = 0, energy = 0, peak = 0;
  void Add(Frame actual, Frame expected) {
    for (size_t ch = 0; ch < 2; ++ch) {
      Require(std::isfinite(actual[ch]) && std::isfinite(expected[ch]), "Nonfinite routed normalization");
      const double delta = double(actual[ch]) - expected[ch];
      peak = std::max(peak, std::abs(delta)); squared += delta * delta;
      energy += double(expected[ch]) * expected[ch];
    }
  }
  void Check() const {
    Require(peak <= 2e-7 && (energy > 0 ? std::sqrt(squared / energy) : std::sqrt(squared)) <= 1e-7,
            "Routed normalization exceeds research error bounds");
  }
};
void Equal(Frame a, Frame b) {
  Require(std::memcmp(a.data(), b.data(), sizeof(a)) == 0, "Wrong factor or stale routed state");
}
template<unsigned Factor> void Case(double rate) {
  Candidate candidate;
  Control reference;
  FixedRatePremiumDrive<Factor, true, false, true, true, Lookup> oracle;
  candidate.Init(rate); reference.Init(rate); oracle.Init(rate);
  Require(candidate.Factor() == Factor && reference.Factor() == Factor, "Normalized boundary routing");
  uint32_t random = 31;
  Error error;
  for (int n = 0; n < 4096; ++n) {
    if (n % 79 == 0) { const double db = (n / 79) % 25; candidate.Set(db); reference.Set(db); oracle.Set(db); }
    if (n % 97 == 0) { candidate.SnapToTargets(); reference.SnapToTargets(); oracle.SnapToTargets(); }
    if (n % 521 == 0) { candidate.Clear(); reference.Clear(); oracle.Clear(); }
    Frame x{};
    for (auto& value : x) { random = random * 1664525u + 1013904223u; value = float((random / 4294967296. - .5) * 8); }
    if (n == 777) x[0] = std::numeric_limits<float>::quiet_NaN();
    if (n == 999) x[1] = std::numeric_limits<float>::infinity();
    if (n == 1200) { candidate.Set(NAN); reference.Set(NAN); oracle.Set(NAN); }
    const auto actual = candidate.Process(x), expected = reference.Process(x);
    Equal(actual, oracle.Process(x)); error.Add(actual, expected); ++compared;
    if (n == 2000) {
      auto copy = candidate; auto copyOracle = oracle;
      for (int k = 0; k < 129; ++k) Equal(copy.Process({.2f, -.3f}), copyOracle.Process({.2f, -.3f}));
    }
  }
  error.Check();
  // Unity gain is exactly the unchanged division path, including FIR wrap.
  candidate.Init(rate); reference.Init(rate);
  for (int n = 0; n < 512; ++n) Equal(candidate.Process({.3f, -.2f}), reference.Process({.3f, -.2f}));
  candidate.Clear();
  for (int n = 0; n < 128; ++n) Equal(candidate.Process({0, 0}), {0, 0});
  for (double db : {0., 20., 24.}) {
    candidate.Init(rate); candidate.Set(db); candidate.SnapToTargets();
    int position = -1; float peak = 0;
    for (int n = 0; n < 256; ++n) {
      const auto y = candidate.Process({n == 0 ? 1.f : 0.f, 0});
      Require(y[1] == 0, "Routed impulse crossed channels");
      if (std::abs(y[0]) > peak) { peak = std::abs(y[0]); position = n; }
    }
    Require(position == 32, "Routed normalization changed latency");
  }
}
void Reinitialize() {
  Candidate drive;
  drive.Init(48000); drive.Set(24); drive.SnapToTargets();
  for (int n = 0; n < 257; ++n) drive.Process({.7f, -.5f});
  for (double rate : {192000., 48000., 176400., 176399., 384000., 8000.}) {
    drive.Init(rate); Candidate fresh; fresh.Init(rate);
    for (int n = 0; n < 512; ++n) Equal(drive.Process({.2f, -.3f}), fresh.Process({.2f, -.3f}));
  }
}
}
int main() {
  try {
    static_assert(sizeof(Candidate) == sizeof(Control) + (Lookup ? 2 * sizeof(void*) : 0));
    static_assert(RateScaledGainPremiumDrive::Latency == 32);
    Error wrong; wrong.Add({.1f, 0}, {.09f, 0}); bool rejected = false;
    try { wrong.Check(); } catch (const std::runtime_error&) { rejected = true; }
    Require(rejected, "Error guard accepted altered audio");
    Case<4>(NAN); Case<4>(INFINITY); Case<4>(-INFINITY); Case<4>(-1);
    Case<4>(7999); Case<4>(8000); Case<4>(44100); Case<4>(48000); Case<4>(96000); Case<4>(176399);
    Case<2>(176399.999); Case<2>(176400); Case<2>(176401);
    Case<2>(192000); Case<2>(384000); Case<2>(384001); Case<2>(1e9);
    Reinitialize();
    std::cout << compared << " normalization frames passed routed fixed-factor oracle and division error bounds\n";
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
