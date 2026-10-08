// SPDX-License-Identifier: MIT
#include "../experiments/premium_filter/PremiumDrive.h"
#include "../experiments/premium_filter/LoopReferencePremiumDrive.h"
#include "../experiments/premium_filter/RateScaledPremiumDrive.h"
#include <cmath>
#include <chrono>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <vector>
#include <limits>
#include <stdexcept>

void Require(bool ok) { if (!ok) throw std::runtime_error("Unrolled FIR bit/state contract"); }
template<class A, class B> void Equal(A a, B b) {
  for (int ch = 0; ch < 2; ++ch) {
    Require(std::isfinite(a[ch]) && std::isfinite(b[ch]));
    Require(std::memcmp(&a[ch], &b[ch], sizeof(float)) == 0);
  }
}
template<unsigned Factor> size_t Fixed() {
  using namespace sawstar::experimental;
  using Old = loop_reference::FixedRatePremiumDrive<Factor, true, false, true, true, true>;
  using Current = FixedRatePremiumDrive<Factor, true, false, true, true, true>;
  using Candidate = FixedRatePremiumDrive<Factor, true, false, true, true, true, true>;
  static_assert(sizeof(Old) == sizeof(Candidate) && sizeof(Current) == sizeof(Candidate));
  static_assert(Candidate::Latency == 32);
  size_t frames = 0;
  for (double rate : {44100., 48000., 96000., 192000.}) for (int scene = 0; scene < 6; ++scene) {
    Old old; Current current; Candidate candidate;
    old.Init(rate); current.Init(rate); candidate.Init(rate);
    for (int n = 0; n < 8192; ++n) {
      if (n % 257 == 0) {
        const double db = (n / 257) % 3 * 12.;
        old.Set(db); current.Set(db); candidate.Set(db);
      }
      if (n == 1024) { old.SnapToTargets(); current.SnapToTargets(); candidate.SnapToTargets(); }
      if (n == 4096) { old.Clear(); current.Clear(); candidate.Clear(); }
      std::array<float, 2> x{};
      if (scene == 0) x = {n % 233 == 0 ? .7f : 0.f, 0.f};
      if (scene == 1) x = {float(.7 * std::sin(n * .713)), float(.5 * std::cos(n * .197))};
      if (scene == 2) x = {n % 2 ? .8f : -.8f, n % 3 ? -.4f : .4f};
      if (scene == 3) x = {float(std::sin(n * .071) + .3 * std::cos(n * .431)), float(.2 * std::sin(n * .953))};
      if (scene == 4) x = {float((n % 101 - 50) * .01), float((n % 79 - 39) * .01)};
      if (scene == 5) x = {n % 199 == 0 ? std::numeric_limits<float>::quiet_NaN() : .25f,
                          n % 317 == 0 ? std::numeric_limits<float>::infinity() : -.1f};
      const auto reference = old.Process(x);
      Equal(reference, current.Process(x)); Equal(reference, candidate.Process(x)); ++frames;
      if (n == 3072) {
        auto copyOld = old; auto copyCandidate = candidate;
        for (int i = 0; i < 96; ++i) { Equal(copyOld.Process({.3f, -.2f}), copyCandidate.Process({.3f, -.2f})); ++frames; }
      }
    }
    old.Clear(); candidate.Clear();
    for (int n = 0; n < 96; ++n) { const auto y = candidate.Process({0,0}); Equal(old.Process({0,0}), y); Require(y[0] == 0 && y[1] == 0); ++frames; }
  }
  return frames;
}
template<unsigned Factor, bool Unrolled> void Timing(double rate, int pair) {
  using Drive = sawstar::experimental::FixedRatePremiumDrive<Factor, true, false, true, true, true, Unrolled>;
  constexpr int frames = 8192;
  auto bank = std::make_unique<std::array<Drive, 16>>();
  for (auto& drive : *bank) { drive.Init(rate); drive.Set(20); drive.SnapToTargets(); }
  std::vector<std::array<float, 2>> input(frames);
  for (int i = 0; i < frames; ++i) input[i] = {float(.4 * std::sin(i * .117)), float(.3 * std::cos(i * .173))};
  for (int i = 0; i < int(rate / 4); ++i) for (auto& drive : *bank) drive.Process(input[i % frames]);
  double energy = 0;
  const auto begin = std::chrono::steady_clock::now();
  for (int i = 0; i < frames; ++i) for (auto& drive : *bank) {
    const auto y = drive.Process(input[i]); energy += double(y[0]) * y[0] + double(y[1]) * y[1];
  }
  const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - begin).count();
  Require(std::isfinite(seconds) && seconds > 0 && std::isfinite(energy) && energy > 0);
  std::cout << (Unrolled ? "unrolled" : "loop") << ',' << rate << ',' << Factor << ',' << pair << ",16,20,8192," << seconds << ',' << energy << '\n';
}
template<unsigned Factor> void TimingGrid() {
  for (double rate : {48000., 96000., 192000.}) for (int pair = 0; pair < 4; ++pair) {
    if (pair % 2) { Timing<Factor, true>(rate, pair); Timing<Factor, false>(rate, pair); }
    else { Timing<Factor, false>(rate, pair); Timing<Factor, true>(rate, pair); }
  }
}
int main(int argc, char** argv) {
  try {
    if (argc == 2 && std::string(argv[1]) == "--benchmark") {
      std::cout << std::setprecision(17) << "variant,rate,factor,pair,voices,drive_db,frames,seconds,energy\n";
      TimingGrid<2>(); TimingGrid<4>(); return 0;
    }
    if (argc != 1) throw std::runtime_error("Usage: unrolled [--benchmark]");
    const auto frames = Fixed<2>() + Fixed<4>();
    using namespace sawstar::experimental;
    for (double rate : {48000., 96000., 176399., 176400., 192000., 384000.}) {
      RateScaledUnrolledPremiumDrive candidate; RateScaledLookupPremiumDrive control;
      candidate.Init(rate); control.Init(rate);
      Require(candidate.Factor() == (rate >= 176400 ? 2u : 4u));
      for (int n = 0; n < 4096; ++n) Equal(candidate.Process({float(.4 * std::sin(n * .171)), -.2f}),
                                          control.Process({float(.4 * std::sin(n * .171)), -.2f}));
    }
    std::cout << frames + 6 * 4096 << " stereo frames: frozen/default/unrolled bit identity, copied state, invalid input, clear and routing PASS\n";
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
