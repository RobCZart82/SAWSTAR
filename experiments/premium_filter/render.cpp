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
  if (argc < 2 || argc > 4) { std::cerr << "Usage: premium_filter_preview OUTPUT_PREFIX [RESONANCE_PERCENT [sustain|lead|pluck|pad]]\n"; return 1; }
  try {
    double resonance = 30;
    if (argc >= 3) {
      size_t used = 0;
      resonance = std::stod(argv[2], &used);
      if (used != std::strlen(argv[2]) || !std::isfinite(resonance) || resonance < 0 || resonance > 100)
        throw std::runtime_error("Resonance must be finite and within 0..100");
    }
    const std::string scene = argc == 4 ? argv[3] : "sustain";
    if (scene != "sustain" && scene != "lead" && scene != "pluck" && scene != "pad")
      throw std::runtime_error("Unknown preview scene");
    constexpr int rate = 48000, count = rate * 12;
    sawstar::SevenSaw source; source.Init(rate);
    source.SetFreq(scene == "lead" ? 261.6255653f : 130.81278265f);
    source.SetShape(20, 1, 1); source.SnapToTargets();
    std::array<sawstar::SevenSaw, 2> chord;
    for (size_t i = 0; i < chord.size(); ++i) {
      chord[i].Init(rate); chord[i].SetFreq(i == 0 ? 164.81377846f : 195.99771799f);
      chord[i].SetShape(20, 1, 1); chord[i].SnapToTargets();
    }
    sawstar::LowPass legacy; legacy.Init(rate); legacy.Set(8000, static_cast<float>(resonance), 100);
    legacy.SetCharacter(0, 1); legacy.SnapToTargets();
    sawstar::experimental::PremiumLowPass candidate; candidate.Init(rate);
    candidate.Set(8000, resonance); candidate.SnapToTargets();
    std::vector<float> a, b; a.reserve(count * 2); b.reserve(count * 2);
    double peakA = 0, peakB = 0;
    for (int i = 0; i < count; ++i) {
      const double t = static_cast<double>(i) / rate;
      // Three seconds to hear the open source before the sweep begins.
      const double position = std::clamp((t - 3) / 5., 0., 1.);
      const double cutoff = 8000 * std::pow(180. / 8000, position);
      legacy.Set(static_cast<float>(cutoff), static_cast<float>(resonance), 100); candidate.Set(cutoff, resonance);
      const double fade = std::min(std::clamp(t / .5, 0., 1.),
                                   std::clamp((12 - t) / .5, 0., 1.));
      const double envelope = fade * fade * (3 - 2 * fade);
      auto x = source.Process();
      if (scene == "pad") {
        for (auto& oscillator : chord) {
          const auto note = oscillator.Process(); x.left += note.left; x.right += note.right;
        }
        x.left /= 3; x.right /= 3;
      }
      double articulation = 1;
      if (scene == "pluck") {
        const double age = std::fmod(t, .5);
        articulation = std::min(1., age / .002) * std::exp(-age / .11)
                     * std::clamp((.5 - age) / .03, 0., 1.);
      } else if (scene == "pad") {
        const double attack = std::clamp(t / 1.5, 0., 1.);
        articulation = attack * attack * (3 - 2 * attack);
      }
      const float sourceGain = static_cast<float>((scene == "sustain" ? .06 : .04) * articulation);
      x.left *= sourceGain; x.right *= sourceGain;
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
    // One constant offline gain over the common 0.5..11.5 s listening window.
    // No compressor, time-varying leveling or modification of either filter.
    double energyA = 0, energyB = 0;
    for (size_t i = rate; i < static_cast<size_t>(rate * 23); ++i) {
      energyA += static_cast<double>(a[i]) * a[i];
      energyB += static_cast<double>(b[i]) * b[i];
    }
    if (energyA <= 0 || energyB <= 0) throw std::runtime_error("Silent RMS reference");
    const double gain = std::sqrt(energyA / energyB);
    std::vector<float> matched = b;
    for (auto& sample : matched) {
      sample = static_cast<float>(sample * gain);
      if (!std::isfinite(sample) || std::abs(sample) >= 1)
        throw std::runtime_error("Matched preview exceeds full scale");
    }
    Write(std::string(argv[1]) + "-B-RMS-matched-LP24.wav", matched, rate);
    std::cout << "B constant RMS-match gain (0.5..11.5 s): " << gain
              << " / dB: " << 20 * std::log10(gain) << '\n';
    std::cout << "Equal source and gain; no output normalization. Peaks: " << peakA << ", " << peakB << '\n';
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
