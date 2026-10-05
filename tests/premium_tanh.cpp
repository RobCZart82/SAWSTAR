// SPDX-License-Identifier: MIT
#include "../experiments/premium_filter/ResearchTanh.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
using sawstar::experimental::ResearchTanh;
volatile double checksum = 0;
void Contract() {
  double maxError = 0, previous = -1;
  auto check = [&](double x) {
    const double y = ResearchTanh(x), reference = std::tanh(x);
    const double error = std::abs(y - reference);
    if (!std::isfinite(y) || std::abs(y) > 1 || error > 5e-15
        || y != -ResearchTanh(-x)) throw std::runtime_error("Research tanh error/range/symmetry");
    maxError = std::max(maxError, error);
  };
  for (int n = 0; n <= 400000; ++n) {
    const double x = -20. + n * .0001;
    check(x);
    const double y = ResearchTanh(x);
    if (y < previous) throw std::runtime_error("Research tanh monotonicity");
    previous = y;
  }
  for (int exponent = -1074; exponent <= 12; ++exponent) {
    const double x = std::ldexp(1., exponent); check(x); check(-x);
  }
  for (double edge : {1e-5, 20.}) for (double x : {edge, std::nextafter(edge, 0.),
       std::nextafter(edge, std::numeric_limits<double>::infinity())}) { check(x); check(-x); }
  if (!std::signbit(ResearchTanh(-0.)) || ResearchTanh(0.) != 0.
      || ResearchTanh(std::numeric_limits<double>::infinity()) != 1.
      || ResearchTanh(-std::numeric_limits<double>::infinity()) != -1.
      || !std::isnan(ResearchTanh(std::numeric_limits<double>::quiet_NaN())))
    throw std::runtime_error("Research tanh special values");
  std::cout << "Maximum scalar absolute error: " << maxError << '\n';
}
template<bool Research> double Measure(const std::vector<double>& input) {
  double sum = 0;
  const auto start = std::chrono::steady_clock::now();
  for (int repeat = 0; repeat < 16; ++repeat) for (double x : input) {
    if constexpr (Research) sum += ResearchTanh(x); else sum += std::tanh(x);
  }
  const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
  checksum = checksum + sum;
  return seconds;
}
void Benchmark() {
  std::cout << "drive_db,pair,std_tanh_seconds,research_seconds\n";
  for (double db : {0., 12., 20., 24.}) {
    std::vector<double> input(65536);
    const double gain = std::pow(10., db / 20.);
    for (int n = 0; n < static_cast<int>(input.size()); ++n)
      input[n] = gain * .4 * std::sin(n * .057);
    Measure<false>(input); Measure<true>(input);
    for (int pair = 0; pair < 8; ++pair) {
      double a, b;
      if (pair % 2) { b = Measure<true>(input); a = Measure<false>(input); }
      else { a = Measure<false>(input); b = Measure<true>(input); }
      if (!std::isfinite(a) || !std::isfinite(b) || a <= 0 || b <= 0)
        throw std::runtime_error("Invalid scalar timing");
      std::cout << db << ',' << pair << ',' << a << ',' << b << '\n';
    }
  }
  if (!std::isfinite(checksum)) throw std::runtime_error("Nonfinite checksum");
}
int main(int argc, char** argv) {
  try {
    if (argc == 1) Contract();
    else if (argc == 2 && std::string(argv[1]) == "--benchmark") Benchmark();
    else throw std::runtime_error("Usage: premium_tanh [--benchmark]");
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
