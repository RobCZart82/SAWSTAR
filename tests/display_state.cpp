#ifdef NDEBUG
#undef NDEBUG
#endif
#include "visual/DisplayState.h"
#include <cassert>
#include <limits>
int main(){
 using namespace std::chrono;using sawstar::DisplayGate;
 auto t=DisplayGate::Clock::time_point{};
 sawstar::CpuDisplay cpu;assert(cpu.Update(10,t));
 for(int i=1;i<250;++i){assert(!cpu.Update(90,t+milliseconds(i)));assert(cpu.Value()==10);}
 assert(cpu.Update(50,t+milliseconds(250)));assert(cpu.Value()==20);
 assert(cpu.Update(std::numeric_limits<float>::quiet_NaN(),t+milliseconds(500)));assert(std::isfinite(cpu.Value()));
 DisplayGate scope;assert(scope.Due(t,milliseconds(90)));
 assert(!scope.Due(t+milliseconds(89),milliseconds(90)));
 assert(scope.Due(t+milliseconds(90),milliseconds(90)));
 assert(scope.Due(t+milliseconds(91),milliseconds(90),true)); // Reveal/stale transition.
 assert(scope.Due(t+seconds(20),milliseconds(90))); // Occlusion/idle gap must recover.
 assert(sawstar::ScopeDisplayPeak(.5f,1.f)==1.f);
 assert(sawstar::ScopeDisplayPeak(1.f,.1f)>.1f);
 assert(sawstar::ScopeDisplayPeak(0,0)>=.05f);
}
