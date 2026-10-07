// SPDX-License-Identifier: MIT
// Qualify the opt-in reciprocal path; default division remains unchanged.
#include "../experiments/premium_filter/PremiumDrive.h"
#ifdef SAWSTAR_GAIN_ADAPTER_CONTRACT
#include "../experiments/premium_filter/EnginePremiumFilter.h"
#endif
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
using Frame = std::array<float, 2>;
constexpr double PeakLimit = 2e-7, RmsLimit = 1e-7;
struct Error {
  double squared = 0, energy = 0, peak = 0;
  unsigned long long frames = 0, differing = 0;
  void Add(Frame a, Frame b) {
    if (std::memcmp(a.data(), b.data(), sizeof(a))) ++differing;
    for (size_t ch = 0; ch < 2; ++ch) {
      if (!std::isfinite(a[ch]) || !std::isfinite(b[ch])) throw std::runtime_error("Nonfinite normalization output");
      const double delta = double(a[ch]) - b[ch];
      peak = std::max(peak, std::abs(delta)); squared += delta * delta;
      energy += double(b[ch]) * b[ch];
    }
    ++frames;
  }
  double Rms() const { return energy > 0 ? std::sqrt(squared / energy) : std::sqrt(squared); }
  void Check(double peakLimit = PeakLimit, double rmsLimit = RmsLimit) const {
    if (peak > peakLimit || Rms() > rmsLimit)
      throw std::runtime_error("Reciprocal normalization exceeds numerical research limits");
  }
};

template<unsigned Factor, bool Vector, bool Research = false> void Contract() {
  using Reference = sawstar::experimental::FixedRatePremiumDrive<Factor, true, Research, Vector>;
  using Candidate = sawstar::experimental::FixedRatePremiumDrive<Factor, true, Research, Vector, true>;
  static_assert(sizeof(Reference) == sizeof(Candidate), "No extra persistent state");
  static_assert(Reference::Latency == 32 && Candidate::Latency == 32);
  Error all;
  for (double rate : {8000., 44100., 48000., 96000., 192000., 384000.}) {
    for (int scene = 0; scene < 4; ++scene) {
      Reference b; Candidate a; a.Init(rate); b.Init(rate);
      Error sceneError;
      auto compare = [&](Frame x) {
        const auto actual = a.Process(x), expected = b.Process(x);
        all.Add(actual, expected); sceneError.Add(actual, expected);
      };
      uint32_t random = 19;
      for (int n = 0; n < 8192; ++n) {
        if (n % 79 == 0) { const double db = (n / 79) % 25; a.Set(db); b.Set(db); }
        if (n % 997 == 0) { a.SnapToTargets(); b.SnapToTargets(); }
        if (n % 4093 == 0) { a.Clear(); b.Clear(); }
        Frame x{};
        for (int ch = 0; ch < 2; ++ch) {
          random = random * 1664525u + 1013904223u;
          if (scene == 0) x[ch] = float((random / 4294967296. - .5) * 8.);
          if (scene == 1) x[ch] = float(.9 * std::sin(n * (.13 + ch * .077)));
          if (scene == 2) x[ch] = (n + ch) % 127 == 0 ? 1.f : 0.f;
          if (scene == 3) x[ch] = float((random / 4294967296. - .5) * 1e-30);
        }
        if (n % 1031 == 0) x[0] = std::numeric_limits<float>::quiet_NaN();
        if (n % 2053 == 0) x[1] = std::numeric_limits<float>::infinity();
        if (n == 3000) { a.Set(std::numeric_limits<double>::quiet_NaN()); b.Set(std::numeric_limits<double>::quiet_NaN()); }
        if (n == 2000) {
          auto ca = a; auto cb = b;
          for (int k = 0; k < 257; ++k) {
            const auto actual = ca.Process({.2f, -.3f}), expected = cb.Process({.2f, -.3f});
            all.Add(actual, expected); sceneError.Add(actual, expected);
          }
          a = ca; b = cb;
        }
        compare(x);
      }
      // Each rate/scene must pass separately, including very quiet input.
      sceneError.Check();
      a.Init(rate); b.Init(rate);
      for (int n = 0; n < 256; ++n) {
        const Frame x{float(.3 * std::sin(n * .1)), -.2f};
        const auto actual = a.Process(x), expected = b.Process(x);
        // Unity gain bypasses normalization: require exact default behavior.
        if (std::memcmp(actual.data(), expected.data(), sizeof(actual)))
          throw std::runtime_error("Unity-gain normalization differs");
        all.Add(actual, expected);
      }
      a.Clear(); b.Clear();
      for (int n = 0; n < 64; ++n) {
        const auto actual = a.Process({0, 0}), expected = b.Process({0, 0});
        if (actual != Frame{} || expected != Frame{}) throw std::runtime_error("Clear did not silence normalization state");
        all.Add(actual, expected);
      }
      for (double db : {0., 20., 24.}) {
        a.Init(rate); b.Init(rate); a.Set(db); b.Set(db); a.SnapToTargets(); b.SnapToTargets();
        int peakAt = -1; float peak = 0;
        for (int n = 0; n < 256; ++n) {
          const Frame x{n == 0 ? 1.f : 0.f, 0};
          const auto actual = a.Process(x), expected = b.Process(x);
          if (actual[1] != 0 || expected[1] != 0) throw std::runtime_error("Impulse crossed channels");
          if (std::abs(actual[0]) > peak) { peak = std::abs(actual[0]); peakAt = n; }
          all.Add(actual, expected);
        }
        if (peakAt != 32) throw std::runtime_error("Normalization changed latency");
      }
    }
  }
  all.Check();
  std::cout << Factor << "x vector=" << Vector << " research_tanh=" << Research
            << " frames=" << all.frames << " differing=" << all.differing
            << " peak=" << all.peak << " relative_rms=" << all.Rms() << '\n';
}
#ifdef SAWSTAR_GAIN_ADAPTER_CONTRACT
void AdapterContract() {
  Error all;
  for (double rate : {8000., 44100., 48000., 96000., 192000., 384000.}) for (int mode = 0; mode < 4; ++mode) {
    sawstar::experimental::SimdStudyEnginePremiumFilter b;
    sawstar::experimental::GainStudyEnginePremiumFilter a;
    a.Init(float(rate)); b.Init(float(rate));
    a.Set(1800, 50, 100); b.Set(1800, 50, 100);
    a.SetCharacter(20, mode); b.SetCharacter(20, mode); a.SnapToTargets(); b.SnapToTargets();
    Error scene;
    for (int n = 0; n < 4096; ++n) {
      if (n % 127 == 0) { const float mix = float((n / 127) % 3 * 50); a.Set(1800, 50, mix); b.Set(1800, 50, mix); }
      if (n % 521 == 0) { a.SetCharacter(20, (mode + n / 521) % 4); b.SetCharacter(20, (mode + n / 521) % 4); }
      if (n == 3000) { a.Clear(); b.Clear(); }
      sawstar::StereoSample input{float(.4 * std::sin(n * .17)), float(.3 * std::cos(n * .23))};
      if (n == 1031) input.left = std::numeric_limits<float>::quiet_NaN();
      const auto actual = a.Process(input), expected = b.Process(input);
      const Frame x{actual.left, actual.right}, y{expected.left, expected.right};
      all.Add(x, y); scene.Add(x, y);
    }
    scene.Check(2e-6);
  }
  all.Check(2e-6);
  std::cout << "adapter frames=" << all.frames << " differing=" << all.differing
            << " peak=" << all.peak << " relative_rms=" << all.Rms() << '\n';
}
#endif
}
int main() {
  try {
    // A visibly altered output must fail the numerical contract.
    Error wrong; wrong.Add({.1f, 0}, {.09f, 0});
    bool rejected = false;
    try { wrong.Check(); } catch (const std::runtime_error&) { rejected = true; }
    if (!rejected) throw std::runtime_error("Numerical guard cannot reject altered audio");
    std::cout << std::setprecision(17);
    Contract<2, false>(); Contract<2, true>(); Contract<4, false>(); Contract<4, true>();
    Contract<2, false, true>(); Contract<2, true, true>();
    Contract<4, false, true>(); Contract<4, true, true>();
#ifdef SAWSTAR_GAIN_ADAPTER_CONTRACT
    AdapterContract();
#endif
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
