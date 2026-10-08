// SPDX-License-Identifier: MIT
#if defined(SAWSTAR_SHARED_FREQUENCY_STUDY)
#include "../experiments/oscillator/SharedFrequencySevenSaw.h"
#else
#include "../experiments/oscillator/CachedSevenSaw.h"
#endif
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
using Reference = sawstar::SevenSaw;
#if defined(SAWSTAR_SHARED_FREQUENCY_STUDY)
using Candidate = sawstar::experimental_oscillator::SharedFrequencySevenSaw;
constexpr int Scenes = 4;
static_assert(sizeof(Candidate) == sizeof(Reference), "No added oscillator state");
#else
using Candidate = sawstar::experimental_oscillator::CachedSevenSaw;
constexpr int Scenes = 3;
#endif
void Equal(sawstar::StereoSample a, sawstar::StereoSample b) {
  if (!std::isfinite(a.left) || !std::isfinite(a.right) ||
      !std::isfinite(b.left) || !std::isfinite(b.right) ||
      std::memcmp(&a.left, &b.left, sizeof(float)) ||
      std::memcmp(&a.right, &b.right, sizeof(float)))
    throw std::runtime_error("Oscillator cache bit identity");
}
template<class S> void Setup(S& s, float rate, int wave) {
  s.Init(rate); s.SetFreq(261.6256f); s.SetShape(25, .7f, .8f);
  s.SetWaveform(wave); s.SnapToTargets();
}
template<class S> void Event(S& s, int n, int wave, int scene) {
  if (!scene) return;
  if (scene == 3) s.SetPitchMultiplier(1.f + .01f * std::sin(float(n) * .001f));
  if (n % 128 == 0) {
    s.SetFreq(scene == 2 ? 20000.f : (n % 256 ? 65.4064f : 2093.005f));
    s.SetPitchMultiplier(scene == 2 ? 4096.f : (n % 256 ? .5f : 2.f));
  }
  if (n % 257 == 0) s.SetShape((n / 257) % 2 ? 50.f : 0.f, (n / 257) % 2 ? 1.f : 0.f, .75f);
  if (n % 511 == 0) s.SetWaveform((wave + n / 511) % 4);
  if (n == 1024 || n == 6000) s.SnapToTargets();
  if (n == 2048) { s.SetFreq(0); s.SetPitchMultiplier(0); }
  if (n == 2112) s.SetPitchMultiplier(-0.f);
  if (n == 2176) s.SetFreq(-0.f);
  if (n == 2300) { s.SetFreq(std::numeric_limits<float>::denorm_min()); s.SetPitchMultiplier(1); }
  if (n == 2304) {
    s.SetFreq(std::numeric_limits<float>::quiet_NaN());
    s.SetPitchMultiplier(std::numeric_limits<float>::infinity());
    s.SetShape(std::numeric_limits<float>::infinity(), -1, 2);
    s.SetWaveform(-1);
  }
  if (n == 4096) Setup(s, 96000, (wave + 1) % 4);
}
int main() {
  try {
    bool rejected = false;
    try { Equal({.1f, 0}, {.1001f, 0}); } catch (const std::runtime_error&) { rejected = true; }
    if (!rejected) throw std::runtime_error("Negative audio control accepted");
    int cases = 0; double energy = 0;
    for (float rate : {8000.f, 44100.f, 48000.f, 96000.f, 192000.f, 384000.f})
      for (int wave = 0; wave < 4; ++wave) for (int scene = 0; scene < Scenes; ++scene) {
        Reference a; Candidate b; Setup(a, rate, wave); Setup(b, rate, wave);
        double caseEnergy = 0;
        for (int n = 0; n < 8192; ++n) {
          Event(a, n, wave, scene); Event(b, n, wave, scene);
          auto x = a.Process(), y = b.Process(); Equal(x, y);
          caseEnergy += double(x.left) * x.left + double(x.right) * x.right;
          if (n == 3072) {
            auto copyA = a; auto copyB = b;
            for (int k = 0; k < 96; ++k) {
              Event(copyA, 3073 + k, wave, scene); Event(copyB, 3073 + k, wave, scene);
              Equal(copyA.Process(), copyB.Process());
            }
          }
        }
        if (caseEnergy < 1e-12) throw std::runtime_error("Silent comparison scene");
        energy += caseEnergy; ++cases;
      }
    if (cases != 24 * Scenes) throw std::runtime_error("Incomplete oscillator grid");
    std::cout << cases << " scenes, " << cases * (8192 + 96) << " stereo frames, bit identity PASS; energy=" << energy << '\n';
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
