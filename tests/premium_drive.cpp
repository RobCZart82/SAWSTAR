// SPDX-License-Identifier: MIT
#include "../experiments/premium_filter/PremiumDrive.h"
#include "../experiments/premium_filter/ReferencePremiumDrive.h"
#include <cstdint>
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
      for (double db : {12., 20., 24.}) {
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
        Require(reduction < (db == 12 ? -60 : -20), "folded third reduction contract (12/20/24 dB)");
      }
      // Reassociated FIR sums have a bounded numerical error to the full reference.
      PremiumDrive optimized; optimized.Init(rate);
      sawstar::experimental::ReferencePremiumDrive reference; reference.Init(rate);
      uint32_t random = 1;
      double maxError = 0, squaredError = 0;
      for (int i = 0; i < 10000; ++i) {
        if (i % 101 == 0) { const double db = (i / 101) % 25; optimized.Set(db); reference.Set(db); }
        if (i % 997 == 0) { optimized.SnapToTargets(); reference.SnapToTargets(); }
        if (i == 5000) { optimized.Clear(); reference.Clear(); }
        std::array<float, 2> x{};
        for (auto& v : x) { random = random * 1664525u + 1013904223u; v = static_cast<float>((random / 4294967296. - .5) * 1.6); }
        if (i == 3333) x[0] = std::numeric_limits<float>::quiet_NaN();
        if (i == 6666) x[1] = std::numeric_limits<float>::infinity();
        const auto actual = optimized.Process(x), expected = reference.Process(x);
        for (size_t ch = 0; ch < 2; ++ch) {
          const double error = static_cast<double>(actual[ch]) - expected[ch];
          maxError = std::max(maxError, std::abs(error)); squaredError += error * error;
          Require(std::abs(static_cast<double>(actual[ch]) - expected[ch]) < 1e-7,
                  "optimized Drive absolute error below -140 dBFS");
        }
      }
      std::cout << "rate=" << rate << ", reference max error=" << maxError
                << ", RMS error=" << std::sqrt(squaredError / 20000) << '\n';
      Require(squaredError / 20000 < 1e-16, "reference RMS error below -160 dBFS");
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
