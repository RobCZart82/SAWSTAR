// SPDX-License-Identifier: MIT
#include "../experiments/premium_filter/LookupTanh.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {
constexpr double AbsoluteLimit = 1e-10, RelativeLimit = 1e-8;
void Require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
double maximum = 0, relativeMaximum = 0;
void Compare(const sawstar::experimental::LookupTanh& candidate, double x) {
  const double actual = candidate.Process(x), expected = std::tanh(x);
  if (std::isnan(x)) { Require(std::isnan(actual), "NaN tanh"); return; }
  Require(std::isfinite(actual) && std::abs(actual) <= 1., "Finite bounded tanh");
  const double error = std::abs(actual - expected);
  maximum = std::max(maximum, error);
  Require(error <= AbsoluteLimit, "Lookup tanh absolute error");
  if (expected != 0.) {
    const double relative = error / std::abs(expected);
    relativeMaximum = std::max(relativeMaximum, relative);
    Require(relative <= RelativeLimit, "Lookup tanh relative error");
  }
  Require(candidate.Process(-x) == -actual, "Odd tanh");
}
void Contract() {
  // First Init occurs concurrently; every instance only reads the immutable
  // common table after construction. TSan runs this concrete initialization.
  std::atomic<bool> failed{false};
  std::array<std::thread, 8> workers;
  for (std::size_t worker = 0; worker < workers.size(); ++worker) {
    workers[worker] = std::thread([&, worker] {
      sawstar::experimental::LookupTanh local; local.Init();
      for (int i = 0; i < 1000; ++i) {
        const double x = (i + worker * .25) / 64.;
        if (std::abs(local.Process(x) - std::tanh(x)) > AbsoluteLimit)
          failed.store(true);
      }
    });
  }
  for (auto& worker : workers) worker.join();
  Require(!failed.load(), "Concurrent lookup initialization");
  sawstar::experimental::LookupTanh a, b; a.Init(); b.Init();
  double previous = -1.;
  for (int i = 0; i <= 1000000; ++i) {
    const double x = -20. + 40. * i / 1000000.;
    Compare(a, x);
    const double y = a.Process(x);
    Require(y >= previous, "Monotonic dense tanh grid"); previous = y;
    Require(y == b.Process(x), "Shared table determinism");
  }
  // Every interpolation boundary and its neighboring representable values.
  for (int i = 0; i <= 2048; ++i) for (double sign : {-1., 1.}) {
    const double x = sign * i / 128.;
    Compare(a, std::nextafter(x, -INFINITY)); Compare(a, x);
    Compare(a, std::nextafter(x, INFINITY));
  }
  for (int exponent = -1074; exponent <= 1023; ++exponent)
    for (double sign : {-1., 1.}) Compare(a, sign * std::ldexp(1., exponent));
  for (double sign : {-1., 1.}) for (double threshold : {1e-8, 16.}) {
    const double x = sign * threshold;
    Compare(a, std::nextafter(x, -INFINITY)); Compare(a, x);
    Compare(a, std::nextafter(x, INFINITY));
  }
  for (double x : {0., -0., std::numeric_limits<double>::infinity(),
                   -std::numeric_limits<double>::infinity(),
                   std::numeric_limits<double>::quiet_NaN()}) Compare(a, x);
  Require(std::signbit(a.Process(-0.)), "Signed zero");
  const auto copy = a; b.Init();
  for (double x : {-.3, .7, 12.}) Require(copy.Process(x) == b.Process(x), "Copy/reinit immutable table");
  Require(.99 * a.Process(1.) != std::tanh(1.) &&
          std::abs(.99 * a.Process(1.) - std::tanh(1.)) > AbsoluteLimit,
          "Altered-output negative control");
  std::cout << std::setprecision(17) << "Lookup scalar contract PASS; absolute=" << maximum
            << " relative=" << relativeMaximum << " shared_bytes=" << a.TableBytes << '\n';
}
volatile double checksum = 0;
template<class Function> double Measure(const std::vector<double>& input, Function evaluate) {
  double sum = 0;
  for (double x : input) sum += evaluate(x);
  const auto start = std::chrono::steady_clock::now();
  for (int repeat = 0; repeat < 16; ++repeat)
    for (double x : input) sum += evaluate(x);
  const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
  Require(std::isfinite(seconds) && seconds > 0 && std::isfinite(sum), "Valid scalar timing");
  checksum = checksum + sum; return seconds;
}
void Benchmark() {
  sawstar::experimental::LookupTanh candidate; candidate.Init();
  std::cout << std::setprecision(17) << "drive_db,pair,order,reference_seconds,candidate_seconds\n";
  for (double db : {0., 12., 20., 24.}) {
    std::vector<double> input(65536);
    const double gain = std::pow(10., db / 20.);
    for (std::size_t i = 0; i < input.size(); ++i) input[i] = gain * .4 * std::sin(i * .057);
    for (int pair = 0; pair < 8; ++pair) {
      double reference = 0, study = 0;
      auto a = [&] { study = Measure(input, [&](double x) { return candidate.Process(x); }); };
      auto b = [&] { reference = Measure(input, [](double x) { return std::tanh(x); }); };
      if (pair % 2) { a(); b(); } else { b(); a(); }
      std::cout << db << ',' << pair << ',' << (pair % 2 ? "study-first" : "reference-first")
                << ',' << reference << ',' << study << '\n';
    }
  }
}
}
int main(int argc, char** argv) {
  try {
    if (argc == 1) Contract();
    else if (argc == 2 && std::string(argv[1]) == "--benchmark") Benchmark();
    else throw std::runtime_error("Usage: premium_lookup_tanh [--benchmark]");
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
