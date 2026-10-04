// SPDX-License-Identifier: MIT
#include "../experiments/premium_filter/RateScaledPremiumDrive.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace sawstar::experimental;
constexpr double pi = 3.14159265358979323846;
using Period = std::array<double, 1024>;
double BinPower(const Period& signal, int bin) {
  double c = 0, s = 0;
  for (int i = 0; i < 1024; ++i) {
    const double angle = 2 * pi * bin * i / 1024;
    c += signal[i] * std::cos(angle); s += signal[i] * std::sin(angle);
  }
  return (c * c + s * s) / (1024. * 1024.);
}
double AudibleResidual(const Period& signal, int fundamental, double rate) {
  // Selected folded odd harmonics up to order 63, excluding bins coinciding
  // with legitimate harmonics. Higher orders / coincident aliases are omitted.
  std::array<bool, 513> bins{};
  for (int harmonic = 3; harmonic <= 63; harmonic += 2) {
    int bin = (harmonic * fundamental) % 1024;
    if (bin > 512) bin = 1024 - bin;
    if (bin > 0 && bin < 512 && bin % fundamental != 0 && rate * bin / 1024 <= 20000)
      bins[bin] = true;
  }
  double power = 0;
  for (int bin = 1; bin < 512; ++bin) if (bins[bin]) power += 2 * BinPower(signal, bin);
  return power;
}
void Require(bool ok, const char* text) { if (!ok) throw std::runtime_error(text); }
int main() {
  try {
    // The established 4x branch must remain the original oracle contract.
    for (double rate : {44100., 48000., 88200., 96000.}) {
      RateScaledPremiumDrive candidate; PremiumDrive reference;
      candidate.Init(rate); reference.Init(rate);
      Require(candidate.Factor() == 4, "low-rate factor contract");
      candidate.Set(20); reference.Set(20);
      for (int i = 0; i < 2048; ++i) {
        const float x = static_cast<float>(.75 * std::sin(i * .29));
        Require(candidate.Process({x, -x}) == reference.Process({x, -x}), "low-rate 4x exact routing");
      }
    }
    // Coherent audible-frequency controls, aligned to a 1024-sample period.
    // Difference to 4x includes harmonic amplitude/phase and alias changes;
    // it is NOT an isolated measurement of total alias power.
    std::cout << "rate,requested_hz,actual_hz,drive_db,amplitude,relative_rms_error,peak_error,candidate_selected_audible_residual_dbfs,reference_selected_audible_residual_dbfs\n";
    double largest = 0;
    int controls = 0;
    for (double rate : {88200., 96000., 176400., 192000.}) {
      RateScaledPremiumDrive impulse; impulse.Init(rate);
      Require(impulse.Factor() == (rate >= 176400 ? 2u : 4u), "high-rate research factor contract");
      double peak = 0; int position = -1;
      for (int i = 0; i < 128; ++i) {
        const auto y = impulse.Process({i == 0 ? 1.f : 0.f, 0});
        Require(y[1] == 0, "impulse stereo isolation");
        if (std::abs(y[0]) > peak) { peak = std::abs(y[0]); position = i; }
      }
      Require(position == 32, "high-rate delay stays 32 host samples");
      impulse.Clear();
      for (int i = 0; i < 128; ++i) Require(impulse.Process({0, 0})[0] == 0, "high-rate clear");
      impulse.Set(20); impulse.SnapToTargets();
      for (int i = 0; i < 128; ++i) impulse.Process({.2f, -.3f});
      auto intact = impulse;
      const auto bad = impulse.Process({std::numeric_limits<float>::quiet_NaN(), -.3f});
      const auto good = intact.Process({.2f, -.3f});
      Require(bad[1] == good[1], "rate-scaled invalid input preserves other channel history");
      impulse.Set(std::numeric_limits<double>::infinity());
      for (int i = 0; i < 128; ++i)
        Require(std::isfinite(impulse.Process({.2f, -.3f})[0]), "rate-scaled invalid control recovery");
      for (double hz : {1000., 3000., 8000., 12000., 18000., 20000.})
        for (double db : {0., 12., 20., 24.}) for (double amplitude : {.1, .75}) {
          RateScaledPremiumDrive candidate; PremiumDrive reference;
          candidate.Init(rate); reference.Init(rate);
          candidate.Set(db); reference.Set(db); candidate.SnapToTargets(); reference.SnapToTargets();
          constexpr int period = 1024;
          const int bin = static_cast<int>(std::round(hz * period / rate));
          double error = 0, energy = 0, maxError = 0;
          Period candidatePeriod{}, referencePeriod{};
          for (int i = 0; i < period * 3; ++i) {
            const float x = static_cast<float>(amplitude * std::sin(2 * pi * bin * (i % period) / period));
            const auto a = candidate.Process({x, -x}), b = reference.Process({x, -x});
            Require(std::isfinite(a[0]) && a[0] == -a[1], "high-rate finite symmetric output");
            if (i >= period) {
              const double d = static_cast<double>(a[0]) - b[0];
              error += d * d; energy += double(b[0]) * b[0]; maxError = std::max(maxError, std::abs(d));
              candidatePeriod[i % period] += a[0] / 2.; referencePeriod[i % period] += b[0] / 2.;
            }
          }
          Require(energy > 0, "non-silent reference");
          ++controls;
          const double relative = std::sqrt(error / energy);
          largest = std::max(largest, relative);
          const double selectedResidual = AudibleResidual(candidatePeriod, bin, rate);
          Require(std::isfinite(selectedResidual) && selectedResidual < 1e-5,
                  "selected audible residual safety ceiling (-50 dBFS), not total alias acceptance");
          std::cout << rate << ',' << hz << ',' << rate * bin / period << ',' << db << ',' << amplitude
                    << ',' << relative << ',' << maxError << ','
                    << 10 * std::log10(selectedResidual + 1e-30) << ','
                    << 10 * std::log10(AudibleResidual(referencePeriod, bin, rate) + 1e-30) << '\n';
          Require(relative < .1, "research error safety bound, not a shipping acceptance threshold");
          if (db == 0) Require(relative < .001, "high-rate linear passband agreement");
        }
    }
    Require(controls == 192, "complete audible-frequency grid");
    // Reinitialization crosses the factor boundary and must forget old input.
    RateScaledPremiumDrive reused, fresh; reused.Init(192000); reused.Set(24);
    for (int i = 0; i < 100; ++i) reused.Process({.5f, -.5f});
    reused.Init(48000); fresh.Init(48000);
    for (int i = 0; i < 100; ++i) {
      const float x = static_cast<float>(.2 * std::sin(i * .37));
      Require(reused.Process({x, -x}) == fresh.Process({x, -x}), "factor reset boundary");
    }
    std::cerr << "Largest 2x vs 4x relative RMS difference: " << largest << '\n';
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
