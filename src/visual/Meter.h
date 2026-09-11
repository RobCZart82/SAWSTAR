// SPDX-License-Identifier: MIT
#pragma once
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
namespace sawstar {
// Single audio producer, one editor consumer. No allocation or UI calls in audio.
// Accumulate block peaks until consumed so short transients survive GUI throttling.
class MeterMailbox {
  static_assert(std::atomic<float>::is_always_lock_free, "Meter requires lock-free floats");
  std::array<std::atomic<float>,2> peaks_{};
  std::array<std::atomic<bool>,2> clips_{};
public:
  void Publish(float left,float right) noexcept {
    const float values[]={left,right};
    for(int ch=0;ch<2;++ch){
      const float value=std::isfinite(values[ch])?std::max(0.f,values[ch]):0.f;
      if(value>=1.f)clips_[ch].store(true,std::memory_order_relaxed);
      float previous=peaks_[ch].load(std::memory_order_relaxed);
      while(previous<value&&!peaks_[ch].compare_exchange_weak(previous,value,std::memory_order_relaxed)){}
    }
  }
  std::array<float,2> Take() noexcept {
    return {peaks_[0].exchange(0.f,std::memory_order_relaxed),peaks_[1].exchange(0.f,std::memory_order_relaxed)};
  }
  bool Clipped(int ch)const noexcept{return clips_[ch].load(std::memory_order_relaxed);}
  void ClearClip(int ch)noexcept{clips_[ch].store(false,std::memory_order_relaxed);}
  void Reset()noexcept{Take();ClearClip(0);ClearClip(1);}
};
// Editor-owned ballistics; wall-clock timing remains correct after redraw pauses.
class MeterDisplay {
public:
  using Clock=std::chrono::steady_clock;
  static constexpr int Cells=20;
  void Update(float amplitude,Clock::time_point now){
    const float dt=ready_?std::max(0.f,std::chrono::duration<float>(now-last_).count()):0.f;
    const float input=std::isfinite(amplitude)&&amplitude>0.f?std::clamp(20.f*std::log10(amplitude),-60.f,0.f):-60.f;
    level_=std::max(input,level_-24.f*dt);
    if(input>-60.f&&input>=hold_){hold_=input;until_=now+std::chrono::seconds(1);}
    else if(ready_&&now>until_){
      const float fall=std::chrono::duration<float>(now-std::max(last_,until_)).count();
      hold_=std::max(input,hold_-18.f*fall);
    }
    hold_=std::max(-60.f,hold_);
    for(int i=0;i<Cells;++i){
      const bool active=level_>(-60.f+60.f*i/Cells);
      glow_[i]=active?1.f:glow_[i]*std::exp(-dt/.065f);
    }
    last_=now;ready_=true;
  }
  float Brightness(int cell)const{return glow_[cell];}
  int HoldCell()const{return hold_<=-60.f?-1:std::min(Cells-1,int((hold_+60.f)/60.f*Cells));}
  float LevelDb()const{return level_;}
  float HoldDb()const{return hold_;}
private:
  Clock::time_point last_{},until_{};
  bool ready_=false;
  float level_=-60.f,hold_=-60.f;
  std::array<float,Cells> glow_{};
};
}
