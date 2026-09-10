// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/Safety.h"
#include "Control/adsr.h"
#include <algorithm>
#include <cmath>

namespace sawstar {
// Per-voice envelope; cutoff targets update every 16 samples, independently of
// host block size. LowPass smooths the resulting coefficient targets.
class FilterModulation {
public:
  void Init(float rate) { rate=SafeSampleRate(rate); env_.Init(rate); limit_=std::min(20000.f,rate*.45f); tick_=0; }
  void Set(float base,float amount,float tracking,float attack,float decay,float sustain,float release) {
    base_=Safe(base,20,20000); amount_=Safe(amount,-96,96); tracking_=Safe(tracking,0,100)*.01f;
    env_.SetAttackTime(Safe(attack,1,10000)*.001f);
    env_.SetDecayTime(Safe(decay,1,10000)*.001f);
    env_.SetSustainLevel(Safe(sustain,0,1));
    env_.SetReleaseTime(Safe(release,1,10000)*.001f);
  }
  void Trigger(bool hard) { env_.Retrigger(hard); tick_=0; }
  float Process(int note,bool gate,float wheelSemitones=0) {
    note=std::clamp(note,0,127);wheelSemitones=FiniteClamp(wheelSemitones,-192.f,192.f,0.f);
    const float envelope=env_.Process(gate);
    if(tick_++==0) {
      const float semitones=amount_*envelope+tracking_*(note-60)+wheelSemitones;
      cutoff_=std::clamp(base_*(semitones==0?1.f:std::exp2(semitones/12.f)),20.f,limit_);
    }
    if(tick_==16) tick_=0;
    return cutoff_;
  }
private:
  static float Safe(float x,float lo,float hi){return std::isfinite(x)?std::clamp(x,lo,hi):lo;}
  daisysp::Adsr env_;
  float base_=12000,amount_=0,tracking_=0,limit_=20000,cutoff_=12000;
  int tick_=0;
};
}
