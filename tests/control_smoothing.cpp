// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <cfenv>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>

int main() {
  // Observe arithmetic exceptions with gradual underflow where the runtime
  // supports it. FTZ hosts already suppress subnormals; do not claim that such
  // a run proves the old source's recurrence was safe on IEEE hosts.
  std::fenv_t saved;
  if (std::fegetenv(&saved) != 0 || std::fesetenv(FE_DFL_ENV) != 0) return 1;
  volatile float smallestNormal = std::numeric_limits<float>::min();
  volatile float half = .5f;
  const float probe = smallestNormal * half;
  const bool gradual = std::fpclassify(probe) == FP_SUBNORMAL;
  if (!gradual) std::cout << "Runtime flushes subnormals; underflow check is not applicable\n";
  bool passed = true;
  for (float rate : {44100.f, 48000.f, 96000.f, 192000.f, 384000.f}) {
    for (int control = 0; control < 3; ++control) {
      auto synth = std::make_unique<sawstar::Synth>();
      synth->Reset(rate);
      // With no voices, only the relevant controller/mixer smoothing can
      // underflow. This isolates the regression from audio/filter/FX tails.
      if (control == 0) synth->SetMixer(100, 100, 100, 100, 0, -1, 0, 0);
      if (control == 1) synth->Midi(0xb0, 1, 127);
      if (control == 2) synth->Midi(0xd0, 127, 0);
      for (int frame = 0; frame < int(rate / 10); ++frame) synth->ProcessStereo();
      if (control == 0) synth->SetMixer(0, 0, 0, 0, 0, -1, 0, 0);
      if (control == 1) synth->Midi(0xb0, 1, 0);
      if (control == 2) synth->Midi(0xd0, 0, 0);
      std::feclearexcept(FE_ALL_EXCEPT);
      for (int frame = 0; frame < int(rate * 2); ++frame) {
        const auto sample = synth->ProcessStereo();
        if (sample.left != 0 || sample.right != 0) passed = false;
      }
      if (gradual && std::fetestexcept(FE_UNDERFLOW)) {
        std::cerr << "Zero-target smoothing underflow: rate=" << rate
                  << " control=" << control << " (mixer=0, wheel=1, pressure=2)\n";
        passed = false;
      }
    }
  }
  std::fesetenv(&saved);
  return passed ? 0 : 1;
}
