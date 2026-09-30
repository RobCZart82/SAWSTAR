// SPDX-License-Identifier: MIT
#include "midi/EditorWheelMailbox.h"
#include "midi/BlockMidiQueue.h"
#include "engine/Synth.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
using namespace sawstar;
void check(bool value,const char* message){if(!value){std::cerr<<message<<'\n';std::exit(1);}}
int main(){
 EditorWheelMailbox wheel;
 auto synth=std::make_unique<Synth>();synth->Reset(48000);
 auto send=[&](int status,int a,int b){synth->Midi(status,a,b);};
 BlockMidiQueue fifo;
 // A full legacy FIFO cannot retain the final wheel return. The new route
 // intercepts these messages before enqueueing, leaving note capacity intact.
 for(int i=0;i<1024;++i)fifo.Push({0,0x90,60,100});
 synth->Midi(0xe0,0,100);synth->Midi(0xb0,1,99);
 for(int i=0;i<100000;++i){
  check(wheel.Publish(0xe0,127,127),"pitch was not intercepted");
  check(wheel.Publish(0xb0,1,127),"CC1 was not intercepted");
 }
 check(wheel.Publish(0xe0,0,64)&&wheel.Publish(0xb0,1,0),"wheel return rejected");
 check(fifo.Size()==1024,"wheel route consumed note queue space");
 wheel.Drain(send);
 check(synth->PitchBend(0)==8192&&synth->ModWheel(0)==0,"final return lost");
 int count=0;wheel.Drain([&](int,int,int){++count;});check(count==0,"stale wheel replayed");
 // All channels and full 14-bit bend range remain independent.
 for(int ch=0;ch<16;++ch){wheel.Publish(0xe0|ch,ch,127);wheel.Publish(0xb0|ch,1,ch);}
 wheel.Drain(send);
 for(int ch=0;ch<16;++ch)check(synth->PitchBend(ch)==16256+ch&&synth->ModWheel(ch)==ch,"channel/value lost");
 check(!wheel.Publish(0x90,60,100)&&!wheel.Publish(0xb0,64,127)&&
       !wheel.Publish(0xb0,121,0)&&!wheel.Publish(0xe0,0,64,3),"non-wheel or scheduled MIDI intercepted");
 check(!wheel.Publish(0xe0,128,0)&&!wheel.Publish(0xe0,0,-1),"invalid data accepted");
 wheel.Publish(0xe0,0,0);wheel.Clear();wheel.Drain([&](int,int,int){++count;});check(count==0,"reset left pending wheel");
 // Production order: drain UI state, then process time-stamped host MIDI.
 // A zero-frame block doesn't drain. Host same-offset reset wins; later host
 // values change only at their offset; no UI repaint/state repeats next block.
 wheel.Publish(0xe0,0,0);wheel.Publish(0xb0,1,100);
 BlockMidiQueue host;host.Push({0,0xb0,121,0});host.Push({3,0xb0,1,53});
 host.Process(0,[&](auto e){send(e.status,e.data1,e.data2);},[](int){},[]{}); // No samples: no drain either.
 wheel.Drain(send);
 host.Process(4,[&](auto e){send(e.status,e.data1,e.data2);},[&](int i){
  check(synth->PitchBend(0)==8192,"host reset overwritten by GUI pitch");
  check(synth->ModWheel(0)==(i<3?0:53),"host control timing changed");
 },[]{});
 wheel.Drain(send);check(synth->ModWheel(0)==53,"old GUI state overwrote host next block");
 // Concurrent UI publication/audio consumption: final zero values must remain
 // observable whether exchanged before or after the producer finishes.
 std::atomic<bool> done{false};
 std::thread producer([&]{
  for(int i=0;i<100000;++i){wheel.Publish(0xe0,i&127,(i>>7)&127);wheel.Publish(0xb0,1,i&127);}
  wheel.Publish(0xe0,0,64);wheel.Publish(0xb0,1,0);done.store(true);
 });
 while(!done.load())wheel.Drain(send);
 producer.join();wheel.Drain(send);
 check(synth->PitchBend(0)==8192&&synth->ModWheel(0)==0,"concurrent final return lost");
 std::cout<<"Editor wheel coalescing, host precedence and concurrent delivery passed\n";
}
