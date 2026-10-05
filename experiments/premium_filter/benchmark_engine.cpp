// SPDX-License-Identifier: MIT
// Offline complete engine probe; elapsed time is not a CI pass/fail gate.
#ifdef SAWSTAR_PREMIUM_HIGH_RATE_STUDY
#include "RatePremiumSynth.h"
namespace ProbeEngine = sawstar::experimental_rate_engine;
#else
#include "PremiumSynth.h"
namespace ProbeEngine = sawstar::experimental_engine;
#endif
#include "engine/Synth.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include <vector>

template<class Synth> void Setup(Synth& s, double rate, int voices, double drive, bool fx, int filterMode = 1) {
  s.Reset(rate); s.SetParameters(-6, 5, 100, .8, 100); s.SetOutputBoost(18);
  s.SetSaw(25, 70, 70); s.SetOsc2(19, 60, 80);
  s.SetMixer(70, 50, 20, 5, 0, -1, 0, 0);
  s.SetFilter(1800, 50, 100); s.SetFilterCharacter(static_cast<float>(drive), filterMode);
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
  // Every adapter mode must use the corresponding Drive -> filter chain.
  for (int filterMode : {0, 1, 2, 3}) {
    filter.Init(48000); filter.Set(1800, 50, 100); filter.SetCharacter(20, filterMode); filter.SnapToTargets();
    sawstar::experimental::PremiumDrive drive; drive.Init(48000); drive.Set(20); drive.SnapToTargets();
    sawstar::experimental::PremiumLowPass lp; lp.Init(48000); lp.Set(1800, 50);
    lp.SetMode(filterMode); lp.SnapToTargets();
    for (int i = 0; i < 1024; ++i) {
      const float x = static_cast<float>(.4 * std::sin(i * .13));
      const auto expected = lp.Process(drive.Process({x, -x}));
      const auto actual = filter.Process({x, -x});
      if (std::abs(actual.left - expected[0]) > 1e-7 || std::abs(actual.right - expected[1]) > 1e-7)
        throw std::runtime_error("Adapter wet-chain reference contract");
    }
  }
  filter.Clear();
  if (filter.Process({0, 0}).left != 0) throw std::runtime_error("Adapter clear contract");
  // Same production MIDI/envelope/FX code, a separate per-voice filter type.
  for (double rate : {44100., 48000., 96000., 192000.})
    for (int filterMode : {0, 1, 2, 3}) for (int mode : {0, 1, 2}) {
    auto s = std::make_unique<ProbeEngine::Synth>();
    Setup(*s, rate, 0, 20, true); s->SetVoiceMode(mode, 0, true);
    s->SetFilterCharacter(20, filterMode);
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
// Offline Synth API timeline only; this does not exercise VST3 automation.
void ModulationSmoke() {
  constexpr int frames = 8192;
  for (double rate : {48000., 96000., 192000.})
    for (int filterMode = 0; filterMode < 4; ++filterMode) for (int voiceMode = 0; voiceMode < 3; ++voiceMode) {
      std::vector<sawstar::StereoSample> reference(frames);
      for (int partition : {0, 1}) {
        auto s = std::make_unique<ProbeEngine::Synth>();
        Setup(*s, rate, 0, 20, true); s->SetVoiceMode(voiceMode, 15, true);
        s->SetFilterCharacter(20, filterMode);
        s->SetFilterEnvelope(60, 100, 1, 30, .3f, 100);
        s->SetPerformance(24, 24);
        s->SetLfo(20, 100, 3, 0, false, 0, 136, true);
        s->SetLfo2(17, 100, 2, 1, false, 0, 136, false);
        s->SetModulation(0, 3, 0, 100);  // wheel -> cutoff
        s->SetModulation(1, 4, 0, -100); // velocity -> cutoff
        s->SetModulation(2, 1, 2, 40);   // LFO1 -> amplitude
        s->SetModulation(3, 5, 1, 100);  // pressure -> pitch
        double energy = 0;
        int begin = 0;
        while (begin < frames) {
          constexpr std::array<int, 5> sizes{16, 32, 64, 256, 2048};
          const int end = std::min(frames, begin + (partition ? sizes[(begin / 16) % sizes.size()] : 1));
          for (int i = begin; i < end; ++i) {
            if (i == 0) {
              s->Midi(0x90, 36, 100); s->Midi(0x90, 60, 100); s->Midi(0x9f, 84, 100);
            }
            if (i % 128 == 0) {
              const int step = i / 128;
              s->SetFilter(step % 2 ? 40 : 18000, step % 3 * 50, step % 3 * 50);
              s->SetFilterCharacter(step % 3 == 0 ? 0 : step % 3 == 1 ? 20 : 24,
                                    (filterMode + step / 2) % 4);
              for (int ch : {0, 15}) {
                s->Midi(0xb0 | ch, 1, step % 2 ? 127 : 0);
                s->Midi(0xe0 | ch, step % 2 ? 127 : 0, step % 2 ? 127 : 0);
                s->Midi(0xd0 | ch, step % 2 ? 127 : 0, 0);
              }
            }
            if (i == 1024) { s->Midi(0xb0, 64, 127); s->Midi(0xbf, 64, 127); }
            if (i == 2048) s->Midi(0x80, 36, 0); // pedal-latched release
            if (i == 4096) {
              s->Midi(0xb0, 64, 0); s->Midi(0xbf, 64, 0);
              s->Midi(0x80, 36, 0); s->Midi(0x80, 60, 0); s->Midi(0x8f, 84, 0);
            }
            const auto y = s->ProcessStereo(); Check(y);
            const auto wet = s->PreFX();
            if (!std::isfinite(wet.left) || !std::isfinite(wet.right))
              throw std::runtime_error("Nonfinite modulation before FX");
            energy += double(wet.left) * wet.left + double(wet.right) * wet.right;
            if (partition == 0) reference[i] = y;
            else if (y.left != reference[i].left || y.right != reference[i].right)
              throw std::runtime_error("Offline sample timeline partition mismatch");
          }
          begin = end;
        }
        if (energy < 1e-12) throw std::runtime_error("Modulation fixture must sound");
        // Release is an exponential time constant, not a finite 100 ms ramp.
        for (int i = 0; i < static_cast<int>(rate); ++i) Check(s->ProcessStereo());
        if (s->Held(36) || s->Held(60) || s->Held(84) || s->ActiveVoices() != 0)
          throw std::runtime_error("Modulated release: rate=" + std::to_string(rate)
            + " filter=" + std::to_string(filterMode) + " voice=" + std::to_string(voiceMode)
            + " active=" + std::to_string(s->ActiveVoices()));
        s->Reset(rate);
        for (int i = 0; i < 64; ++i) {
          const auto y = s->ProcessStereo();
          if (y.left != 0 || y.right != 0) throw std::runtime_error("Modulated reset silence");
        }
      }
    }
  std::cout << "36 modulation scenes, two offline partitions, release/reset PASS\n";
}
template<class Synth> void Benchmark(const char* label, bool allModes) {
  using Clock = std::chrono::steady_clock;
  constexpr int frames = 4096;
  double checksum = 0;
  for (int filterMode : allModes ? std::vector<int>{0, 1, 2, 3} : std::vector<int>{1})
    for (double rate : {48000., 96000., 192000.}) for (int voices : {1, 8, 16})
    for (double drive : {0., 20., 24.}) for (bool fx : {false, true})
      for (int buffer : {32, 64, 128, 256}) {
        std::array<double, 3> times{}; double worst = 0, energy = 0, peak = 0;
        for (auto& time : times) {
          auto s = std::make_unique<Synth>(); Setup(*s, rate, voices, drive, fx, filterMode);
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
        std::cout << label << ',' << rate << ',' << voices;
        if (allModes) std::cout << ',' << filterMode;
        std::cout << ',' << drive << ',' << fx << ',' << buffer
          << ',' << 100 * times[1] * rate / frames << ',' << worst << ',' << peak
          << ',' << std::sqrt(energy / (frames * 2 * times.size())) << '\n';
      }
  if (!std::isfinite(checksum)) throw std::runtime_error("Invalid benchmark checksum");
}
// Focused offline deadline distribution: no host scheduling or CI timing gate.
template<class Synth> void Stress(const char* label) {
  using Clock = std::chrono::steady_clock;
  constexpr int blocks = 1024;
  std::cout << "engine,rate,voices,filter_mode,drive_db,fx,buffer,blocks,median_block_percent,p95_block_percent,p99_block_percent,worst_block_percent,over_budget_blocks,peak,rms\n";
  for (double rate : {48000., 96000., 192000.})
    for (int mode : {0, 1, 2, 3}) for (int buffer : {32, 64, 128}) {
      auto s = std::make_unique<Synth>(); Setup(*s, rate, 16, 20, true, mode);
      for (int i = 0; i < static_cast<int>(rate / 4); ++i) Check(s->ProcessStereo());
      std::array<double, blocks> times{};
      double energy = 0, peak = 0; int over = 0;
      for (auto& time : times) {
        const auto start = Clock::now();
        for (int i = 0; i < buffer; ++i) {
          const auto y = s->ProcessStereo(); Check(y);
          energy += double(y.left) * y.left + double(y.right) * y.right;
          peak = std::max(peak, double(std::max(std::abs(y.left), std::abs(y.right))));
        }
        time = 100 * rate / buffer * std::chrono::duration<double>(Clock::now() - start).count();
        if (time > 100) ++over;
      }
      if (s->ActiveVoices() != 16 || energy <= 0 || !std::isfinite(energy))
        throw std::runtime_error("Stress fixture must hold 16 sounding voices");
      std::sort(times.begin(), times.end());
      // Nearest-rank percentiles over the measured blocks.
      std::cout << label << ',' << rate << ",16," << mode << ",20,1," << buffer << ',' << blocks
        << ',' << (times[blocks / 2 - 1] + times[blocks / 2]) / 2
        << ',' << times[(95 * blocks + 99) / 100 - 1]
        << ',' << times[(99 * blocks + 99) / 100 - 1]
        << ',' << times.back() << ',' << over << ',' << peak
        << ',' << std::sqrt(energy / (2 * blocks * buffer)) << '\n';
    }
}
int main(int argc, char** argv) {
  try {
    if (argc == 2 && std::string(argv[1]) == "--smoke") { Smoke(); return 0; }
    if (argc == 2 && std::string(argv[1]) == "--modulation") { ModulationSmoke(); return 0; }
    if (argc == 2 && std::string(argv[1]) == "--stress-legacy") { Stress<sawstar::Synth>("legacy"); return 0; }
    if (argc == 2 && std::string(argv[1]) == "--stress") {
#ifdef SAWSTAR_PREMIUM_HIGH_RATE_STUDY
      Stress<ProbeEngine::Synth>("rate-scaled");
#else
      Stress<ProbeEngine::Synth>("premium");
#endif
      return 0;
    }
    bool legacy = false, allModes = false;
    for (int i = 1; i < argc; ++i) {
      const std::string arg = argv[i];
      if (arg == "--legacy" && !legacy) legacy = true;
      else if (arg == "--all-modes" && !allModes) allModes = true;
      else { std::cerr << "Usage: premium_engine_benchmark [--smoke|--modulation|--stress|--stress-legacy|[--legacy] [--all-modes]]\n"; return 1; }
    }
    std::cout << "engine,rate,voices,";
    if (allModes) std::cout << "filter_mode,";
    std::cout << "drive_db,fx,buffer,median_realtime_percent,worst_block_percent,peak,rms\n";
    if (legacy) Benchmark<sawstar::Synth>("legacy", allModes);
    else {
#ifdef SAWSTAR_PREMIUM_HIGH_RATE_STUDY
      Benchmark<ProbeEngine::Synth>("rate-scaled", allModes);
#else
      Benchmark<ProbeEngine::Synth>("premium", allModes);
#endif
    }
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
