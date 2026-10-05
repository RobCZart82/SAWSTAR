// SPDX-License-Identifier: MIT
// Offline FIR-only control: no shipping or user-selectable bypass policy.
#include "../experiments/premium_filter/PremiumDrive.h"
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
using Frame = std::array<float, 2>;
volatile double checksum = 0;
template<unsigned Factor> void Contract() {
  using Full = sawstar::experimental::FixedRatePremiumDrive<Factor>;
  using Linear = sawstar::experimental::FixedRatePremiumDrive<Factor, false>;
  static_assert(sizeof(Full) == sizeof(Linear), "Same FIR storage");
  static_assert(Full::Latency == Linear::Latency, "Same latency");
  for (double rate : {8000., 48000., 96000., 192000., 384000.}) {
    Full a; Linear b; a.Init(rate); b.Init(rate);
    for (int n = 0; n < 4096; ++n) {
      const Frame x{static_cast<float>(.4 * std::sin(n * .11)), n == 129 ? .7f : 0.f};
      const auto y = a.Process(x), z = b.Process(x);
      if (std::memcmp(y.data(), z.data(), sizeof(y))) throw std::runtime_error("Zero Drive FIR control differs");
    }
    a.Clear(); b.Clear();
    if (a.Process({0, 0}) != Frame{0, 0} || b.Process({0, 0}) != Frame{0, 0})
      throw std::runtime_error("Cost control clear");
    a.Set(20); b.Set(20); a.SnapToTargets(); b.SnapToTargets();
    double difference = 0;
    for (int n = 0; n < 4096; ++n) {
      const Frame x{static_cast<float>(.4 * std::sin(n * .11)), -.3f};
      const auto y = a.Process(x), z = b.Process(x);
      for (int ch = 0; ch < 2; ++ch) {
        if (!std::isfinite(y[ch]) || !std::isfinite(z[ch])) throw std::runtime_error("Cost control nonfinite");
        difference += std::abs(y[ch] - z[ch]);
      }
    }
    if (difference <= 1) throw std::runtime_error("Nonlinear control must change sound");
  }
}
template<class Drive> double Measure(double rate, double db, const std::vector<Frame>& input) {
  std::array<Drive, 16> bank;
  for (auto& d : bank) { d.Init(rate); d.Set(db); d.SnapToTargets(); }
  for (int n = 0; n < 1024; ++n) for (auto& d : bank) d.Process(input[n]);
  double sum = 0;
  const auto start = std::chrono::steady_clock::now();
  for (const auto& x : input) for (auto& d : bank) {
    const auto y = d.Process(x); sum += y[0] + y[1];
  }
  const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
  checksum = checksum + sum;
  return seconds;
}
template<unsigned Factor> void Profile() {
  for (double rate : {48000., 96000., 192000.}) {
    std::vector<Frame> input(16384);
    for (int n = 0; n < static_cast<int>(input.size()); ++n)
      input[n] = {static_cast<float>(.4 * std::sin(6.283185307179586 * 440 * n / rate)),
                  static_cast<float>(.4 * std::cos(6.283185307179586 * 660 * n / rate))};
    for (double db : {20., 24.}) for (int pair = 0; pair < 6; ++pair) {
      double full, linear;
      using Full = sawstar::experimental::FixedRatePremiumDrive<Factor>;
      using Linear = sawstar::experimental::FixedRatePremiumDrive<Factor, false>;
      if (pair % 2) { linear = Measure<Linear>(rate, db, input); full = Measure<Full>(rate, db, input); }
      else { full = Measure<Full>(rate, db, input); linear = Measure<Linear>(rate, db, input); }
      if (!std::isfinite(full) || !std::isfinite(linear) || full <= 0 || linear <= 0)
        throw std::runtime_error("Invalid timing");
      std::cout << Factor << ',' << rate << ",16," << db << ',' << pair << ',' << full << ',' << linear << '\n';
    }
  }
}
int main(int argc, char** argv) {
  try {
    if (argc == 1) { Contract<2>(); Contract<4>(); std::cout << "Cost controls PASS\n"; }
    else if (argc == 2 && std::string(argv[1]) == "--benchmark") {
      std::cout << "factor,rate,voices,drive_db,pair,full_seconds,fir_only_seconds\n";
      Profile<2>(); Profile<4>();
      if (!std::isfinite(checksum)) throw std::runtime_error("Invalid checksum");
    } else throw std::runtime_error("Usage: premium_drive_cost [--benchmark]");
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
