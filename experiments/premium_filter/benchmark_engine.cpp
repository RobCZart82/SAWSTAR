// SPDX-License-Identifier: MIT
// Offline complete engine probe; elapsed time is not a CI pass/fail gate.
#include "PremiumSynth.h"
#include "engine/Synth.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>

template<class Synth> void Setup(Synth& s, double rate, int voices, double drive, bool fx) {
  s.Reset(rate); s.SetParameters(-6, 5, 100, .8, 100); s.SetOutputBoost(18);
  s.SetSaw(25, 70, 70); s.SetOsc2(19, 60, 80);
  s.SetMixer(70, 50, 20, 5, 0, -1, 0, 0);
  s.SetFilter(1800, 50, 100); s.SetFilterCharacter(static_cast<float>(drive), 1);
  if (fx) {
    s.SetChorus(true, 20, .3, 30); s.SetDelay(true, 15, 250, 30, 60, true, false, 2, 120);
    s.SetReverb(true, 15, 50, 40, 50);
  }
  for (int n = 0; n < voices; ++n) s.Midi(0x90, 48 + n, 100);
}
void Check(sawstar::StereoSample x) {
  if (!std::isfinite(x.left) || !std::isfinite(x.right) ||
      std::abs(x.left) > .98001f || std::abs(x.right) > .98001f)
    throw std::runtime_error("Nonfinite or unprotected engine output");
}
void Smoke() {
  using sawstar::experimental::EnginePremiumFilter;
  EnginePremiumFilter filter; filter.Init(48000); filter.Set(1800, 50, 0);
  filter.SetCharacter(20, 1); filter.SnapToTargets();
  for (int i = 0; i < 96; ++i) {
    auto y = filter.Process({i == 0 ? .5f : 0.f, 0});
    if (y.left != (i == 32 ? .5f : 0.f) || y.right != 0)
      throw std::runtime_error("Delayed dry path contract");
  }
  // Adapter's fully wet output must be the accepted Drive -> filter chain.
  filter.Init(48000); filter.Set(1800, 50, 100); filter.SetCharacter(20, 1); filter.SnapToTargets();
  sawstar::experimental::PremiumDrive drive; drive.Init(48000); drive.Set(20); drive.SnapToTargets();
  sawstar::experimental::PremiumLowPass lp; lp.Init(48000); lp.Set(1800, 50); lp.SnapToTargets();
  for (int i = 0; i < 1024; ++i) {
    const float x = static_cast<float>(.4 * std::sin(i * .13));
    const auto expected = lp.Process(drive.Process({x, -x}));
    const auto actual = filter.Process({x, -x});
    if (std::abs(actual.left - expected[0]) > 1e-7 || std::abs(actual.right - expected[1]) > 1e-7)
      throw std::runtime_error("Adapter wet-chain reference contract");
  }
  filter.Clear();
  if (filter.Process({0, 0}).left != 0) throw std::runtime_error("Adapter clear contract");
  bool rejected = false;
  try { filter.SetCharacter(0, 0); } catch (const std::invalid_argument&) { rejected = true; }
  if (!rejected) throw std::runtime_error("Unsupported mode must not silently use LP24");
  // Same production MIDI/envelope/FX code, a separate per-voice filter type.
  for (double rate : {44100., 48000., 96000., 192000.}) for (int mode : {0, 1, 2}) {
    auto s = std::make_unique<sawstar::experimental_engine::Synth>();
    Setup(*s, rate, 0, 20, true); s->SetVoiceMode(mode, 0, true);
    for (int n = 0; n < 3; ++n) s->Midi(0x90, 48 + n, 100);
    for (int n = 0; n < 3; ++n) if (!s->Held(48 + n))
      throw std::runtime_error("Probe must actually hold MIDI notes");
    for (int i = 0; i < 512; ++i) Check(s->ProcessStereo());
    for (int n = 0; n < 3; ++n) s->Midi(0x80, 48 + n, 0);
    for (int i = 0; i < static_cast<int>(rate); ++i) Check(s->ProcessStereo());
    if (s->ActiveVoices() != 0) throw std::runtime_error("Probe voice release contract");
    s->Reset(rate);
    for (int i = 0; i < 64; ++i) {
      auto y = s->ProcessStereo();
      if (y.left != 0 || y.right != 0) throw std::runtime_error("Probe reset must be silent");
    }
  }
  std::cout << "Premium engine probe lifecycle and delayed-mix contracts PASS\n";
}
template<class Synth> void Benchmark(const char* label) {
  using Clock = std::chrono::steady_clock;
  constexpr int frames = 4096;
  double checksum = 0;
  for (double rate : {48000., 96000., 192000.}) for (int voices : {1, 8, 16})
    for (double drive : {0., 20., 24.}) for (bool fx : {false, true})
      for (int buffer : {32, 64, 128, 256}) {
        std::array<double, 3> times{}; double worst = 0, energy = 0, peak = 0;
        for (auto& time : times) {
          auto s = std::make_unique<Synth>(); Setup(*s, rate, voices, drive, fx);
          for (int i = 0; i < 2048; ++i) Check(s->ProcessStereo());
          double sum = 0;
          const auto start = Clock::now();
          for (int begin = 0; begin < frames; begin += buffer) {
            const auto blockStart = Clock::now();
            for (int i = 0; i < buffer; ++i) {
              const auto y = s->ProcessStereo(); Check(y);
              energy += double(y.left) * y.left + double(y.right) * y.right;
              peak = std::max(peak, double(std::max(std::abs(y.left), std::abs(y.right))));
              sum += y.left + y.right;
            }
            worst = std::max(worst, 100 * rate / buffer *
              std::chrono::duration<double>(Clock::now() - blockStart).count());
          }
          time = std::chrono::duration<double>(Clock::now() - start).count(); checksum += sum;
        }
        std::sort(times.begin(), times.end());
        std::cout << label << ',' << rate << ',' << voices << ',' << drive << ',' << fx << ',' << buffer
          << ',' << 100 * times[1] * rate / frames << ',' << worst << ',' << peak
          << ',' << std::sqrt(energy / (frames * 2 * times.size())) << '\n';
      }
  if (!std::isfinite(checksum)) throw std::runtime_error("Invalid benchmark checksum");
}
int main(int argc, char** argv) {
  try {
    if (argc == 2 && std::string(argv[1]) == "--smoke") { Smoke(); return 0; }
    const bool legacy = argc == 2 && std::string(argv[1]) == "--legacy";
    if (argc != 1 && !legacy) { std::cerr << "Usage: premium_engine_benchmark [--smoke|--legacy]\n"; return 1; }
    std::cout << "engine,rate,voices,drive_db,fx,buffer,median_realtime_percent,worst_block_percent,peak,rms\n";
    if (legacy) Benchmark<sawstar::Synth>("legacy");
    else Benchmark<sawstar::experimental_engine::Synth>("premium");
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
