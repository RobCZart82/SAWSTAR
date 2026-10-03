// SPDX-License-Identifier: MIT
#include "PremiumLowPass.h"
#include "dsp/LowPass.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void U32(std::ostream& out, uint32_t x) {
  for (int i = 0; i < 4; ++i) out.put(static_cast<char>((x >> (i * 8)) & 255));
}
void U16(std::ostream& out, uint16_t x) {
  out.put(static_cast<char>(x & 255)); out.put(static_cast<char>(x >> 8));
}
void Write(const std::string& path, const std::vector<float>& samples, uint32_t rate) {
  std::ofstream out(path, std::ios::binary);
  if (!out) throw std::runtime_error("Cannot open output: " + path);
  const auto bytes = static_cast<uint32_t>(samples.size() * 4);
  out.write("RIFF", 4); U32(out, 48 + bytes); out.write("WAVEfmt ", 8);
  U32(out, 16); U16(out, 3); U16(out, 2); U32(out, rate); U32(out, rate * 8);
  U16(out, 8); U16(out, 32);
  out.write("fact", 4); U32(out, 4); U32(out, static_cast<uint32_t>(samples.size() / 2));
  out.write("data", 4); U32(out, bytes);
  for (float sample : samples) {
    uint32_t bits; static_assert(sizeof(bits) == sizeof(sample));
    std::memcpy(&bits, &sample, 4); U32(out, bits);
  }
  if (!out) throw std::runtime_error("Failed writing output: " + path);
}
}
int main(int argc, char** argv) {
  if (argc != 2) { std::cerr << "Usage: premium_filter_preview OUTPUT_PREFIX\n"; return 1; }
  try {
    constexpr int rate = 48000, count = rate * 12;
    sawstar::SevenSaw source; source.Init(rate);
    source.SetFreq(130.81278265f); source.SetShape(20, 60, 75); source.SnapToTargets();
    sawstar::LowPass legacy; legacy.Init(rate); legacy.Set(8000, 30, 100);
    legacy.SetCharacter(0, 1); legacy.SnapToTargets();
    sawstar::experimental::PremiumLowPass candidate; candidate.Init(rate);
    candidate.Set(8000, 30); candidate.SnapToTargets();
    std::vector<float> a, b; a.reserve(count * 2); b.reserve(count * 2);
    double peakA = 0, peakB = 0;
    for (int i = 0; i < count; ++i) {
      const double t = static_cast<double>(i) / rate;
      // Three seconds to hear the open source before the sweep begins.
      const double position = std::clamp((t - 3) / 5., 0., 1.);
      const double cutoff = 8000 * std::pow(180. / 8000, position);
      legacy.Set(static_cast<float>(cutoff), 30, 100); candidate.Set(cutoff, 30);
      const double fade = std::min(std::clamp(t / .5, 0., 1.),
                                   std::clamp((12 - t) / .5, 0., 1.));
      const double envelope = fade * fade * (3 - 2 * fade);
      auto x = source.Process(); x.left *= .06f; x.right *= .06f;
      const auto old = legacy.Process(x);
      const auto next = candidate.Process({x.left, x.right});
      for (float value : {old.left, old.right}) {
        const float y = static_cast<float>(value * envelope);
        if (!std::isfinite(y)) throw std::runtime_error("Nonfinite legacy output");
        peakA = std::max(peakA, std::abs(static_cast<double>(y))); a.push_back(y);
      }
      for (float value : next) {
        const float y = static_cast<float>(value * envelope);
        if (!std::isfinite(y)) throw std::runtime_error("Nonfinite candidate output");
        peakB = std::max(peakB, std::abs(static_cast<double>(y))); b.push_back(y);
      }
    }
    if (peakA >= 1 || peakB >= 1) throw std::runtime_error("Preview exceeds full scale");
    Write(std::string(argv[1]) + "-A-legacy-LP24.wav", a, rate);
    Write(std::string(argv[1]) + "-B-candidate-LP24.wav", b, rate);
    std::cout << "Equal source and gain; no output normalization. Peaks: " << peakA << ", " << peakB << '\n';
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
