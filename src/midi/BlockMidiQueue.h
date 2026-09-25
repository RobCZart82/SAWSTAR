// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <cstddef>
namespace sawstar {
struct BlockMidiEvent { int offset=0,status=0,data1=0,data2=0; bool fromEditor=false; };
// Audio-thread-owned, fixed-capacity queue. Equal offsets retain arrival order.
// Host callbacks must be serialized by the adapter; this is not a UI-thread FIFO.
class BlockMidiQueue {
public:
 static constexpr std::size_t Capacity=1024;
 void Clear() noexcept {count_=0;overflow_=false;}
 void Push(BlockMidiEvent event) noexcept {
  if(count_==Capacity){overflow_=true;return;}
  if(event.offset<0)event.offset=0;
  auto pos=count_++;
  while(pos>0&&events_[pos-1].offset>event.offset){events_[pos]=events_[pos-1];--pos;}
  events_[pos]=event;
 }
 std::size_t Size()const noexcept{return count_;}
 template<class Send,class Sample,class Recover>
 void Process(int frames,Send send,Sample sample,Recover recover){
  if(overflow_){Clear();recover();}
  if(frames<=0)return;
  std::size_t event=0;
  for(int i=0;i<frames;++i){
   while(event<count_&&events_[event].offset<=i)send(events_[event++]);
   sample(i);
  }
  for(std::size_t i=event;i<count_;++i){events_[i-event]=events_[i];events_[i-event].offset-=frames;}
  count_-=event;
 }
private:
 std::array<BlockMidiEvent,Capacity> events_{};
 std::size_t count_=0;
 bool overflow_=false;
};
// Overflow drops the uncertain MIDI burst and silences all channels/ARP/FX.
// Retain bend/mod controller values, matching All Sound Off semantics.
template<class Arp,class Synth> void RecoverMidiOverflow(Arp& arp,Synth& synth){
 arp.Clear([&](int s,int n,int v){synth.Midi(s,n,v);});
 for(int ch=0;ch<16;++ch)synth.Midi(0xb0|ch,120,0);
}
template<class Value,class Sample>
void WriteHostOutput(const Value& value,Sample** outputs,int channels,int frame){
 if(channels==1)outputs[0][frame]=static_cast<Sample>((value.left+value.right)*.5f);
 else for(int ch=0;ch<channels;++ch)outputs[ch][frame]=static_cast<Sample>(ch%2?value.right:value.left);
}
}
