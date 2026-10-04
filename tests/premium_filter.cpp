// SPDX-License-Identifier: MIT
#include "../experiments/premium_filter/PremiumLowPass.h"
#include "dsp/LowPass.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace {
constexpr double pi = 3.14159265358979323846;
void Check(bool ok, const char* message) {
  if (!ok) { std::cerr << message << '\n'; std::exit(1); }
}
double Gain(double sr, double cutoff, double hz, double resonance = 0) {
  sawstar::experimental::PremiumLowPass filter;
  filter.Init(sr); filter.Set(cutoff, resonance); filter.SnapToTargets();
  double output = 0, input = 0;
  const int count = static_cast<int>(sr);
  for (int n = 0; n < count; ++n) {
    const float x = static_cast<float>(std::sin(2 * pi * hz * n / sr));
    const auto y = filter.Process({x, 0});
    Check(y[1] == 0, "right channel must remain silent");
    if (n >= count / 2) { output += y[0] * y[0]; input += x * x; }
  }
  return std::sqrt(output / input);
}
double ExpectedGain(double sr, double cutoff, double hz) {
  const double ratio = std::tan(pi * hz / sr) / std::tan(pi * cutoff / sr);
  return 1. / std::sqrt(1. + std::pow(ratio, 8));
}
}
int main() {
  std::cout << "sample_rate,legacy_lp24_cutoff_db,candidate_cutoff_db\n";
  for (double sr : {8000., 44100., 48000., 96000., 192000., 384000.}) {
    for (double hz : {100., 1000., 2000., 3000.}) {
      const double measured = Gain(sr, 1000, hz);
      Check(std::abs(measured - ExpectedGain(sr, 1000, hz)) < 2e-5,
            "response must match independent Butterworth transfer function");
    }
    // Independent response for both quadratic factors, including resonance.
    for (double resonance : {25., 50., 75., 100.}) {
      const double k1 = 1.8477590650225735 * std::pow(.1 / 1.8477590650225735, resonance / 100.);
      constexpr double k2 = .7653668647301795;
      for (double hz : {100., 800., 1000., 1400., 3000.}) {
        const double ratio = std::tan(pi * hz / sr) / std::tan(pi * 1000 / sr);
        const double squared = (1 - ratio * ratio) * (1 - ratio * ratio);
        const double expected = 1. / std::sqrt((squared + k1 * k1 * ratio * ratio)
                                               * (squared + k2 * k2 * ratio * ratio));
        Check(std::abs(Gain(sr, 1000, hz, resonance) - expected) < 2e-4,
              "resonant response must match independent transfer function");
      }
    }
    sawstar::LowPass old; old.Init(static_cast<float>(sr));
    old.Set(1000, 0, 100); old.SetCharacter(0, 1); old.SnapToTargets();
    double energy = 0, input = 0;
    for (int i = 0; i < static_cast<int>(sr); ++i) {
      const float x = static_cast<float>(std::sin(2 * pi * 1000 * i / sr));
      const auto y = old.Process({x, 0});
      if (i >= sr / 2) { energy += y.left * y.left; input += x * x; }
    }
    Check(std::abs(std::sqrt(energy / input) - .25) < 2e-5,
          "legacy LP24 cutoff characterization must remain reproducible");
    std::cout << sr << ',' << 10 * std::log10(energy / input) << ','
              << 20 * std::log10(Gain(sr, 1000, 1000)) << '\n';
    sawstar::experimental::PremiumLowPass f; f.Init(sr);
    for (int block = 0; block < 400; ++block) {
      f.Set(block % 2 ? 20 : 20000, block % 3 ? 100 : 0);
      for (int i = 0; i < 128; ++i) {
        const float x = static_cast<float>(.2 * std::sin((block * 128 + i) * .19));
        const auto y = f.Process({x, -x});
        Check(std::isfinite(y[0]) && std::abs(y[0]) < 100 && y[0] == -y[1],
              "rapid cutoff/resonance modulation must stay finite and symmetric");
      }
    }
    // Init must invalidate coefficient caches even after a nondefault patch.
    f.Set(900, 50); f.SnapToTargets(); f.Process({.3f, -.3f});
    f.Init(sr);
    sawstar::experimental::PremiumLowPass fresh; fresh.Init(sr);
    for (int i = 0; i < 256; ++i) {
      const float x = static_cast<float>(.1 * std::sin(i * .17));
      f.Set(12000, 0); // Synth publishes unchanged controls every sample.
      Check(f.Process({x, -x}) == fresh.Process({x, -x}),
            "cached controls and reinitialization preserve fresh filter output");
    }
    f.Clear(); Check(f.Process({0, 0})[0] == 0, "clear must erase both stages");
    f.Process({.2f, .2f});
    const auto recovered = f.Process({std::numeric_limits<float>::quiet_NaN(), .2f});
    Check(recovered[0] == 0 && std::isfinite(recovered[1]), "invalid input isolation");
    Check(f.Process({0, 0})[0] == 0, "invalid input clears affected histories");
    f.Set(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN());
    Check(std::isfinite(f.Process({.2f, .2f})[0]), "invalid controls recover");
  }
  std::cout << "resonance_percent,low_100hz_gain_db,cutoff_1000hz_gain_db\n";
  for (double r : {0., 10., 30., 50., 70., 90., 100.})
    std::cout << r << ',' << 20 * std::log10(Gain(48000, 1000, 100, r))
              << ',' << 20 * std::log10(Gain(48000, 1000, 1000, r)) << '\n';
}
