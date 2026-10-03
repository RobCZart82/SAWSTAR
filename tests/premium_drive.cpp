// SPDX-License-Identifier: MIT
#include "../experiments/premium_filter/PremiumDrive.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
using sawstar::experimental::PremiumDrive;
void Require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
int main() {
  try {
    constexpr double pi = 3.14159265358979323846;
    for (double rate : {44100., 48000., 96000., 192000.}) {
      for (double db : {12., 24.}) {
        PremiumDrive drive; drive.Init(rate); drive.Set(db); drive.SnapToTargets();
        // Coherent sinusoid: third harmonic folds from 9/16 to 7/16 at host rate.
        double rawC = 0, rawS = 0, overC = 0, overS = 0;
        constexpr int warmup = 1024, length = 8192;
        for (int i = 0; i < warmup + length; ++i) {
          const float x = static_cast<float>(.75 * std::sin(2 * pi * 3 * i / 16));
          const auto y = drive.Process({x, 0});
          Require(std::isfinite(y[0]) && y[1] == 0, "finite stereo-isolated output");
          if (i >= warmup) {
            const double angle = 2 * pi * 7 * i / 16;
            const double raw = std::tanh(std::pow(10., db / 20) * x) / std::pow(10., db / 20);
            rawC += raw * std::cos(angle); rawS += raw * std::sin(angle);
            overC += y[0] * std::cos(angle); overS += y[0] * std::sin(angle);
          }
        }
        const double reduction = 20 * std::log10(std::hypot(overC, overS) / std::hypot(rawC, rawS));
        std::cout << "drive=" << db << ", rate=" << rate << ", folded-third change dB=" << reduction << '\n';
        Require(reduction < (db == 12 ? -60 : -20), "folded third reduction contract (12/24 dB)");
      }
      PremiumDrive drive;
      drive.Init(rate);
      double peak = 0; int position = -1;
      for (int i = 0; i < 100; ++i) {
        const auto y = drive.Process({i == 0 ? 1.f : 0.f, 0});
        if (std::abs(y[0]) > peak) { peak = std::abs(y[0]); position = i; }
      }
      Require(position == PremiumDrive::Latency, "linear impulse delay contract");
      drive.Clear();
      for (int i = 0; i < 100; ++i) Require(drive.Process({0, 0})[0] == 0, "Clear erases FIR state");
      drive.Set(std::numeric_limits<double>::quiet_NaN());
      drive.Process({std::numeric_limits<float>::infinity(), 0});
      for (int i = 0; i < 1000; ++i) Require(std::isfinite(drive.Process({.1f, -.1f})[0]), "invalid input recovery");
      drive.Init(rate); drive.Set(12); drive.SnapToTargets();
      for (int i = 0; i < 256; ++i) drive.Process({.1f, -.1f});
      auto intact = drive;
      const auto bad = drive.Process({std::numeric_limits<float>::quiet_NaN(), -.1f});
      const auto good = intact.Process({.1f, -.1f});
      Require(bad[1] == good[1], "invalid left input preserves right history");
      for (int i = 0; i < 2048; ++i) {
        drive.Set((i % 17) == 0 ? 24 : 0);
        const auto y = drive.Process({.8f, -.8f});
        Require(std::isfinite(y[0]) && std::isfinite(y[1]), "modulated Drive remains finite");
      }
      // Unity/phase contract in the flat passband at zero Drive.
      for (double fraction : {.01, .1, .25, .33}) {
        drive.Init(rate); double error = 0, energy = 0;
        for (int i = 0; i < 4096; ++i) {
          const float x = static_cast<float>(.1 * std::sin(2 * pi * fraction * i));
          const float y = drive.Process({x, x})[0];
          if (i > 256) {
            const double reference = .1 * std::sin(2 * pi * fraction * (i - PremiumDrive::Latency));
            error += (y - reference) * (y - reference); energy += reference * reference;
          }
        }
        Require(error / energy < 1e-6, "zero Drive delayed passband accuracy");
      }
    }
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
