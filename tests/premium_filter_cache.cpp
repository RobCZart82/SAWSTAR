// SPDX-License-Identifier: MIT
#include "../experiments/premium_filter/PremiumLowPass.h"
#include "../experiments/premium_filter/ReferencePremiumFilter.h"
#include <chrono>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using Candidate = sawstar::experimental::PremiumLowPass;
using Reference = sawstar::experimental::ReferencePremiumFilter;
namespace {
std::array<float, 2> Input(int n) {
  return {static_cast<float>(.2 * std::sin(n * .173)),
          static_cast<float>(.3 * std::cos(n * .031))};
}
void Contract() {
  unsigned long long frames = 0;
  for (double rate : {8000., 44100., 48000., 96000., 192000., 384000.})
    for (int mode = 0; mode < 4; ++mode) {
      Candidate a; Reference b;
      a.Init(rate); b.Init(rate);
      a.SetMode(mode); b.SetMode(mode);
      auto compare = [&](int n) {
        auto input = Input(n);
        if (n == 131) input[0] = std::numeric_limits<float>::quiet_NaN();
        if (n == 132) input[1] = std::numeric_limits<float>::infinity();
        const auto x = a.Process(input), y = b.Process(input);
        if (std::memcmp(x.data(), y.data(), sizeof(x)) != 0)
          throw std::runtime_error("Cache changed samples: rate=" + std::to_string(rate)
            + " mode=" + std::to_string(mode) + " frame=" + std::to_string(n));
        ++frames;
      };
      // Hold beyond floating-point convergence, then invalidate the cache.
      a.Set(1800, 50); b.Set(1800, 50);
      for (int n = 0; n < static_cast<int>(rate); ++n) compare(n);
      for (int n = 0; n < 32768; ++n) {
        if (n % 128 == 0) {
          const double cutoff = n % 256 ? 20 : 20000;
          const double resonance = n % 384 ? 100 : 0;
          a.Set(cutoff, resonance); b.Set(cutoff, resonance);
          a.SetMode((n / 128) % 4); b.SetMode((n / 128) % 4);
        }
        if (n == 777) { a.SnapToTargets(); b.SnapToTargets(); }
        if (n == 999) { a.Clear(); b.Clear(); }
        if (n == 1234) {
          a.Set(std::numeric_limits<double>::quiet_NaN(), -1);
          b.Set(std::numeric_limits<double>::quiet_NaN(), -1);
        }
        compare(n);
      }
      // Snap must refresh coefficients even after a settled, copied state.
      a.Set(900, 90); b.Set(900, 90); a.SnapToTargets(); b.SnapToTargets();
      Candidate copy = a; Reference oracleCopy = b; a = copy; b = oracleCopy;
      for (int n = 0; n < 1024; ++n) compare(n);
      // Reinit at another rate must not retain coefficients or cache flags.
      a.Init(rate == 48000 ? 192000 : 48000); b.Init(rate == 48000 ? 192000 : 48000);
      for (int n = 0; n < 1024; ++n) compare(n);
    }
  std::cout << frames << " stereo frames bit-identical to uncached oracle\n";
}

volatile double checksum = 0;
template<class Filter> double Measure(double rate, int mode, bool modulation,
                                      const std::vector<std::array<float, 2>>& input) {
  std::array<Filter, 16> bank;
  for (auto& f : bank) {
    f.Init(rate); f.Set(1800, 50); f.SetMode(mode); f.SnapToTargets();
    for (int n = 0; n < 4096; ++n) f.Process(input[n]);
  }
  double sum = 0;
  const auto start = std::chrono::steady_clock::now();
  for (int n = 0; n < static_cast<int>(input.size()); ++n) for (auto& f : bank) {
    if (modulation && n % 128 == 0) f.Set(n % 256 ? 500 : 12000, n % 384 ? 50 : 100);
    const auto y = f.Process(input[n]); sum += y[0] + y[1];
  }
  const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
  checksum = checksum + sum;
  return seconds;
}
void Benchmark() {
  std::vector<std::array<float, 2>> input(32768);
  for (int n = 0; n < static_cast<int>(input.size()); ++n) input[n] = Input(n);
  std::cout << "rate,mode,modulated,pair,reference_seconds,cached_seconds\n";
  for (double rate : {48000., 96000., 192000.}) for (int mode = 0; mode < 4; ++mode)
    for (bool mod : {false, true}) for (int pair = 0; pair < 8; ++pair) {
      double a, b;
      // Alternate execution order; time is diagnostic, never a CI threshold.
      if (pair % 2) {
        b = Measure<Candidate>(rate, mode, mod, input);
        a = Measure<Reference>(rate, mode, mod, input);
      } else {
        a = Measure<Reference>(rate, mode, mod, input);
        b = Measure<Candidate>(rate, mode, mod, input);
      }
      std::cout << rate << ',' << mode << ',' << mod << ',' << pair << ',' << a << ',' << b << '\n';
    }
  if (!std::isfinite(checksum)) throw std::runtime_error("Nonfinite benchmark checksum");
}
}
int main(int argc, char** argv) {
  try {
    if (argc == 1) Contract();
    else if (argc == 2 && std::string(argv[1]) == "--benchmark") Benchmark();
    else throw std::runtime_error("Usage: premium_filter_cache [--benchmark]");
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
