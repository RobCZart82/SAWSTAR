// SPDX-License-Identifier: MIT
#include "dsp/SevenSaw.h"
#include <algorithm>
#include <cmath>
namespace sawstar {
namespace {
// Symmetric cents offsets: inner, middle, outer pairs. This is not a hardware emulation.
constexpr std::array<float,7> offsets{{0,-0.19f,0.19f,-0.53f,0.53f,-1,1}};
constexpr std::array<float,7> pans{{0,-0.30f,0.30f,0.65f,-0.65f,-1,1}};
constexpr std::array<float,7> phases{{0,0.17f,0.73f,0.31f,0.91f,0.53f,0.09f}};
float Safe(float value,float lo,float hi) { return std::isfinite(value)?std::clamp(value,lo,hi):lo; }
}
void SevenSaw::Init(float rate) {
  rate_=std::isfinite(rate)&&rate>=8000?rate:44100;
  slew_=1-std::exp(-1.f/(0.01f*rate_));
  pitch_=1;waveform_=0;waveWeights_={{1,0,0,0}};
  ratios_.fill(1);targets_.fill(1);detune_=-1;
  mix_=width_=targetMix_=targetWidth_=0;hz_=100;
  for(size_t i=0;i<saws_.size();++i) {
    // DaisySP Init leaves the triangle integrator history untouched. Zero it explicitly.
    saws_[i]=daisysp::Oscillator{};
    saws_[i].Init(rate_);saws_[i].SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_SAW);
    saws_[i].SetAmp(1);saws_[i].Reset(phases[i]);
    const int kinds[]={daisysp::Oscillator::WAVE_POLYBLEP_SQUARE,daisysp::Oscillator::WAVE_POLYBLEP_TRI,daisysp::Oscillator::WAVE_SIN};
    for(int w=0;w<3;++w){auto& o=alternatives_[w][i];o=daisysp::Oscillator{};o.Init(rate_);o.SetWaveform(kinds[w]);o.SetAmp(1);o.Reset(phases[i]);}
  }
}
void SevenSaw::SetFreq(float hz) {
  // Retain the MIDI fundamental so a downward bend can bring high notes back
  // below Nyquist. Clamp each oscillator only after applying bend and detune.
  hz_=Safe(hz,0,20000);
  ratios_=targets_; // MIDI pitch changes immediately; only detune automation is slewed.
  for(size_t i=0;i<saws_.size();++i) saws_[i].SetFreq(std::min(hz_*pitch_*ratios_[i],rate_*0.45f));
}
void SevenSaw::SetWaveform(int waveform){waveform_=std::clamp(waveform,0,3);}
void SevenSaw::SetShape(float cents,float mix,float width) {
  cents=Safe(cents,0,50);targetMix_=Safe(mix,0,1);targetWidth_=Safe(width,0,1);
  if(cents!=detune_) {
    detune_=cents;
    for(size_t i=0;i<saws_.size();++i) targets_[i]=std::exp2(offsets[i]*cents/1200.f);
  }
}
StereoSample SevenSaw::Process() {
  mix_+=slew_*(targetMix_-mix_);width_+=slew_*(targetWidth_-width_);
  for(int w=0;w<4;++w){waveWeights_[w]+=slew_*((w==waveform_?1.f:0.f)-waveWeights_[w]);
    if(w!=waveform_&&waveWeights_[w]<1.e-8f)waveWeights_[w]=0;}
  StereoSample result;
  for(size_t i=0;i<saws_.size();++i) {
    ratios_[i]+=slew_*(targets_[i]-ratios_[i]);
    saws_[i].SetFreq(std::min(hz_*pitch_*ratios_[i],rate_*0.45f));
    float sample=waveWeights_[0]>0?saws_[i].Process()*waveWeights_[0]:0;
    for(int w=1;w<4;++w)if(waveWeights_[w]>0){auto& o=alternatives_[w-1][i];
      o.SetFreq(std::min(hz_*pitch_*ratios_[i],rate_*.45f));sample+=o.Process()*waveWeights_[w];}
    const float value=sample*(i==0?1:mix_);
    const float pan=pans[i]*width_;
    result.left+=value*(1-pan);result.right+=value*(1+pan);
  }
  // Symmetric pan pairs make each channel's absolute weight sum 1+6*mix.
  const float norm=1/(1+6*mix_);result.left*=norm;result.right*=norm;
  return result;
}
}
