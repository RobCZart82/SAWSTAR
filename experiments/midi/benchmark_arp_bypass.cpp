// SPDX-License-Identifier: MIT
// Standalone microbenchmark: g++ -std=c++17 -O2 -Isrc <this file> -o /tmp/arp-bench
// Run from a checkout. Compare identical compiler/options on the parent and fix.
#include "midi/Arpeggiator.h"
#include <chrono>
#include <iostream>

int main() {
  sawstar::Arpeggiator arp;
  arp.Init(8000);
  unsigned long events=0;
  volatile unsigned long long checksum=0;
  auto send=[&](int status,int note,int value) {
    ++events;
    checksum=checksum*131+status*16384+note*128+value;
  };
  arp.Set(false,4,3,80,4,0,false,1000,true,send);
  for(int note=0;note<128;++note)arp.Midi(0x90,note,100,send);
  const auto start=std::chrono::steady_clock::now();
  for(int i=0;i<20000;++i) {
    arp.Midi(0x90,i%128,1+i%127,send);
    arp.Midi(0x80,i%128,0,send);
  }
  const auto ms=std::chrono::duration<double,std::milli>(
      std::chrono::steady_clock::now()-start).count();
  // Consume retained roots after timing, keeping their state observable.
  arp.Set(true,4,3,80,4,0,false,1000,true,send);
  for(int i=0;i<50000;++i)arp.Process(send);
  arp.Clear(send);
  std::cout<<ms<<" ms; events="<<events<<"; checksum="<<checksum<<'\n';
}
