// SPDX-License-Identifier: MIT
#pragma once
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
namespace sawstar {
// Single audio producer, one editor consumer. No allocation or UI calls in audio.
// Accumulate block peaks until consumed so short transients survive GUI throttling.
class MeterMailbox {
  static_assert(std::atomic<uint64_t>::is_always_lock_free, "Meter requires lock-free packets");
  std::array<std::atomic<uint64_t>,2> peaks_{};
  std::array<std::atomic<bool>,2> clips_{};
  static float Amplitude(uint64_t packet)noexcept{
    const uint32_t bits=uint32_t(packet);float value;std::memcpy(&value,&bits,sizeof(value));return value;
  }
public:
  static uint32_t Now()noexcept{
    return uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
  }
  void Publish(float left,float right,uint32_t now=Now()) noexcept {
    const float values[]={left,right};
    for(int ch=0;ch<2;++ch){
      const float value=std::isfinite(values[ch])?std::max(0.f,values[ch]):0.f;
      if(value>=1.f)clips_[ch].store(true,std::memory_order_relaxed);
      uint32_t bits;std::memcpy(&bits,&value,sizeof(bits));
      const uint64_t packet=(uint64_t(now)<<32)|bits;
      auto previous=peaks_[ch].load(std::memory_order_relaxed);
      // Timestamp and peak travel together. Unsigned subtraction handles clock wrap.
      while((!previous||now-uint32_t(previous>>32)>200||Amplitude(previous)<=value)
            &&!peaks_[ch].compare_exchange_weak(previous,packet,std::memory_order_relaxed)){}
    }
  }
  std::array<float,2> Take(uint32_t now=Now()) noexcept {
    std::array<float,2> result{};
    for(int ch=0;ch<2;++ch){const auto packet=peaks_[ch].exchange(0,std::memory_order_relaxed);
      if(packet&&now-uint32_t(packet>>32)<=200)result[ch]=Amplitude(packet);}
    return result;
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
