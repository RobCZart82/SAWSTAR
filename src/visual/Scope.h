// SPDX-License-Identifier: MIT
#pragma once
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>

namespace sawstar {
// One audio producer, one editor/idle consumer. Neither ever waits for the other.
// Capture native samples: no decimation, filtering or alteration of the sound.
class Scope {
public:
  static constexpr unsigned Capacity=16384;
  struct Frame {
    std::array<float,Capacity> samples{};
    unsigned count=0;
    double rate=44100;
    std::chrono::steady_clock::time_point time{};
  };
  static_assert(std::atomic<unsigned>::is_always_lock_free,"Scope requires lock-free indices");
  void Reset(double rate) { // Audio thread only; never reset queue ownership indices.
    rate_=std::isfinite(rate)&&rate>0?rate:44100;
    position_=count_=ticks_=0;
    interval_=static_cast<unsigned>(std::clamp(rate_/30.,1.,1000000.));
  }
  void Push(float value) {
    ring_[position_]=std::isfinite(value)?value:0.f;
    position_=(position_+1)%Capacity;
    count_=std::min(count_+1,Capacity);
    if(++ticks_<interval_)return;
    ticks_=0;
    const unsigned write=write_.load(std::memory_order_relaxed),next=(write+1)%Slots;
    if(next==read_.load(std::memory_order_acquire))return; // Drop, never block audio.
    auto& frame=frames_[write];frame.count=count_;frame.rate=rate_;
    const unsigned first=(position_+Capacity-count_)%Capacity;
    for(unsigned i=0;i<count_;++i)frame.samples[i]=ring_[(first+i)%Capacity];
    frame.time=std::chrono::steady_clock::now();
    write_.store(next,std::memory_order_release);
  }
  bool Latest(Frame& result) {
    bool found=false;
    // Bounded drain even if producer runs concurrently.
    for(unsigned i=0;i<Slots-1;++i){
      const unsigned read=read_.load(std::memory_order_relaxed);
      if(read==write_.load(std::memory_order_acquire))break;
      result=frames_[read];
      read_.store((read+1)%Slots,std::memory_order_release);found=true;
    }
    return found;
  }
private:
  static constexpr unsigned Slots=3;
  std::array<float,Capacity> ring_{};
  std::array<Frame,Slots> frames_{};
  std::atomic<unsigned> read_{0},write_{0};
  unsigned position_=0,count_=0,ticks_=0,interval_=1470;
  double rate_=44100;
};

// A fixed 32 ms view, rising zero-crossing trigger with a small hysteresis.
// Keep the original samples; the renderer uses per-pixel extrema at high pitches.
inline unsigned ScopeStart(const Scope::Frame& f,unsigned& length){
  length=std::min(f.count,static_cast<unsigned>(std::clamp(f.rate*.032,2.,double(Scope::Capacity/2))));
  if(length<2)return 0;
  const unsigned latest=f.count-length;
  const unsigned begin=latest>length?latest-length:0;
  float peak=0;for(unsigned i=begin;i<f.count;++i)peak=std::max(peak,std::abs(f.samples[i]));
  const float threshold=std::max(.00001f,peak*.02f);
  bool armed=false;
  for(unsigned i=begin;i<=latest;++i){
    if(f.samples[i]<-threshold)armed=true;
    if(armed&&f.samples[i]>=0)return i;
  }
  return latest; // Noise/untriggered audio still refreshes.
}
}
