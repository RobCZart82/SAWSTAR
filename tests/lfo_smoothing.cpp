// SPDX-License-Identifier: MIT
#include "dsp/Lfo.h"
#include <cfenv>
#include <cmath>
#include <iostream>
#include <limits>

int main() {
  std::fenv_t saved;
  if (std::fegetenv(&saved) != 0 || std::fesetenv(FE_DFL_ENV) != 0) return 1;
  volatile float normal = std::numeric_limits<float>::min();
  volatile float half = .5f;
  const bool gradual = std::fpclassify(normal * half) == FP_SUBNORMAL;
  if (!gradual) std::cout << "Runtime flushes subnormals; underflow check not applicable\n";
  bool passed = true;
  for (float rate : {44100.f, 48000.f, 96000.f, 192000.f, 384000.f}) {
    for (int target = 0; target < 4; ++target) {
      for (bool reroute : {false, true}) {
        sawstar::Lfo lfo;
        lfo.Init(rate);
        lfo.Set(1, 100, 3, target, false, 0, 120, false);
        for (int i = 0; i < int(rate); ++i) lfo.Process();
        lfo.Set(1, reroute ? 100 : 0, 3, reroute ? (target + 1) % 4 : target,
                false, 0, 120, false);
        std::feclearexcept(FE_ALL_EXCEPT);
        sawstar::LfoOutput output;
        for (int i = 0; i < int(rate * 2); ++i) output = lfo.Process();
        const float oldTarget = target == 0 ? output.cutoff : target == 1 ? output.pitch :
                                target == 2 ? output.amp : output.pan;
        if (oldTarget != (target == 2 ? 1.f : 0.f) ||
            (gradual && std::fetestexcept(FE_UNDERFLOW))) {
          std::cerr << "LFO zero-target tail: rate=" << rate << " target=" << target
                    << " reroute=" << reroute << '\n';
          passed = false;
        }
      }
    }
  }
  std::fesetenv(&saved);
  return passed ? 0 : 1;
}
