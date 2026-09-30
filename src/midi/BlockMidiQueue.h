// SPDX-License-Identifier: MIT
#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstddef>
namespace sawstar {
struct BlockMidiEvent { int offset=0,status=0,data1=0,data2=0; bool fromEditor=false; };
// Audio-thread-owned, fixed-capacity queue. Equal offsets retain arrival order.
// Host callbacks must be serialized by the adapter; this is not a UI-thread FIFO.
class BlockMidiQueue {
public:
 static constexpr std::size_t Capacity=1024;
 void Clear() noexcept {count_=0;overflow_=false;controllerOrder_=0;controllers_={};}
 void Push(BlockMidiEvent event) noexcept {
  if(event.offset<0)event.offset=0;
  if(overflow_){RememberController(event);return;}
  if(count_==Capacity){
   overflow_=true;
   for(std::size_t i=0;i<count_;++i)RememberController(events_[i]);
   RememberController(event);
   return;
  }
  auto pos=count_++;
  while(pos>0&&events_[pos-1].offset>event.offset){events_[pos]=events_[pos-1];--pos;}
  events_[pos]=event;
 }
 std::size_t Size()const noexcept{return count_;}
 template<class Send,class Sample,class Recover>
 void Process(int frames,Send send,Sample sample,Recover recover){
  if(overflow_){
   // All notes in the uncertain burst are discarded, but do not strand a
   // wheel at its previously applied value. Replay only the final value per
   // controller/channel, with CC121 retained to preserve reset ordering.
   // No early application: future offsets still wait for their audio sample.
   auto saved=controllers_;
   Clear();recover();
   std::sort(saved.begin(),saved.end(),[](const auto& a,const auto& b){
    if(a.valid!=b.valid)return a.valid;
    if(a.event.offset!=b.event.offset)return a.event.offset<b.event.offset;
    return a.order<b.order;
   });
   for(const auto& controller:saved)if(controller.valid)Push(controller.event);
  }
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
 struct Controller {BlockMidiEvent event{};std::uint64_t order=0;bool valid=false;};
 void RememberController(const BlockMidiEvent& event) noexcept {
  const int kind=event.status&0xf0;
  const int slot=kind==0xe0?0:(kind==0xb0&&event.data1==1?1:
                             (kind==0xb0&&event.data1==121?2:-1));
  if(slot<0)return;
  auto& saved=controllers_[(event.status&15)*3+slot];
  const auto order=controllerOrder_++;
  if(!saved.valid||event.offset>=saved.event.offset)saved={event,order,true};
 }
 // Audio-thread-only fixed storage; source tags and same-offset arrival order
 // survive recovery. This does not recover events lost in the upstream UI FIFO.
 std::array<Controller,16*3> controllers_{};
 std::uint64_t controllerOrder_=0;
 std::array<BlockMidiEvent,Capacity> events_{};
 std::size_t count_=0;
 bool overflow_=false;
};
// Silence all channels/ARP/FX after discarding the uncertain note burst.
// CC120 itself retains applied bend/mod values. BlockMidiQueue separately
// replays surviving controller targets at their original sample offsets.
template<class Arp,class Synth> void RecoverMidiOverflow(Arp& arp,Synth& synth){
 arp.Clear([&](int s,int n,int v){synth.Midi(s,n,v);});
 for(int ch=0;ch<16;++ch)synth.Midi(0xb0|ch,120,0);
}
template<class Value,class Sample>
void WriteHostOutput(const Value& value,Sample** outputs,int channels,int frame,bool muted=false){
 if(muted){for(int ch=0;ch<channels;++ch)outputs[ch][frame]=Sample(0);return;}
 if(channels==1)outputs[0][frame]=static_cast<Sample>((value.left+value.right)*.5f);
 else for(int ch=0;ch<channels;++ch)outputs[ch][frame]=static_cast<Sample>(ch%2?value.right:value.left);
}
}
