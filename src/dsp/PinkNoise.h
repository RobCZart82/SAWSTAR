// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <cstdint>
namespace sawstar {
// Octave-rate random sample-and-hold rows plus a full-rate white component.
// Original implementation; fixed storage and constant work per sample.
class PinkNoise {
public:
 void Reset(uint32_t seed) { state_=seed?seed:1; counter_=0; sum_=0;
   for(auto& row:rows_){row=White();sum_+=row;}
 }
 float Process() {
   if(++counter_==0) counter_=1;
   uint32_t bits=counter_; unsigned row=0;
   while((bits&1u)==0u && row<15){bits>>=1;++row;}
   const float next=White();sum_+=double(next)-rows_[row];rows_[row]=next;
   return static_cast<float>((sum_+White())*.125);
 }
private:
 float White(){state_^=state_<<13;state_^=state_>>17;state_^=state_<<5;
   return float(state_>>8)*(2.f/16777216.f)-1.f;}
 std::array<float,16> rows_{};
 uint32_t state_=1,counter_=0;double sum_=0;
};
}
