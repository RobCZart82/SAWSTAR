// SPDX-License-Identifier: MIT
#include "plugin/TailContract.h"
#include "plugin/Parameters.h"
#include "engine/Synth.h"
#include "dsp/Effects/Chorus.h"
#include "dsp/Effects/Delay.h"
#include "dsp/Effects/Reverb.h"
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>

void check(bool ok, const char* why) {
  if (!ok) { std::cerr << why << '\n'; std::exit(1); }
}
struct HostReport {
  double rate = 44100.;
  int tail = 0;
  double GetSampleRate() const { return rate; }
  void SetTailSize(int value) { tail = value; }
#include "tail_report_hook.inc"
};

void chain(float rate, bool pingPong, bool changed, bool completeTail = true) {
  // Production envelope and FX, in the exact Synth signal order. Warm real
  // buffers before release; no separate normalization of the measured tail.
  auto chorus = std::make_unique<sawstar::Chorus>();
  sawstar::Delay delay; sawstar::Reverb reverb; sawstar::Adsr amp;
  chorus->Init(rate); delay.Init(rate); reverb.Init(rate); amp.Init(rate);
  amp.SetAttackTime(.001f); amp.SetDecayTime(.001f);
  amp.SetSustainLevel(1.f); amp.SetReleaseTime(10.f);
  chorus->Set(true, 100, .05f, 100);
  delay.Set(true, 100, changed ? 350 : 2000, 85, 200, pingPong, false, 0, 120);
  reverb.Set(true, 100, changed ? 0 : 100, changed ? .2f : 10, 500);
  const int warm = static_cast<int>(4 * rate);
  // Debug exercises the complete maximum envelope release at both rates.
  // Release and extended sanitizer/coverage builds additionally measure the
  // residual after all 380 seconds. A smoke run is not a residual proof.
  const int end = completeTail ? sawstar::TailSamples(rate) : static_cast<int>(48 * rate);
  std::cout << "Starting FX chain, " << rate << " Hz, full-tail=" << completeTail << std::endl;
  float latePeak = 0, tenSecondPeak = 0;
  double lateEnergy = 0; int lateCount = 0;
  for (int i = -warm; i < end + static_cast<int>(rate); ++i) {
    if (i == 0 && changed) {
      delay.Set(true, 100, 2000, 85, 16000, pingPong, false, 0, 120);
      reverb.Set(true, 100, 100, 10, 16000);
    }
    const float level = amp.Process(i < 0);
    const float source = level == 0.f ? 0.f :
        level * (.25f + .1f * std::sin(i * 502.65482457 / rate));
    const auto y = reverb.Process(delay.Process(chorus->Process({source, source * .7f})));
    // Include the maximum +24 dB output boost in absolute residual checks.
    const float left = y.left * 15.848932f, right = y.right * 15.848932f;
    check(std::isfinite(left) && std::isfinite(right), "Non-finite tail");
    const float peak = std::max(std::abs(left), std::abs(right));
    if (i >= 10 * rate && i < 11 * rate) tenSecondPeak = std::max(tenSecondPeak, peak);
    if (i >= end) {
      latePeak = std::max(latePeak, peak);
      lateEnergy += double(left) * left + double(right) * right;
      ++lateCount;
    }
  }
  check(tenSecondPeak > 1.e-4f, "Fixture does not expose incorrect 10-second tail");
  check(!amp.IsRunning(), "Envelope still active at reported tail end");
  if (completeTail)
    check(latePeak < 1.e-6f && std::sqrt(lateEnergy / (2 * lateCount)) < 1.e-6,
          "Actual FX residual exceeds reported finite tail budget");
  else
    check(latePeak > 1.e-6f, "FX smoke fixture has no continuing history");
  std::cout << rate << " Hz, ping-pong=" << pingPong << ", changed=" << changed
            << ", full-tail=" << completeTail << ": late peak=" << latePeak << std::endl;
}

int main() {
  using namespace sawstar;
#if defined(SAWSTAR_EXTENDED_TAIL_RENDER)
  constexpr bool completeTail = true;
#else
  constexpr bool completeTail = false;
#endif
  // If parameter limits change, the independently reviewed budget must change.
  check(kParameters[size_t(ParameterId::AmpRelease)].maximum == 10000 &&
        kParameters[size_t(ParameterId::DelayTime)].maximum == 2000 &&
        kParameters[size_t(ParameterId::DelayFeedback)].maximum == 85 &&
        kParameters[size_t(ParameterId::ReverbDecay)].maximum == 10 &&
        kParameters[size_t(ParameterId::ChorusDepth)].maximum == 100 &&
        kParameters[size_t(ParameterId::ReverbSize)].minimum == 0 &&
        kParameters[size_t(ParameterId::ReverbSize)].maximum == 100 &&
        kParameters[size_t(ParameterId::OutputBoost)].maximum == 24,
        "Tail budget no longer matches DSP parameter limits");
  HostReport host; host.initialize();
  check(host.tail > 0 && host.tail == TailSamples(44100), "Missing initial host tail report");
  for (double rate : {8000., 44100., 48000., 96000., 192000., 384000., 44100.001}) {
    host.rate = rate; host.reset();
    check(host.tail > 10 * rate && host.tail < std::numeric_limits<int>::max(),
          "Zero/short/infinite host tail report");
    check(host.tail >= TailBudgetSeconds() * rate &&
          host.tail < TailBudgetSeconds() * rate + 1.,
          "Tail report has wrong sample-rate conversion");
  }
  check(TailSamples(NAN) == TailSamples(44100) && TailSamples(INFINITY) == TailSamples(44100),
        "Invalid rate not sanitized");
  check(TailSamples(-1) == TailSamples(8000) && TailSamples(1.e9) == TailSamples(384000),
        "Unsupported rate outside DSP limits");
  // Complete production Synth: 16 voices, all sources, maximum release/FX/boost.
  auto synth = std::make_unique<Synth>(); synth->Reset(8000);
  synth->SetParameters(0, 1, 1, 1, 10000);
  synth->SetMixer(100, 100, 100, 100, 0, -1, 0, 0);
  synth->SetOutputBoost(24);
  synth->SetChorus(true, 100, .05f, 100);
  synth->SetDelay(true, 100, 2000, 85, 200, true, false, 0, 120);
  synth->SetReverb(true, 100, 100, 10, 500);
  synth->SetWidth(true, 100);
  for (int note = 48; note < 64; ++note) synth->Midi(0x90, note, 100);
  for (int i = 0; i < 4 * 8000; ++i) synth->ProcessStereo();
  synth->Midi(0xb0, 64, 0);
  for (int note = 48; note < 64; ++note) synth->Midi(0x80, note, 0);
  float peakAfterBudget = 0;
  // Keep a short 16-voice Debug prefix: the full 10-second time-constant
  // release renders over 80 million oscillators before the voices become idle.
  // Maximum envelope convergence is independently checked in every FX chain.
  const int budget = completeTail ? TailSamples(8000) : 2 * 8000;
  std::cout << "Starting 16-voice engine, full-tail=" << completeTail << std::endl;
  for (int i = 0; i < budget + 8000; ++i) {
    const auto y = synth->ProcessStereo();
    check(std::isfinite(y.left) && std::isfinite(y.right), "Non-finite full-engine tail");
    if (i == 48 * 8000) check(synth->ActiveVoices() == 0, "Max-release voices failed to finish");
    if (i >= budget) peakAfterBudget = std::max(peakAfterBudget,
        std::max(std::abs(y.left), std::abs(y.right)));
  }
  if (completeTail) {
    check(synth->ActiveVoices() == 0, "Max-release chord exceeds source-tail allowance");
    check(peakAfterBudget < 1.e-6f, "Full-engine tail exceeds advertised budget");
  } else {
    check(synth->ActiveVoices() == 16, "Short engine smoke lost a max-release voice");
    check(peakAfterBudget > 1.e-6f, "Engine smoke fixture has no continuing FX history");
  }
  std::cout << "16-voice engine: late peak=" << peakAfterBudget << std::endl;
  chain(8000, false, false, completeTail);
  chain(8000, true, true, completeTail);
  chain(44100, true, false, completeTail);
}
