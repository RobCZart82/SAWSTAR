// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <algorithm>
#include <cmath>
namespace sawstar {
void Synth::Reset(double rate) {
  const float sr = static_cast<float>(std::isfinite(rate) && rate >= 8000 ? rate : 44100);
  bend_.fill(8192);mod_.fill(0);bendRatio_.fill(1);bendTarget_.fill(1);
  sustain_.fill(false); age_ = 0; gain_ = 0;
  boost_=targetBoost_; protection_=1;
  protectionRelease_=1.f-std::exp(-1.f/(0.08f*sr));
  smoothing_ = 1.f - std::exp(-1.f / (0.005f * sr));
  for (auto& v : voices_) {
    v = Voice{};
    v.osc.Init(sr); v.env.Init(sr); v.filter.Init(sr); v.filterMod.Init(sr);
  }
}
void Synth::SetOutputBoost(float dB) {
  dB=std::isfinite(dB)?std::clamp(dB,0.f,24.f):0.f;
  targetBoost_=std::pow(10.f,dB/20.f);
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
  cutoff_=cutoff; resonance_=resonance; filterMix_=mix;
  SetFilterEnvelope(amount_,tracking_,filterAttack_,filterDecay_,filterSustain_,filterRelease_);
}
void Synth::SetFilterEnvelope(float amount,float tracking,float attack,float decay,float sustain,float release) {
  amount_=amount; tracking_=tracking; filterAttack_=attack; filterDecay_=decay;
  filterSustain_=sustain; filterRelease_=release;
  for(auto& v:voices_) v.filterMod.Set(cutoff_,amount,tracking,attack,decay,sustain,release);
}
void Synth::UpdateBend(int ch) {
  const float norm=(bend_[ch]-8192)/(bend_[ch]>=8192?8191.f:8192.f);
  bendTarget_[ch]=std::exp2(norm*bendRange_/12.f);
}
void Synth::SetPerformance(float range,float depth) {
  range=std::isfinite(range)?std::clamp(range,0.f,24.f):2;
  modDepth_=std::isfinite(depth)?std::clamp(depth,0.f,48.f):24;
  if(range!=bendRange_){bendRange_=range;for(int ch=0;ch<16;++ch)UpdateBend(ch);}
}
void Synth::SetSaw(float detune,float mix,float width) {
  for(auto& v:voices_) v.osc.SetShape(detune,mix*.01f,width*.01f);
}
void Synth::Midi(int status, int note, int value) {
  const int channel = status & 15, kind = status & 240;
  if (note < 0 || note > 127 || value < 0 || value > 127) return;
  if(kind==0xe0){bend_[channel]=note+(value<<7);UpdateBend(channel);return;}
  if (kind == 0x90 && value > 0) {
    // Repeated note retriggers one voice; idle, then oldest released, then oldest held.
    Voice* chosen = nullptr;
    for (auto& v : voices_) if (v.note == note && v.channel == channel) { chosen = &v; break; }
    if (!chosen) for (auto& v : voices_) if (v.note < 0) { chosen = &v; break; }
    if (!chosen) for (auto& v : voices_) if (!v.held && (!chosen || v.age < chosen->age)) chosen = &v;
    if (!chosen) chosen = &*std::min_element(voices_.begin(), voices_.end(), [](const Voice& a, const Voice& b) { return a.age < b.age; });
    auto& v = *chosen;
    v.filterMod.Trigger(v.note<0);
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
    if(note==1){mod_[channel]=value;return;}
    if(note==121){bend_[channel]=8192;mod_[channel]=0;UpdateBend(channel);}
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
  for(int ch=0;ch<16;++ch)bendRatio_[ch]+=smoothing_*(bendTarget_[ch]-bendRatio_[ch]);
  for (auto& v : voices_) if (v.note >= 0) {
    const float env = v.env.Process(v.gate);
    v.filter.Set(v.filterMod.Process(v.note,v.gate,mod_[v.channel]/127.f*modDepth_),resonance_,filterMix_);
    v.osc.SetPitchMultiplier(bendRatio_[v.channel]);
    const auto value=v.filter.Process(v.osc.Process());
    sum.left+=value.left*env*v.velocity;sum.right+=value.right*env*v.velocity;
    if (!v.gate && !v.env.IsRunning()) v.note = -1;
  }
  gain_ += smoothing_ * (targetGain_ - gain_);
  boost_ += smoothing_ * (targetBoost_ - boost_);
  const float scale=gain_*boost_/16.f;
  sum.left*=scale; sum.right*=scale;
  // Stereo-linked peak guard: instant attack, 80 ms recovery, zero latency.
  // At settled 0 dB boost use the historical path for old project recall.
  if(targetBoost_==1.f && std::abs(boost_-1.f)<0.00001f) {
    protection_=1;
  } else {
    const float peak=std::max(std::abs(sum.left),std::abs(sum.right));
    const float required=peak>0.98f ? 0.98f/peak : 1.f;
    protection_+=protectionRelease_*(1.f-protection_);
    protection_=std::min(protection_,required);
  }
  return {std::clamp(sum.left*protection_, -1.f, 1.f),
          std::clamp(sum.right*protection_, -1.f, 1.f)};
}
bool Synth::Held(int note) const {
  for (const auto& v : voices_) if (v.note == note && v.held) return true;
  return false;
}
int Synth::ActiveVoices() const {
  int count = 0; for (const auto& v : voices_) if (v.note >= 0) ++count; return count;
}
}
