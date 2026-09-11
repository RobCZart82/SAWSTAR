// SPDX-License-Identifier: MIT
#pragma once
#include <algorithm>
#include <chrono>
#include <cmath>
namespace sawstar {
// Editor-owned timing only: never read or written by the audio thread.
class DisplayGate {
public:
  using Clock=std::chrono::steady_clock;
  bool Due(Clock::time_point now, std::chrono::milliseconds interval, bool force=false) {
    if(!ready_||force||now-last_>=interval){last_=now;ready_=true;return true;}
    return false;
  }
private:
  Clock::time_point last_{};
  bool ready_=false;
};
class CpuDisplay {
public:
  bool Update(float input, DisplayGate::Clock::time_point now) {
    if(!gate_.Due(now,std::chrono::milliseconds(250)))return false;
    input=std::isfinite(input)?std::max(0.f,input):0.f;
    value_=initialized_?value_*.75f+input*.25f:input;
    initialized_=true;return true;
  }
  float Value()const{return value_;}
private:
  DisplayGate gate_;float value_=0;bool initialized_=false;
};
// Auto scale follows louder frames immediately and quieter ones gently.
inline float ScopeDisplayPeak(float previous,float peak){
  peak=std::max(.05f,peak);
  return std::max(peak,previous*.8f+peak*.2f);
}
}
