// SPDX-License-Identifier: MIT
#include "visual/Scope.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include <limits>
static void check(bool ok){if(!ok){std::cerr<<"Scope check failed\n";std::exit(1);}}
int main(){
 auto scope=std::make_unique<sawstar::Scope>();auto frame=std::make_unique<sawstar::Scope::Frame>();
 for(double rate:{44100.,48000.,96000.,192000.}){
  scope->Reset(rate);while(scope->Latest(*frame)){}
  for(int i=0;i<int(rate*.12);++i){scope->Push(std::sin(float(i)*6.2831853f*110.f/float(rate)));if(i%1000==0)scope->Latest(*frame);}
  scope->Latest(*frame);unsigned n=0;unsigned start=sawstar::ScopeStart(*frame,n);
  check(frame->rate==rate&&n>=unsigned(rate*.031)&&start+n<=frame->count);
  check(frame->samples[start]>=0&&std::abs(frame->samples[start])<.1f);
 }
 scope->Reset(44100);while(scope->Latest(*frame)){}
 // A stalled/closed editor never blocks a producer, even with a full queue.
 for(int i=0;i<100000;++i)scope->Push(std::numeric_limits<float>::quiet_NaN());
 check(scope->Latest(*frame));for(unsigned i=0;i<frame->count;++i)check(frame->samples[i]==0);
 // New data resumes after draining. Consumer-owned frame data must remain coherent.
 std::atomic<bool> done{false};
 std::thread audio([&]{for(int i=0;i<500000;++i)scope->Push(.25f);done.store(true);});
 while(!done.load()){if(scope->Latest(*frame))for(unsigned i=0;i<frame->count;++i)check(frame->samples[i]==0||frame->samples[i]==.25f);}
 audio.join();scope->Latest(*frame);
 // Reset touches only producer-owned state, while the consumer may drain.
 scope->Reset(96000);for(int i=0;i<10000;++i){scope->Push(.5f);if(i%4000==0)scope->Latest(*frame);}scope->Latest(*frame);
 check(frame->rate==96000);for(unsigned i=0;i<frame->count;++i)check(frame->samples[i]==.5f);
 std::cout<<"Scope native capture, trigger, queue saturation, concurrency and reset passed\n";
}
