// SPDX-License-Identifier: MIT
#include "dsp/Adsr.h"
#include "Control/adsr.h"
#include <algorithm>
#include <cmath>
#include <iostream>

int main() {
  for (const int rate : {44100, 48000, 96000}) {
    sawstar::Adsr stable;
    daisysp::Adsr reference;
    stable.Init(static_cast<float>(rate));
    reference.Init(static_cast<float>(rate));
    stable.SetAttackTime(.01f); reference.SetAttackTime(.01f);
    stable.SetDecayTime(.1f); reference.SetDecayTime(.1f);
    stable.SetSustainLevel(.7f); reference.SetSustainLevel(.7f);
    stable.SetReleaseTime(.1f); reference.SetReleaseTime(.1f);
    float maxDifference = 0.f;
    for (int i = 0; i < rate / 4; ++i)
      maxDifference = std::max(maxDifference,
          std::abs(stable.Process(true) - reference.Process(true)));
    for (int i = 0; i < rate / 4; ++i)
      maxDifference = std::max(maxDifference,
          std::abs(stable.Process(false) - reference.Process(false)));
    if (maxDifference > 2.e-5f) {
      std::cerr << "Short ADSR response changed at " << rate
                << " Hz; max difference=" << maxDifference << '\n';
      return 1;
    }
  }

  float value = 0.f;
  for (const int rate : {44100, 48000, 96000, 192000, 384000}) {
    sawstar::Adsr attack;
    attack.Init(static_cast<float>(rate));
    attack.SetAttackTime(1.f);
    attack.SetDecayTime(10.f);
    attack.SetSustainLevel(.5f);
    attack.SetReleaseTime(1.f);
    value = 0.f;
    for (int i = 0; i < rate * 101 / 100; ++i) value = attack.Process(true);
    if (!std::isfinite(value) || attack.GetCurrentSegment() != sawstar::Adsr::Decay ||
        value < .999f) {
      std::cerr << "Exactly 1 s attack stalled at " << rate
                << " Hz; value=" << value << '\n';
      return 1;
    }

    sawstar::Adsr envelope;
    envelope.Init(static_cast<float>(rate));
    envelope.SetAttackTime(10.f);
    envelope.SetDecayTime(10.f);
    envelope.SetSustainLevel(.2f);
    envelope.SetReleaseTime(10.f);

    value = 0.f;
    for (int i = 0; i < rate * 101 / 10; ++i)
      value = envelope.Process(true);
    if (envelope.GetCurrentSegment() != sawstar::Adsr::Decay ||
        !std::isfinite(value) || value < .99f || value > 1.001f) {
      std::cerr << "10 s attack failed to enter decay at " << rate
                << " Hz; value=" << value << '\n';
      return 1;
    }

    for (int i = 0; i < rate * 120; ++i)
      value = envelope.Process(true);
    if (!std::isfinite(value) || value < .2f || value > .201f) {
      std::cerr << "Long decay failed to converge at " << rate
                << " Hz; value=" << value << '\n';
      return 1;
    }
  }

  // At exactly one second the legacy float recurrence could stop moving at
  // high sample rates. Exercise the boundary separately from the >1 s case.
  for (const int rate : {44100, 48000, 96000, 192000, 384000}) {
    sawstar::Adsr envelope;
    envelope.Init(static_cast<float>(rate));
    envelope.SetAttackTime(.001f);
    envelope.SetDecayTime(1.f);
    envelope.SetSustainLevel(.99f);
    envelope.SetReleaseTime(.1f);
    float value = 0.f;
    for (int i = 0; i < rate * 12; ++i) value = envelope.Process(true);
    if (!std::isfinite(value) || std::abs(value - .99f) > 2.e-5f) {
      std::cerr << "Exactly 1 s decay stalled at " << rate
                << " Hz; value=" << value << '\n';
      return 1;
    }

    sawstar::Adsr release;
    release.Init(static_cast<float>(rate));
    release.SetAttackTime(.001f);
    release.SetDecayTime(.1f);
    release.SetSustainLevel(.5f);
    release.SetReleaseTime(1.f);
    for (int i = 0; i < rate * 2; ++i) release.Process(true);
    for (int i = 0; i < rate * 12; ++i) value = release.Process(false);
    if (!std::isfinite(value) || release.GetCurrentSegment() != sawstar::Adsr::Idle ||
        value != 0.f) {
      std::cerr << "Exactly 1 s release stalled at " << rate
                << " Hz; value=" << value << '\n';
      return 1;
    }
  }
  return 0;
}
