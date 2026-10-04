// SPDX-License-Identifier: MIT
#include "../experiments/premium_filter/PremiumLowPass.h"
#include <complex>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace {
using Filter = sawstar::experimental::PremiumLowPass;
constexpr double pi = 3.14159265358979323846;
void Check(bool ok, const char* message) {
  if (!ok) { std::cerr << message << '\n'; std::exit(1); }
}
// Independent bilinear transform of the continuous transfer functions.
// This checks phase as well as magnitude, including HP and normalized BP.
std::complex<double> Expected(double rate, double cutoff, double hz, double res, int mode) {
  const double r = std::tan(pi * hz / rate) / std::tan(pi * cutoff / rate);
  if (mode == 1) {
    const double k = 1.8477590650225735 * std::pow(.1 / 1.8477590650225735, res / 100.);
    return 1. / (std::complex<double>(1 - r * r, k * r)
      * std::complex<double>(1 - r * r, .7653668647301795 * r));
  }
  const double k = std::sqrt(2.) * std::pow(.1 / std::sqrt(2.), res / 100.);
  const std::complex<double> d(1 - r * r, k * r);
  if (mode == 0) return 1. / d;
  if (mode == 2) return -r * r / d;
  return std::complex<double>(0, k * r) / d;
}
std::complex<double> Measure(double rate, double hz, double res, int mode) {
  Filter f; f.Init(rate); f.Set(1000, res); f.SetMode(mode); f.SnapToTargets();
  double cosine = 0, sine = 0;
  const int frames = static_cast<int>(rate);
  for (int n = 0; n < frames; ++n) {
    const double phase = 2 * pi * hz * n / rate;
    const float x = static_cast<float>(std::cos(phase));
    const auto y = f.Process({x, 0});
    Check(y[1] == 0, "response right channel isolation");
    if (n >= frames / 2) { cosine += y[0] * std::cos(phase); sine += y[0] * std::sin(phase); }
  }
  return {4 * cosine / frames, -4 * sine / frames};
}
void Transitions(double rate) {
  std::array<Filter, 4> locked;
  Filter switching; switching.Init(rate);
  for (int mode = 0; mode < 4; ++mode) {
    locked[mode].Init(rate); locked[mode].SetMode(mode); locked[mode].SnapToTargets();
  }
  std::array<double, 4> weights{0, 1, 0, 0};
  const double slew = 1 - std::exp(-1 / (.01 * rate));
  for (int n = 0; n < 32768; ++n) {
    const int mode = (n / 97) % 4;
    const double cutoff = (n / 521) % 2 ? 20 : 20000;
    const double res = (n / 193) % 2 ? 100 : 0;
    switching.Set(cutoff, res); switching.SetMode(mode);
    const float x = static_cast<float>(.1 * (std::sin(n * .17) + std::sin(n * .031)));
    std::array<double, 2> expected{};
    for (int m = 0; m < 4; ++m) {
      locked[m].Set(cutoff, res);
      const auto y = locked[m].Process({x, -x});
      weights[m] += slew * ((m == mode ? 1. : 0.) - weights[m]);
      Check(weights[m] >= 0 && weights[m] <= 1, "mode weights remain convex");
      for (int ch = 0; ch < 2; ++ch) expected[ch] += weights[m] * y[ch];
    }
    const auto actual = switching.Process({x, -x});
    Check(std::abs(weights[0] + weights[1] + weights[2] + weights[3] - 1) < 1e-12,
          "mode weights sum to unity");
    for (int ch = 0; ch < 2; ++ch)
      Check(std::isfinite(actual[ch]) && std::abs(actual[ch]) < 100
        && std::abs(actual[ch] - expected[ch]) < 2e-6,
        "mode transition matches independently running warm networks");
    Check(actual[0] == -actual[1], "mode transition stereo symmetry");
  }
  switching.Clear();
  Check(switching.Process({0, 0}) == std::array<float, 2>{0, 0}, "clear erases all mode histories");
}
void Lifecycle(double rate, int mode) {
  Filter f; f.Init(rate); f.Set(1000, 100); f.SetMode(mode); f.SnapToTargets();
  for (int n = 0; n < 256; ++n) f.Process({.1f, -.2f});
  Filter untouched = f;
  const auto invalid = f.Process({std::numeric_limits<float>::quiet_NaN(), .1f});
  Check(invalid[0] == 0 && invalid[1] == untouched.Process({0, .1f})[1],
        "invalid sample clears only affected mode histories");
  Check(f.Process({0, .1f})[0] == 0, "affected channel remains clear");
  f.Set(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN());
  Check(std::isfinite(f.Process({.1f, .1f})[0]), "invalid controls sanitized in every mode");
  f.Init(rate);
  Filter fresh; fresh.Init(rate);
  for (int n = 0; n < 256; ++n)
    Check(f.Process({.1f, -.1f}) == fresh.Process({.1f, -.1f}), "Init resets mode and caches");
  f.SetMode(mode); f.SnapToTargets(); f.Clear();
  for (int n = 0; n < 2048; ++n) f.Process({n == 0 ? .1f : 0.f, 0});
  for (int n = 0; n < static_cast<int>(rate / 2); ++n) f.Process({0, 0});
  Check(std::abs(f.Process({0, 0})[0]) < 1e-18, "mode impulse decays without self oscillation");
  f.Set(1000, 100); f.SetMode(mode); f.SnapToTargets(); f.Clear();
  untouched = f;
  for (int n = 0; n < static_cast<int>(rate / 10); ++n) {
    const float extreme = std::numeric_limits<float>::max()
      * static_cast<float>(std::cos(2 * pi * 1000 * n / rate));
    const auto y = f.Process({extreme, .01f});
    Check(std::isfinite(y[0]) && y[1] == untouched.Process({0, .01f})[1],
          "extreme finite input cannot poison output or other channel");
  }
}
}
int main() {
  std::cout << "rate,mode,resonance,hz,measured_gain,expected_gain,complex_error\n";
  for (double rate : {8000., 44100., 48000., 96000., 192000., 384000.}) {
    for (int mode = 0; mode < 4; ++mode) {
      for (double res : {0., 50., 100.}) for (double hz : {100., 800., 1000., 1400., 3000.}) {
        const auto actual = Measure(rate, hz, res, mode);
        const auto expected = Expected(rate, 1000, hz, res, mode);
        Check(std::abs(actual - expected) < 2e-4, "complex transfer function mismatch");
        std::cout << rate << ',' << mode << ',' << res << ',' << hz << ','
          << std::abs(actual) << ',' << std::abs(expected) << ',' << std::abs(actual - expected) << '\n';
      }
      Lifecycle(rate, mode);
    }
    Transitions(rate);
    for (int invalid : {-999, 999}) {
      Filter a, b; a.Init(rate); b.Init(rate);
      a.SetMode(invalid); b.SetMode(invalid < 0 ? 0 : 3);
      a.SnapToTargets(); b.SnapToTargets();
      for (int n = 0; n < 128; ++n)
        Check(a.Process({.1f, 0}) == b.Process({.1f, 0}), "mode clamps to existing parameter range");
    }
  }
}
