// SPDX-License-Identifier: MIT
#include "dsp/SubOscillator.h"
#include <array>
#include <cstdlib>
#include <iostream>
#include <limits>

void check(bool ok,const char* what){if(!ok){std::cerr<<what<<'\n';std::exit(1);}}
int main() {
  using sawstar::SubOscillator;
  for(float sr:{44100.f,48000.f,96000.f,192000.f,384000.f}) {
    for(int wave=0;wave<3;++wave) {
      SubOscillator osc;osc.Init(sr);osc.SetWaveform(wave);
      // Retune a running oscillator: cover ordinary bass, the former 20 kHz
      // boundary, a bent MIDI 127 and requests above the Nyquist safety limit.
      for(float hz:{110.f,19000.f,21000.f,25087.708f,sr*.449f,sr*.49f,std::numeric_limits<float>::max()}) {
        osc.SetFreq(hz);
        int crossings=0;float last=osc.Process();
        for(int i=0;i<int(sr);++i) {
          const float sample=osc.Process();
          check(std::isfinite(sample)&&std::abs(sample)<=1.01f,"SUB frequency sweep must stay bounded");
          if(last<=0&&sample>0)++crossings;
          last=sample;
        }
        const double expected=std::min(double(hz),double(sr)*.45);
        if(std::abs(crossings-expected)>2) {
          std::cerr<<"SUB frequency mismatch: rate="<<sr<<" wave="<<wave
                   <<" requested="<<hz<<" expected="<<expected<<" measured="<<crossings<<'\n';
          return 1;
        }
      }
      // Invalid or negative frequency freezes phase without poisoning it;
      // a square at zero frequency need not have a zero sample value.
      for(float hz:{-100.f,0.f,std::numeric_limits<float>::quiet_NaN(),
                    std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()}) {
        osc.SetFreq(hz);const float first=osc.Process();
        for(int i=0;i<32;++i)check(osc.Process()==first&&std::isfinite(first),"invalid SUB frequency must freeze phase safely");
      }
      osc.SetFreq(110);double energy=0;
      for(int i=0;i<4096;++i){const float x=osc.Process();energy+=x*x;}
      check(energy>1,"SUB must recover after invalid frequencies");
    }
  }
  for(float sr:{44100.f,48000.f,96000.f,192000.f}) {
    std::array<SubOscillator,3> reference;
    for(int i=0;i<3;++i){reference[i].Init(sr);reference[i].SetFreq(110);reference[i].SetWaveform(i);}
    const int fade=int(sr*.01);
    for(int phase=0;phase<32;++phase) {
      for(auto& osc:reference)for(int i=0;i<37;++i)osc.Process();
      for(int from=0;from<3;++from)for(int to=0;to<3;++to)if(from!=to) {
        auto changed=reference[from],unchanged=reference[from],target=reference[to];
        changed.SetWaveform(to);
        check(std::abs(changed.Process()-unchanged.Process())<=2.f/fade+1.e-6f,"wave switch introduces a discontinuity");
        target.Process();
        // Sending the same parameter on every block/sample must still settle.
        for(int i=1;i<fade;++i){changed.SetWaveform(to);changed.Process();target.Process();}
        for(int i=0;i<32;++i)check(std::abs(changed.Process()-target.Process())<1.e-6f,"fade must reach the new waveform with continuous phase");
      }
    }
    auto rapid=reference[0];
    for(int i=0;i<fade*4;++i) {
      auto before=rapid;
      if(i%17==0)rapid.SetWaveform((i/17)%3);
      const float sample=rapid.Process();
      check(std::isfinite(sample)&&std::abs(sample)<=1.01f,"rapid changes must stay bounded");
      check(std::abs(sample-before.Process())<=4.f/fade+1.e-6f,"retargeting a fade must preserve its current mixture");
    }
  }
  std::cout<<"SUB transitions: phase continuity, settling and rapid retargeting passed\n";
}
