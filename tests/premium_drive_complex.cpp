// SPDX-License-Identifier: MIT
#include "../experiments/premium_filter/RateScaledPremiumDrive.h"
#include "../experiments/premium_filter/ReferencePremiumDrive.h"
#include "../experiments/premium_filter/PremiumLowPass.h"
#include "dsp/SevenSaw.h"
#include "dsp/SubOscillator.h"
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace sawstar::experimental;
#ifdef SAWSTAR_COMPLEX_GAIN_STUDY
using CandidateDrive = RateScaledGainPremiumDrive;
constexpr std::array<double, 3> Rates{176400., 192000., 384000.};
#else
using CandidateDrive = RateScaledPremiumDrive;
constexpr std::array<double, 2> Rates{176400., 192000.};
#endif
void Require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
struct Error {
  double squared = 0, energy = 0, peak = 0;
  void Add(std::array<float, 2> x, std::array<float, 2> reference) {
    for (size_t ch = 0; ch < 2; ++ch) {
      Require(std::isfinite(x[ch]) && std::isfinite(reference[ch]), "finite complex outputs");
      const double d = double(x[ch]) - reference[ch];
      squared += d * d; energy += double(reference[ch]) * reference[ch];
      peak = std::max(peak, std::abs(d));
    }
  }
  double Relative() const { Require(energy > 1e-20, "audible non-silent control"); return std::sqrt(squared / energy); }
};
std::vector<std::array<float, 2>> Source(double rate, int scene, int count) {
  sawstar::SevenSaw one, two; sawstar::SubOscillator sub;
  const std::array<float, 4> roots{65.4064f, 261.6256f, 130.8128f, 2093.005f};
  one.Init(static_cast<float>(rate)); two.Init(static_cast<float>(rate)); sub.Init(static_cast<float>(rate));
  one.SetFreq(roots[scene]); two.SetFreq(2 * roots[scene]); sub.SetFreq(.5f * roots[scene]);
  one.SetShape(20, 1, .75f); two.SetShape(27, 1, .8f);
  // Per-voice OSC1/OSC2/SUB, not a chord mixed into one nonlinear filter.
  one.SetWaveform(scene == 2 ? 2 : 0); two.SetWaveform(scene == 1 ? 1 : 0);
  sub.SetWaveform(scene == 0 ? 2 : 0);
  one.SnapToTargets(); two.SnapToTargets(); sub.SnapToTargets();
  std::vector<std::array<float, 2>> result(count);
  double peak = 0;
  for (auto& frame : result) {
    const auto a = one.Process(), b = two.Process(); const float c = sub.Process();
    frame = {.7f * a.left + .5f * b.left + .2f * c, .7f * a.right + .5f * b.right + .2f * c};
    peak = std::max(peak, double(std::max(std::abs(frame[0]), std::abs(frame[1]))));
  }
  Require(peak > 0, "complex source peak");
  // One common constant scale per scene, identical for all three paths.
  for (auto& frame : result) for (auto& x : frame) x = static_cast<float>(x * .75 / peak);
  return result;
}
int main() {
  try {
    // Independent 8x impulse/clear/passband checks before using it as reference.
    for (double rate : Rates) {
      ReferencePremiumDrive8x reference; reference.Init(rate);
      int position = -1; double peak = 0;
      for (int i = 0; i < 128; ++i) {
        const auto y = reference.Process({i == 0 ? 1.f : 0.f, 0});
        Require(y[1] == 0, "8x channel isolation");
        if (std::abs(y[0]) > peak) { peak = std::abs(y[0]); position = i; }
      }
      Require(position == 32, "8x reference delay");
      reference.Clear();
      for (int i = 0; i < 128; ++i) Require(reference.Process({0, 0})[0] == 0, "8x clear");
      for (double hz : {1000., 12000., 20000.}) {
        reference.Init(rate); double energy = 0, error = 0;
        for (int i = 0; i < 1024; ++i) {
          constexpr double pi = 3.14159265358979323846;
          const float x = static_cast<float>(.1 * std::sin(2 * pi * hz * i / rate));
          const auto y = reference.Process({x, 0});
          if (i >= 256) {
            const double expected = .1 * std::sin(2 * pi * hz * (i - 32) / rate);
            energy += expected * expected; error += (y[0] - expected) * (y[0] - expected);
          }
        }
        Require(error / energy < 1e-6, "8x independent delayed linear passband");
      }
    }
    std::cout << "rate,scene,filter_mode,drive_control,rate_to_4x_rms,rate_to_8x_rms,4x_to_8x_rms,filtered_rate_to_8x_rms,filtered_4x_to_8x_rms,rate_to_8x_peak";
#ifdef SAWSTAR_COMPLEX_GAIN_STUDY
    std::cout << ",rate_to_division_rms,filtered_rate_to_division_rms,rate_to_division_peak,filtered_rate_to_division_peak";
    std::cout << std::setprecision(17);
#endif
    std::cout << '\n';
    const std::array<const char*, 4> names{"bass", "lead", "pad-voice", "high-lead"};
    constexpr int warmup = 1024, frames = 4096;
    int controls = 0;
    for (double rate : Rates) for (int scene = 0; scene < 4; ++scene) {
      const auto source = Source(rate, scene, warmup + frames);
      for (int mode = 0; mode < 4; ++mode) for (int control = 0; control < 3; ++control) {
        CandidateDrive candidate; PremiumDrive four; ReferencePremiumDrive8x eight;
        candidate.Init(rate); four.Init(rate); eight.Init(rate);
        const double initial = control == 0 ? 20 : control == 1 ? 24 : 0;
        candidate.Set(initial); four.Set(initial); eight.Set(initial);
        candidate.SnapToTargets(); four.SnapToTargets(); eight.SnapToTargets();
#ifdef SAWSTAR_COMPLEX_GAIN_STUDY
        RateScaledSimdPremiumDrive division;
        division.Init(rate); division.Set(initial); division.SnapToTargets();
        std::array<PremiumLowPass, 4> filters;
        Error toDivision, filteredToDivision;
#else
        std::array<PremiumLowPass, 3> filters;
#endif
        for (auto& f : filters) { f.Init(rate); f.Set(12000, 50); f.SetMode(mode); f.SnapToTargets(); }
        Error toFour, toEight, fourToEight, filteredToEight, filteredFourToEight;
        for (int i = 0; i < warmup + frames; ++i) {
          if (control == 2 && i % 512 == 0) {
            constexpr std::array<double, 8> targets{0, 12, 20, 24, 20, 12, 0, 24};
            const double db = targets[(i / 512) % targets.size()];
            candidate.Set(db); four.Set(db); eight.Set(db);
#ifdef SAWSTAR_COMPLEX_GAIN_STUDY
            division.Set(db);
#endif
          }
          // Shared rapid cutoff target changes stress the downstream state;
          // this is a short numerical control, not a musical listening fixture.
          if (i % 128 == 0) {
            const double cutoff = i < warmup ? 12000 : 12000 * std::pow(500. / 12000, double(i - warmup) / frames);
            for (auto& f : filters) f.Set(cutoff, 50);
          }
          const auto a = candidate.Process(source[i]), b = four.Process(source[i]), c = eight.Process(source[i]);
          const auto fa = filters[0].Process(a), fb = filters[1].Process(b), fc = filters[2].Process(c);
#ifdef SAWSTAR_COMPLEX_GAIN_STUDY
          const auto d = division.Process(source[i]), fd = filters[3].Process(d);
          if (i >= warmup) { toDivision.Add(a, d); filteredToDivision.Add(fa, fd); }
#endif
          if (i >= warmup) {
            toFour.Add(a, b); toEight.Add(a, c); fourToEight.Add(b, c);
            filteredToEight.Add(fa, fc); filteredFourToEight.Add(fb, fc);
          }
        }
        ++controls;
        std::cout << rate << ',' << names[scene] << ',' << mode << ',' << (control == 0 ? "20" : control == 1 ? "24" : "steps")
          << ',' << toFour.Relative() << ',' << toEight.Relative() << ',' << fourToEight.Relative()
          << ',' << filteredToEight.Relative() << ',' << filteredFourToEight.Relative() << ',' << toEight.peak;
#ifdef SAWSTAR_COMPLEX_GAIN_STUDY
        std::cout << ',' << toDivision.Relative() << ',' << filteredToDivision.Relative()
          << ',' << toDivision.peak << ',' << filteredToDivision.peak;
        Require(toDivision.peak <= 2e-7 && toDivision.Relative() <= 1e-7,
                "complex normalization numerical bounds");
        Require(filteredToDivision.peak <= 2e-6 && filteredToDivision.Relative() <= 1e-7,
                "filtered complex normalization numerical bounds");
#endif
        std::cout << '\n';
        // Diagnostic failure bounds, not a claim of transparent or alias-free audio.
        Require(toFour.Relative() < .01 && toEight.Relative() < .01, "complex-source diagnostic difference ceiling (1%)");
        Require(filteredToEight.Relative() < .005, "filtered complex-source diagnostic difference ceiling (0.5%)");
        Require(fourToEight.Relative() < .001, "4x/8x convergence diagnostic ceiling (0.1%)");
      }
    }
#ifdef SAWSTAR_COMPLEX_GAIN_STUDY
    Require(controls == 144, "complete combined gain four-mode high-rate grid");
#else
    Require(controls == 96, "complete four-mode complex source grid");
#endif
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
