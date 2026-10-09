// SPDX-License-Identifier: MIT
#if defined(SAWSTAR_SAW_DISPATCH_STUDY)
#include "SawDispatchSevenSaw.h"
using Candidate = sawstar::experimental_oscillator::SawDispatchSevenSaw;
constexpr const char* Variant = "saw-dispatch-v1";
#elif defined(SAWSTAR_SHARED_FREQUENCY_STUDY)
#include "SharedFrequencySevenSaw.h"
using Candidate = sawstar::experimental_oscillator::SharedFrequencySevenSaw;
constexpr const char* Variant = "shared-frequency-v1";
#else
#include "CachedSevenSaw.h"
using Candidate = sawstar::experimental_oscillator::CachedSevenSaw;
constexpr const char* Variant = "held-saw-tuning-v2";
#endif
#include <array>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
struct Campaign {
  bool extended = false;
  bool referenceRepeat = false;
  int Frames() const { return extended ? 131072 : 8192; }
  int Pairs() const { return extended ? 8 : 4; }
};
template<class S> void Measure(const char* path, int rate, int wave, int workload, int pair,
                              const Campaign& campaign) {
  std::array<S, 32> bank; // OSC1 + OSC2 for sixteen voices; no synth/FX workload.
  for (int i = 0; i < 32; ++i) {
    bank[i].Init(float(rate)); bank[i].SetFreq(65.4064f * (1 + i % 8));
    bank[i].SetShape(25, .7f, .8f); bank[i].SetWaveform(wave); bank[i].SnapToTargets();
  }
  auto process = [&](int n) {
    double energy = 0;
    for (int i = 0; i < 32; ++i) {
      if (workload) bank[i].SetPitchMultiplier(1.f + .01f * std::sin(float(n) * .001f));
      auto y = bank[i].Process(); energy += double(y.left) * y.left + double(y.right) * y.right;
    }
    return energy;
  };
  for (int n = 0; n < rate / 4; ++n) process(n);
  const int frames = campaign.Frames();
  double energy = 0; const auto start = std::chrono::steady_clock::now();
  for (int n = 0; n < frames; ++n) energy += process(rate / 4 + n);
  const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
  if (!std::isfinite(energy) || energy <= 0 || !std::isfinite(seconds) || seconds <= 0)
    throw std::runtime_error("Invalid oscillator measurement");
  std::cout << path << ',' << rate << ',' << wave << ',' << workload << ',' << pair << ','
            << (pair % 2 ? "study-first" : "reference-first") << ",32," << frames << ',' << seconds << ',' << energy
            << ',' << Variant;
  if (campaign.extended)
    std::cout << ",extended-v1," << (campaign.referenceRepeat ? "reference-repeat" : "candidate")
              << ',' << rate / 4;
  std::cout << '\n';
}
int main(int argc, char** argv) {
  try {
    Campaign campaign;
    for (int i = 1; i < argc; ++i) {
      const std::string option = argv[i];
      if (option == "--extended" && !campaign.extended) campaign.extended = true;
      else if (option == "--reference-repeat" && !campaign.referenceRepeat) campaign.referenceRepeat = true;
      else throw std::runtime_error("Unknown or duplicate benchmark option");
    }
    if (campaign.referenceRepeat && !campaign.extended)
      throw std::runtime_error("Reference repeat requires the extended campaign");
#if !defined(SAWSTAR_SHARED_FREQUENCY_STUDY) && !defined(SAWSTAR_SAW_DISPATCH_STUDY)
    if (campaign.extended) throw std::runtime_error("Extended campaign requires a sharing or dispatch study");
#endif
    std::cout << std::setprecision(17) << "path,rate,waveform,modulated,pair,order,oscillators,frames,seconds,energy,variant";
    if (campaign.extended) std::cout << ",campaign,comparison,warmup_frames";
    std::cout << '\n';
    for (int rate : {48000, 96000, 192000}) for (int wave = 0; wave < 4; ++wave)
      for (int workload = 0; workload < 2; ++workload) for (int pair = 0; pair < campaign.Pairs(); ++pair) {
        if (pair % 2 == 0) Measure<sawstar::SevenSaw>("reference", rate, wave, workload, pair, campaign);
        // Both repeat paths instantiate the identical function/type in this executable.
        if (campaign.referenceRepeat)
          Measure<sawstar::SevenSaw>("study", rate, wave, workload, pair, campaign);
        else Measure<Candidate>("study", rate, wave, workload, pair, campaign);
        if (pair % 2) Measure<sawstar::SevenSaw>("reference", rate, wave, workload, pair, campaign);
      }
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
