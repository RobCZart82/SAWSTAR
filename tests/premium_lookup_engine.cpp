// SPDX-License-Identifier: MIT
// Synth API control timeline only: not VST3 automation or native acceptance.
#ifdef SAWSTAR_UNROLLED_FIR_STUDY
#include "UnrolledPremiumSynth.h"
#else
#include "RateGainPremiumSynth.h"
#endif
#include "LookupPremiumSynth.h"
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#ifdef SAWSTAR_UNROLLED_FIR_STUDY
using Reference = sawstar::experimental_lookup_engine::Synth;
using Study = sawstar::experimental_unrolled_engine::Synth;
#else
using Reference = sawstar::experimental_rate_gain_engine::Synth;
using Study = sawstar::experimental_lookup_engine::Synth;
#endif
void Require(bool ok, const char* why) { if (!ok) throw std::runtime_error(why); }
struct Error {
  double peak = 0, squared = 0, energy = 0;
  void Add(sawstar::StereoSample a, sawstar::StereoSample b) {
    for (auto pair : {std::pair<float, float>{a.left, b.left}, {a.right, b.right}}) {
      Require(std::isfinite(pair.first) && std::isfinite(pair.second), "Nonfinite engine comparison");
#ifdef SAWSTAR_UNROLLED_FIR_STUDY
      Require(std::memcmp(&pair.first, &pair.second, sizeof(float)) == 0, "Unrolled engine bit identity");
#endif
      const double delta = double(pair.first) - pair.second;
      peak = std::max(peak, std::abs(delta)); squared += delta * delta;
      energy += double(pair.second) * pair.second;
    }
  }
  double Relative() const { Require(energy > 1e-12, "Silent engine fixture"); return std::sqrt(squared / energy); }
  void Check() const { Require(peak <= 2e-6 && Relative() <= 1e-7, "Engine lookup error bounds"); }
};
template<class S> void Setup(S& s, double rate, int filter, int voice, bool fx) {
  s.Reset(rate); s.SetParameters(-12, 2, 50, .7, 10); s.SetOutputBoost(12);
  s.SetSaw(25, 70, 70); s.SetOsc2(19, 60, 80);
  s.SetMixer(70, 50, 20, 5, 0, -1, 0, 0);
  s.SetFilter(1800, 50, 100); s.SetFilterCharacter(20, filter);
  s.SetFilterEnvelope(60, 100, 1, 30, .3f, 40);
  s.SetVoiceMode(voice, 15, true); s.SetPerformance(24, 24);
  s.SetLfo(20, 70, 3, 0, false, 0, 136, true);
  s.SetLfo2(17, 50, 2, 1, false, 0, 136, false);
  s.SetModulation(0, 3, 0, 100); s.SetModulation(1, 4, 0, -50);
  s.SetModulation(2, 5, 1, 20);
  if (fx) {
    s.SetChorus(true, 20, .3, 30);
    s.SetDelay(true, 15, 250, 30, 2000, true, false, 2, 136);
    s.SetReverb(true, 15, 50, 2, 6000);
  }
  for (int note = 48; note < (voice == 0 ? 64 : 51); ++note) s.Midi(0x90, note, 100);
}
template<class S> void Event(S& s, int n, int filter, int voice) {
  if (n % 128 == 0) s.SetFilter(500 + 6000 * ((n / 128) % 8) / 7.f, 50, 100);
  if (n % 512 == 0) {
    const float db = (n / 512) % 3 * 12.f;
    s.SetFilterCharacter(db, filter);
    s.Midi(0xb0, 1, (n / 512) % 2 ? 127 : 0);
    s.Midi(0xd0, (n / 512) % 2 ? 127 : 0, 0);
  }
  if (n == 511) s.Midi(0xb0, 64, 127);
  if (n == 768) s.Midi(0x80, voice == 0 ? 48 : 50, 0);
  if (n == 1024) s.Midi(0x90, 36, 100); // Poly stealing or Mono fallback/Legato
  if (n == 1536) s.Midi(0xe0, 0, 127);
  if (n == 2048) { s.Midi(0x80, 36, 0); s.Midi(0xb0, 64, 0); }
  if (n == 2560) {
    for (int note = 48; note < (voice == 0 ? 64 : 51); ++note) s.Midi(0x80, note, 0);
    s.Midi(0xe0, 0, 64); s.Midi(0x90, 60, 80);
  }
  if (n == 3072) {
    s.SetFilterCharacter(20, (filter + 1) % 4);
    s.SetModulation(0, 3, 1, -30); // changing source/destination weights
    s.SetFilter(12000, 50, 35);    // delayed dry/wet blend
    s.Midi(0x80, 60, 0); s.Midi(0x90, 60, 80);
  }
  if (n == 3584) s.Midi(0x80, 60, 0);
}
int main() {
  try {
    // Exact comparison rejects in Add; bounded comparison rejects in Check.
    // Both rejection paths belong inside the negative control's handler.
    bool rejected = false;
    try {
      Error altered; altered.Add({.1f, .1f}, {.09f, .09f}); altered.Check();
    } catch (const std::runtime_error&) { rejected = true; }
    Require(rejected, "Engine comparison accepted altered audio");
    int cases = 0;
    std::cout << std::setprecision(17) << "rate,filter_mode,voice_mode,fx,output_peak_error,output_relative_rms,prefx_peak_error,prefx_relative_rms\n";
    for (double rate : {48000., 96000., 176400., 192000., 384000.})
      for (int filter = 0; filter < 4; ++filter) for (int voice = 0; voice < 3; ++voice)
        for (bool fx : {false, true}) {
          auto a = std::make_unique<Reference>(); auto b = std::make_unique<Study>();
          Setup(*a, rate, filter, voice, fx); Setup(*b, rate, filter, voice, fx);
          Require(a->ActiveVoices() == (voice == 0 ? 16 : 1) && a->ActiveVoices() == b->ActiveVoices(),
                  "Missing sounding voice fixture");
          Error out, pre;
          for (int n = 0; n < 4096; ++n) {
            Event(*a, n, filter, voice); Event(*b, n, filter, voice);
            const auto x = a->ProcessStereo(), y = b->ProcessStereo();
            Require(std::max({std::abs(x.left), std::abs(x.right), std::abs(y.left), std::abs(y.right)}) <= .98001f,
                    "Unprotected engine output");
            out.Add(y, x); pre.Add(b->PreFX(), a->PreFX());
            Require(a->ActiveVoices() == b->ActiveVoices(), "Voice lifecycle mismatch");
            for (int note : {36, 48, 50, 60}) Require(a->Held(note) == b->Held(note), "MIDI ownership mismatch");
          }
          for (int n = 0; n < int(rate / 10); ++n) {
            out.Add(b->ProcessStereo(), a->ProcessStereo()); pre.Add(b->PreFX(), a->PreFX());
          }
          Require(a->ActiveVoices() == 0 && b->ActiveVoices() == 0, "Release did not complete");
          out.Check(); pre.Check();
          std::cout << rate << ',' << filter << ',' << voice << ',' << fx << ',' << out.peak << ',' << out.Relative()
                    << ',' << pre.peak << ',' << pre.Relative() << '\n';
          a->Reset(rate); b->Reset(rate);
          for (int n = 0; n < 64; ++n) {
            const auto x = a->ProcessStereo(), y = b->ProcessStereo();
            Require(x.left == 0 && x.right == 0 && y.left == 0 && y.right == 0, "Reset silence mismatch");
          }
          ++cases;
        }
    Require(cases == 120, "Incomplete engine grid");
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}

