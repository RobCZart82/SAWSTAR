// SPDX-License-Identifier: MIT
#include "Synthesis/oscillator.h"
#include "Control/adsr.h"
#include <cmath>
#include <iostream>

int main() {
  for (const int rate : {44100, 48000, 96000}) {
    daisysp::Oscillator oscillator;
    oscillator.Init(static_cast<float>(rate));
    oscillator.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_SAW);
    oscillator.SetFreq(440.f);
    oscillator.SetAmp(1.f);
    daisysp::Adsr envelope;
    envelope.Init(static_cast<float>(rate));
    envelope.SetAttackTime(0.01f);
    envelope.SetDecayTime(0.1f);
    envelope.SetSustainLevel(0.7f);
    envelope.SetReleaseTime(0.1f);
    double energy = 0.;
    int crossings = 0;
    float previous = 0.f, last = 0.f;
    for (int i = 0; i < rate * 3; ++i) {
      const float raw = oscillator.Process();
      const float amp = envelope.Process(i < rate);
      last = raw * amp;
      if (!std::isfinite(last) || std::abs(raw) > 1.01f || amp < 0.f || amp > 1.01f) {
        std::cerr << "Invalid oscillator/envelope output at " << rate << '\n';
        return 1;
      }
      if (i < rate) {
        energy += last * last;
        if (previous < 0.f && raw >= 0.f) ++crossings;
      }
      previous = raw;
    }
    if (energy / rate < 0.01 || std::abs(crossings - 440) > 2 ||
        std::abs(last) > 1.e-5f || envelope.IsRunning()) {
      std::cerr << "Saw pitch, energy or release check failed at " << rate << '\n';
      return 1;
    }
  }
  return 0;
}
