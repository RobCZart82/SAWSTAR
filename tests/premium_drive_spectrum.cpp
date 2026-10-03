// SPDX-License-Identifier: MIT
#include "../experiments/premium_filter/PremiumDrive.h"
#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
constexpr double pi = 3.14159265358979323846;
using Period = std::array<double, 32>;
double BinPower(const Period& signal, int bin) {
  double c = 0, s = 0;
  for (int i = 0; i < 32; ++i) {
    const double angle = 2 * pi * bin * i / 32;
    c += signal[i] * std::cos(angle); s += signal[i] * std::sin(angle);
  }
  return (c * c + s * s) / (32 * 32);
}
double Residual(const Period& signal, int fundamental) {
  double power = 0;
  for (int bin = 1; bin < 16; ++bin) {
    // Exclude legitimate below-Nyquist harmonics. Aliases coinciding with those
    // bins are not measured here; this is not total alias-energy separation.
    if (bin % fundamental != 0) power += BinPower(signal, bin);
  }
  return power;
}
void Require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
}
int main() {
  try {
    int meaningful = 0;
    std::cout << "rate,input_hz,drive_db,input_amplitude,residual_change_db,residual_to_fundamental_change_db\n";
    for (double rate : {44100., 48000., 96000., 192000.}) {
      for (int bin : {3, 5, 7, 11, 13}) for (double db : {6., 12., 20., 24.}) {
        for (double amplitude : {.1, .75}) {
          sawstar::experimental::PremiumDrive drive; drive.Init(rate); drive.Set(db); drive.SnapToTargets();
          const double gain = std::pow(10., db / 20);
          Period input{}, raw{}, output{};
          for (int i = 0; i < 32; ++i) {
            input[i] = static_cast<float>(amplitude * std::sin(2 * pi * bin * i / 32));
            raw[i] = std::tanh(gain * input[i]) / gain;
          }
          for (int i = 0; i < 1024 + 2048; ++i) {
            const auto y = drive.Process({static_cast<float>(input[i % 32]), 0});
            Require(std::isfinite(y[0]) && y[1] == 0, "finite isolated spectral output");
            if (i >= 1024) output[i % 32] += y[0] / 64.;
          }
          const double oldPower = Residual(raw, bin), newPower = Residual(output, bin);
          const double change = 10 * std::log10((newPower + 1e-30) / (oldPower + 1e-30));
          const double fundamentalRaw = BinPower(raw, bin), fundamentalNew = BinPower(output, bin);
          Require(fundamentalRaw > 0 && fundamentalNew > 0, "non-silent fundamental controls");
          const double relativeChange = change + 10 * std::log10(fundamentalRaw / fundamentalNew);
          Require(std::isfinite(change) && std::isfinite(relativeChange), "finite spectral ratios");
          if (oldPower > 1e-12) {
            ++meaningful;
            Require(change < -6 && relativeChange < -6,
                    "significant residual bins improve at least 6 dB, also relative to fundamental");
            std::cout << rate << ',' << rate * bin / 32 << ',' << db << ',' << amplitude << ',' << change << ',' << relativeChange << '\n';
          }
        }
      }
      // Separately characterize zero-Drive amplitude and phase at band edge.
      for (int bin : {1, 4, 8, 10, 12, 13, 14, 15}) {
        sawstar::experimental::PremiumDrive drive; drive.Init(rate);
        Period output{}, input{};
        for (int i = 0; i < 32; ++i) input[i] = .1 * std::sin(2 * pi * bin * i / 32);
        for (int i = 0; i < 3072; ++i) {
          const auto y = drive.Process({static_cast<float>(input[i % 32]), 0});
          if (i >= 1024) output[i % 32] += y[0] / 64.;
        }
        const double gain = std::sqrt(BinPower(output, bin) / BinPower(input, bin));
        Require(std::isfinite(gain) && gain < 1.001, "no significant linear boost");
        if (bin <= 10) Require(std::abs(gain - 1) < .001, "flat passband through 0.3125 host rate");
        std::cout << "linear," << rate << ',' << rate * bin / 32 << ',' << 20 * std::log10(gain) << '\n';
      }
    }
    Require(meaningful >= 80, "enough non-negligible spectral controls");
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
