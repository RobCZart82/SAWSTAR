// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <algorithm>
#include <cmath>
namespace sawstar {
void Synth::Reset(double rate) {
  const float sr = static_cast<float>(std::isfinite(rate) && rate >= 8000 ? rate : 44100);
  sustain_.fill(false); age_ = 0; gain_ = 0;
  smoothing_ = 1.f - std::exp(-1.f / (0.005f * sr));
  for (auto& v : voices_) {
    v = Voice{};
    v.osc.Init(sr); v.env.Init(sr); v.filter.Init(sr);
  }
}
void Synth::SetParameters(double gain, double attack, double decay, double sustain, double release) {
  targetGain_ = static_cast<float>(std::pow(10., gain / 20.));
  for (auto& v : voices_) {
    v.env.SetAttackTime(static_cast<float>(attack * .001));
    v.env.SetDecayTime(static_cast<float>(decay * .001));
    v.env.SetSustainLevel(static_cast<float>(sustain));
    v.env.SetReleaseTime(static_cast<float>(release * .001));
  }
}
void Synth::SetFilter(float cutoff,float resonance,float mix) {
  for(auto& v:voices_) v.filter.Set(cutoff,resonance,mix);
}
void Synth::SetSaw(float detune,float mix,float width) {
  for(auto& v:voices_) v.osc.SetShape(detune,mix*.01f,width*.01f);
}
void Synth::Midi(int status, int note, int value) {
  const int channel = status & 15, kind = status & 240;
  if (note < 0 || note > 127 || value < 0 || value > 127) return;
  if (kind == 0x90 && value > 0) {
    // Repeated note retriggers one voice; idle, then oldest released, then oldest held.
    Voice* chosen = nullptr;
    for (auto& v : voices_) if (v.note == note && v.channel == channel) { chosen = &v; break; }
    if (!chosen) for (auto& v : voices_) if (v.note < 0) { chosen = &v; break; }
    if (!chosen) for (auto& v : voices_) if (!v.held && (!chosen || v.age < chosen->age)) chosen = &v;
    if (!chosen) chosen = &*std::min_element(voices_.begin(), voices_.end(), [](const Voice& a, const Voice& b) { return a.age < b.age; });
    auto& v = *chosen;
    if(v.note<0) v.filter.Clear();
    v.note = note; v.channel = channel; v.held = v.gate = true;
    v.velocity = static_cast<float>(value) / 127.f; v.age = ++age_;
    v.osc.SetFreq(static_cast<float>(440. * std::pow(2., (note - 69) / 12.)));
    v.env.Retrigger(false);
  } else if (kind == 0x80 || (kind == 0x90 && value == 0)) {
    for (auto& v : voices_) if (v.note == note && v.channel == channel) {
      v.held = false; v.gate = sustain_[channel];
    }
  } else if (kind == 0xB0) {
    if (note == 64 || note == 121) {
      sustain_[channel] = note == 64 && value >= 64;
      if (!sustain_[channel]) for (auto& v : voices_) if (v.channel == channel && !v.held) v.gate = false;
    } else if (note == 120 || note == 123) {
      for (auto& v : voices_) if (v.channel == channel) {
        v.held = v.gate = false;
        if (note == 120) { v.note = -1; v.env.Retrigger(true); }
      }
      sustain_[channel] = false;
    }
  }
}
StereoSample Synth::ProcessStereo() {
  StereoSample sum;
  for (auto& v : voices_) if (v.note >= 0) {
    const float env = v.env.Process(v.gate);
    const auto value=v.filter.Process(v.osc.Process());
    sum.left+=value.left*env*v.velocity;sum.right+=value.right*env*v.velocity;
    if (!v.gate && !v.env.IsRunning()) v.note = -1;
  }
  gain_ += smoothing_ * (targetGain_ - gain_);
  // Fixed 16-voice headroom; no level pumping when voices enter or leave.
  return {std::clamp(sum.left * gain_ / 16.f, -1.f, 1.f),
          std::clamp(sum.right * gain_ / 16.f, -1.f, 1.f)};
}
bool Synth::Held(int note) const {
  for (const auto& v : voices_) if (v.note == note && v.held) return true;
  return false;
}
int Synth::ActiveVoices() const {
  int count = 0; for (const auto& v : voices_) if (v.note >= 0) ++count; return count;
}
}
