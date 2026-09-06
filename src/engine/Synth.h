// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/SevenSaw.h"
#include "dsp/Lfo.h"
#include "dsp/Effects/Chorus.h"
#include "dsp/LowPass.h"
#include "dsp/FilterModulation.h"
#include "Control/adsr.h"
#include <array>
#include <cstdint>
namespace sawstar {
// Audio-thread owned. Fixed storage: no allocation or locks while rendering.
class Synth {
public:
  void Reset(double sampleRate);
  void SetParameters(double gainDb, double attackMs, double decayMs, double sustain, double releaseMs);
  void SetWaveforms(int osc1,int osc2);
  void SetLfo(float hz,float depth,int shape,int target,bool sync,int division,double bpm,bool retrigger);
  void SetChorus(bool enabled,float mix,float hz,float depth){chorus_.Set(enabled,mix,hz,depth);}
  void SetOutputBoost(float dB);
  void SetMixer(float osc1, float osc2, float sub, float noise,
                int osc2Octave, int subOctave, int noiseType, int osc1Octave);
  void SetOsc2(float detune, float mix, float width);
  void Midi(int status, int data1, int data2);
  void SetFilterCharacter(float driveDb,int mode);
  void SetFilter(float cutoffHz, float resonancePercent, float mixPercent);
  void SetFilterEnvelope(float amount, float tracking, float attack, float decay, float sustain, float release);
  void SetPerformance(float bendRange,float modDepth);
  int PitchBend(int channel) const { return bend_[channel&15]; }
  int ModWheel(int channel) const { return mod_[channel&15]; }
  void SetSaw(float detuneCents, float mixPercent, float widthPercent);
  StereoSample ProcessStereo();
  float Process() { const auto s=ProcessStereo(); return (s.left+s.right)*0.5f; }
  bool Held(int note) const;
  int ActiveVoices() const;
private:
  struct Voice {
    SevenSaw osc, osc2;
    daisysp::Oscillator sub;
    float fundamental=440, darkNoise=0;
    uint32_t noiseState=1;
    LowPass filter;
    FilterModulation filterMod;
    daisysp::Adsr env;
    int note = -1, channel = 0;
    bool held = false, gate = false;
    float velocity = 0;
    uint64_t age = 0;
  };
  std::array<Voice, 16> voices_{};
  Lfo lfo_;
  Chorus chorus_;
  bool alternateWave_=false;
  std::array<bool, 16> sustain_{};
  std::array<int,16> bend_{{8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192,8192}}, mod_{};
  std::array<float,16> bendRatio_{{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}}, bendTarget_{{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}};
  float bendRange_=2,modDepth_=24;
  void UpdateBend(int channel);
  uint64_t age_ = 0;
  float cutoff_=12000, resonance_=0, filterMix_=0;
  float amount_=0, tracking_=0, filterAttack_=10, filterDecay_=200, filterSustain_=0, filterRelease_=250;
  std::array<float,4> levels_{{1,0,0,0}}, targetLevels_{{1,0,0,0}};
  int osc1Octave_=0, osc2Octave_=0, subOctave_=-1, noiseType_=0;
  float sampleRate_=44100, noisePole_=0;
  float boost_=1, targetBoost_=1, protection_=1, protectionRelease_=0;
  float gain_ = 0, targetGain_ = 0.25f, smoothing_ = 0.002f;
};
}
