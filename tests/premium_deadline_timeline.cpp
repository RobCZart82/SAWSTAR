// SPDX-License-Identifier: MIT
// Observe actual benchmark setter calls, independently of the DSP dependency.
#include "../experiments/premium_filter/DeadlineModulation.h"
#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>
using Event = std::array<double, 5>;
struct Recorder {
  std::vector<Event> events;
  void SetLfo(float a,float b,int c,int d,bool e,int f,int g,bool h) {
    if(a!=20||b!=70||c!=3||d!=0||e||f!=0||g!=120||!h) throw std::runtime_error("LFO1 setup");
    events.push_back({0,0,0,0,0});
  }
  void SetLfo2(float a,float b,int c,int d,bool e,int f,int g,bool h) {
    if(a!=17||b!=50||c!=2||d!=1||e||f!=0||g!=120||h) throw std::runtime_error("LFO2 setup");
    events.push_back({1,0,0,0,0});
  }
  void SetModulation(int a,int b,int c,float d) { events.push_back({2,double(a),double(b),double(c),d}); }
  void SetFilter(float a,float b,float c) { events.push_back({3,a,b,c,0}); }
  void SetFilterCharacter(float a,int b) { events.push_back({4,a,double(b),0,0}); }
  void Midi(int a,int b,int c) { events.push_back({5,double(a),double(b),double(c),0}); }
};
void Require(bool ok) { if(!ok) throw std::runtime_error("Deadline modulation timeline mismatch"); }
std::vector<Event> Run(int buffer,int mode) {
  Recorder s;
  sawstar::experimental::PrepareDeadlineModulation(s);
  Require(s.events.size()==5);
  s.events.clear();
  for(std::uint64_t base=0;base<8192;base+=buffer)
    for(int n=0;n<buffer;++n) sawstar::experimental::DeadlineModulationEvent(s,base+n,mode);
  std::array<int,6> counts{};
  for(const auto& e:s.events) {
    ++counts[int(e[0])];
    if(e[0]==3) Require(e[1]>=500&&e[1]<=6500&&e[2]==50&&(e[3]==35||e[3]==100));
    if(e[0]==4) Require((e[1]==0||e[1]==12||e[1]==24)&&e[2]==mode);
    if(e[0]==5) {
      Require(e[1]==0xb0||e[1]==0xd0||e[1]==0xe0);
      if(e[1]==0xb0) Require(e[2]==1&&(e[3]==0||e[3]==127));
      if(e[1]==0xd0) Require((e[2]==0||e[2]==127)&&e[3]==0);
      if(e[1]==0xe0) Require(e[2]==0&&(e[3]==64||e[3]==80));
    }
  }
  Require(counts[2]==4&&counts[3]==64&&counts[4]==16&&counts[5]==48);
  Require(s.events.front()==Event{3,500,50,100,0});
  Require(s.events[1]==Event{4,0,double(mode),0,0});
  return s.events;
}
int main() {
  try {
    for(int mode=0;mode<4;++mode) {
      const auto expected=Run(32,mode);
      Require(Run(64,mode)==expected&&Run(128,mode)==expected&&Run(32,mode)==expected);
      Recorder noEvent; sawstar::experimental::DeadlineModulationEvent(noEvent,127,mode);
      Require(noEvent.events.empty());
      Recorder warm; sawstar::experimental::DeadlineModulationEvent(warm,12000,mode);
      Require(warm.events.empty()); // no event at this 48 kHz warmup boundary
      sawstar::experimental::DeadlineModulationEvent(warm,12032,mode);
      Require(warm.events.size()==1&&warm.events[0][0]==3);
    }
    std::cout<<"Versioned modulation timeline, buffer invariance and warmup offset PASS\n";
  } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
