// SPDX-License-Identifier: MIT
#include "engine/Synth.h"
#include <algorithm>
#include <cmath>
namespace sawstar {
void Synth::Reset(double rate) {
  const float sr = static_cast<float>(std::isfinite(rate) && rate >= 8000 ? rate : 44100);
  bend_.fill(8192);mod_.fill(0);bendRatio_.fill(1);bendTarget_.fill(1);
  sustain_.fill(false); age_ = 0; gain_ = 0;
  monoKeys_.fill(MonoKey{});monoKey_=-1;monoPitchValid_=false;glideRemaining_=monoOrder_=0;voiceMode_=0;
  boost_=targetBoost_; protection_=1;
  protectionRelease_=1.f-std::exp(-1.f/(0.08f*sr));
  smoothing_ = 1.f - std::exp(-1.f / (0.005f * sr));
  lfo_.Init(sr);chorus_.Init(sr);delay_.Init(sr);reverb_.Init(sr);alternateWave_=false;
  noiseColor_=targetNoiseColor_=0;
  colorPole_=1.f-std::exp(-2.f*3.14159265358979323846f*1000.f/sr);
  sampleRate_=sr; noisePole_=1.f-std::exp(-2.f*3.14159265358979323846f*1200.f/sr);
  levels_=targetLevels_;
  uint32_t seed=0x9e3779b9u;
  for (auto& v : voices_) {
    v = Voice{}; v.noiseState=seed; v.pink.Reset(seed^0xa341316cu); seed+=0x9e3779b9u;
    v.osc2.Init(sr); v.sub.Init(sr); v.sub.SetWaveform(daisysp::Oscillator::WAVE_SIN); v.sub.SetAmp(1);
    v.osc.Init(sr); v.env.Init(sr); v.filter.Init(sr); v.filterMod.Init(sr);
  }
}
void Synth::SetVoiceMode(int mode,float glideMs,bool overlapOnly) {
  mode=std::clamp(mode,0,2);glideMs_=std::isfinite(glideMs)?std::clamp(glideMs,0.f,2000.f):0;
  overlapOnly_=overlapOnly;
  if(mode!=voiceMode_){
    if(voiceMode_!=0&&monoPitchValid_&&voices_[0].note>=0){auto& v=voices_[0];v.fundamental=static_cast<float>(440*std::exp2((monoPitch_-69)/12.));v.osc.SetFreq(v.fundamental);v.osc2.SetFreq(v.fundamental);}
    // A mode change starts a new key phrase; release current sources safely.
    for(auto& v:voices_){if(v.note>=0&&(v.gate||v.gatePending)){v.env.Process(true);v.filterMod.Process(v.note,true);}v.held=v.gate=v.gatePending=false;}
    monoKeys_.fill(MonoKey{});monoKey_=-1;monoPitchValid_=false;glideRemaining_=0;
    voiceMode_=mode;
  }
  if(glideMs_==0){monoPitch_=monoTarget_;glideRemaining_=0;}
}
void Synth::SelectMono(bool retrigger,bool allowGlide) {
  int selected=-1;
  // Physically held keys take priority over pedal-latched keys, then last-note priority.
  for(int i=0;i<2048;++i){const auto& k=monoKeys_[i];if(!k.held&&!k.latched)continue;
    if(selected<0||(k.held&&!monoKeys_[selected].held)||(k.held==monoKeys_[selected].held&&k.order>monoKeys_[selected].order))selected=i;
  }
  auto& v=voices_[0];
  if(selected<0){monoKey_=-1;v.held=v.gate=false;return;}
  const bool same=selected==monoKey_;if(same&&!retrigger){v.held=monoKeys_[selected].held;return;}
  retrigger=retrigger||voiceMode_==1;
  const int note=selected%128,ch=selected/128;
  const bool wasRunning=v.note>=0;
  const bool slide=monoPitchValid_&&allowGlide&&glideMs_>0;
  monoTarget_=note;
  if(slide){glideRemaining_=std::max<uint64_t>(1,static_cast<uint64_t>(sampleRate_*glideMs_*.001));monoStep_=(monoTarget_-monoPitch_)/glideRemaining_;}
  else{monoPitch_=monoTarget_;glideRemaining_=0;}
  if(!monoPitchValid_)monoVelocity_=monoKeys_[selected].velocity/127.f;
  monoPitchValid_=true;monoKey_=selected;
  // Mono uses a fixed tuning reference; the sample-accurate pitch ratio carries glide.
  if(!wasRunning){v.filter.Clear();monoVelocity_=monoKeys_[selected].velocity/127.f;}
  v.note=note;v.channel=ch;v.held=monoKeys_[selected].held;v.gate=true;
  v.velocity=monoKeys_[selected].velocity/127.f;v.fundamental=440;v.osc.SetFreq(440);v.osc2.SetFreq(440);
  if(retrigger||!wasRunning){v.env.Retrigger(false);v.filterMod.Trigger(!wasRunning);v.gatePending=true;}
}
void Synth::MonoMidi(int status,int note,int value) {
  const int channel=status&15,kind=status&240,index=channel*128+note;
  if(kind==0x90&&value>0){
    bool overlap=false;for(const auto& k:monoKeys_)overlap|=k.held;
    if(!overlap)lfo_.Trigger();
    auto& key=monoKeys_[index];key.held=true;key.latched=false;key.velocity=value;key.order=++monoOrder_;
    // Any release-only poly voices from a mode change stop before the mono note starts.
    for(size_t i=1;i<voices_.size();++i){voices_[i].note=-1;voices_[i].held=voices_[i].gate=false;}
    SelectMono(voiceMode_==1||!overlap,!overlapOnly_||overlap);
  }else if(kind==0x80||(kind==0x90&&value==0)){
    auto& key=monoKeys_[index];if(!key.held)return;key.held=false;key.latched=sustain_[channel];
    SelectMono(false,true);
  }else if(kind==0xb0){
    if(note==121){bend_[channel]=8192;mod_[channel]=0;UpdateBend(channel);}
    if(note==64||note==121){sustain_[channel]=note==64&&value>=64;
      if(!sustain_[channel])for(int i=channel*128;i<(channel+1)*128;++i)monoKeys_[i].latched=false;
      SelectMono(false,true);
    }else if(note==120||note==123){
      for(int i=channel*128;i<(channel+1)*128;++i)monoKeys_[i]=MonoKey{};
      sustain_[channel]=false;
      if(note==120)for(size_t i=1;i<voices_.size();++i)if(voices_[i].channel==channel){voices_[i].note=-1;voices_[i].gate=voices_[i].held=false;}
      if(note==120&&voices_[0].channel==channel){voices_[0].note=-1;voices_[0].gate=voices_[0].held=false;voices_[0].env.Retrigger(true);monoKey_=-1;}
      SelectMono(false,true);
    }
  }
}
void Synth::SetWaveforms(int osc1,int osc2){alternateWave_=osc1!=0;for(auto& v:voices_){v.osc.SetWaveform(osc1);v.osc2.SetWaveform(osc2);}}
void Synth::SetLfo(float hz,float depth,int shape,int target,bool sync,int division,double bpm,bool retrigger){lfo_.Set(hz,depth,shape,target,sync,division,bpm,retrigger);}
void Synth::SetMixer(float osc1,float osc2,float sub,float noise,
                     int osc2Octave,int subOctave,int noiseType,int osc1Octave) {
  const float values[]={osc1,osc2,sub,noise};
  for(size_t i=0;i<4;++i)targetLevels_[i]=std::isfinite(values[i])?std::clamp(values[i]*.01f,0.f,1.f):0;
  osc1Octave_=std::clamp(osc1Octave,-2,2); osc2Octave_=std::clamp(osc2Octave,-2,2);
  subOctave_=std::clamp(subOctave,-2,0); noiseType_=std::clamp(noiseType,0,2);
}
void Synth::SetOsc2(float detune,float mix,float width) {
  for(auto& v:voices_)v.osc2.SetShape(detune,mix*.01f,width*.01f);
}
void Synth::SetOutputBoost(float dB) {
  dB=std::isfinite(dB)?std::clamp(dB,0.f,24.f):0.f;
  targetBoost_=std::pow(10.f,dB/20.f);
}
void Synth::SetParameters(double gain, double attack, double decay, double sustain, double release) {
  targetGain_ = static_cast<float>(std::pow(10., gain / 20.));
  for (auto& v : voices_) {
    v.env.SetAttackTime(static_cast<float>(attack * .001));
    v.env.SetDecayTime(static_cast<float>(decay * .001));
    v.env.SetSustainLevel(static_cast<float>(sustain));
    v.env.SetReleaseTime(static_cast<float>(release * .001));
  }
}
void Synth::SetFilterCharacter(float driveDb,int mode) {
  for(auto& v:voices_)v.filter.SetCharacter(driveDb,mode);
}
void Synth::SetFilter(float cutoff,float resonance,float mix) {
  cutoff_=cutoff; resonance_=resonance; filterMix_=mix;
  SetFilterEnvelope(amount_,tracking_,filterAttack_,filterDecay_,filterSustain_,filterRelease_);
}
void Synth::SetFilterEnvelope(float amount,float tracking,float attack,float decay,float sustain,float release) {
  amount_=amount; tracking_=tracking; filterAttack_=attack; filterDecay_=decay;
  filterSustain_=sustain; filterRelease_=release;
  for(auto& v:voices_) v.filterMod.Set(cutoff_,amount,tracking,attack,decay,sustain,release);
}
void Synth::UpdateBend(int ch) {
  const float norm=(bend_[ch]-8192)/(bend_[ch]>=8192?8191.f:8192.f);
  bendTarget_[ch]=std::exp2(norm*bendRange_/12.f);
}
void Synth::SetPerformance(float range,float depth) {
  range=std::isfinite(range)?std::clamp(range,0.f,24.f):2;
  modDepth_=std::isfinite(depth)?std::clamp(depth,0.f,48.f):24;
  if(range!=bendRange_){bendRange_=range;for(int ch=0;ch<16;++ch)UpdateBend(ch);}
}
void Synth::SetSaw(float detune,float mix,float width) {
  for(auto& v:voices_) v.osc.SetShape(detune,mix*.01f,width*.01f);
}
void Synth::Midi(int status, int note, int value) {
  const int channel = status & 15, kind = status & 240;
  if (note < 0 || note > 127 || value < 0 || value > 127) return;
  if(voiceMode_!=0&&(kind==0x90||kind==0x80||(kind==0xb0&&(note==64||note==120||note==121||note==123)))){MonoMidi(status,note,value);return;}
  if(kind==0xe0){bend_[channel]=note+(value<<7);UpdateBend(channel);return;}
  if (kind == 0x90 && value > 0) {
    bool held=false;for(const auto& v:voices_)held|=v.held;
    if(!held)lfo_.Trigger();
    // Repeated note retriggers one voice; idle, then oldest released, then oldest held.
    Voice* chosen = nullptr;
    for (auto& v : voices_) if (v.note == note && v.channel == channel) { chosen = &v; break; }
    if (!chosen) for (auto& v : voices_) if (v.note < 0) { chosen = &v; break; }
    if (!chosen) for (auto& v : voices_) if (!v.held && (!chosen || v.age < chosen->age)) chosen = &v;
    if (!chosen) chosen = &*std::min_element(voices_.begin(), voices_.end(), [](const Voice& a, const Voice& b) { return a.age < b.age; });
    auto& v = *chosen;
    v.filterMod.Trigger(v.note<0);
    if(v.note<0) v.filter.Clear();
    v.note = note; v.channel = channel; v.held = v.gate = true;
    v.velocity = static_cast<float>(value) / 127.f; v.age = ++age_;
    v.fundamental=static_cast<float>(440. * std::pow(2., (note - 69) / 12.));
    v.osc.SetFreq(v.fundamental); v.osc2.SetFreq(v.fundamental);
    v.env.Retrigger(false);v.gatePending=true;
  } else if (kind == 0x80 || (kind == 0x90 && value == 0)) {
    for (auto& v : voices_) if (v.note == note && v.channel == channel) {
      v.held = false; v.gate = sustain_[channel];
    }
  } else if (kind == 0xB0) {
    if(note==1){mod_[channel]=value;return;}
    if(note==121){bend_[channel]=8192;mod_[channel]=0;UpdateBend(channel);}
    if (note == 64 || note == 121) {
      sustain_[channel] = note == 64 && value >= 64;
      if (!sustain_[channel]) for (auto& v : voices_) if (v.channel == channel && !v.held) v.gate = false;
    } else if (note == 120 || note == 123) {
      for (auto& v : voices_) if (v.channel == channel) {
        v.held = v.gate = false;
        if (note == 120) { v.note = -1; v.env.Retrigger(true); }
      }
      sustain_[channel] = false;
    }
  }
}
StereoSample Synth::ProcessStereo() {
  noiseColor_+=smoothing_*(targetNoiseColor_-noiseColor_);
  StereoSample sum;
  const auto lfo=lfo_.Process();
  const float vibrato=lfo.pitch==0?1.f:std::exp2(lfo.pitch/12.f);
  for(size_t i=0;i<4;++i)levels_[i]+=smoothing_*(targetLevels_[i]-levels_[i]);
  for(int ch=0;ch<16;++ch)bendRatio_[ch]+=smoothing_*(bendTarget_[ch]-bendRatio_[ch]);
  if(voiceMode_!=0&&glideRemaining_){monoPitch_+=monoStep_;if(--glideRemaining_==0)monoPitch_=monoTarget_;}
  const float monoRatio=voiceMode_!=0?static_cast<float>(std::exp2((monoPitch_-69)/12.)):1;
  for (auto& v : voices_) if (v.note >= 0) {
    const float glide=(voiceMode_!=0&& &v==&voices_[0]&&monoPitchValid_)?monoRatio:1.f;
    // DaisySP detects release from a gate edge. Preserve a zero-length MIDI
    // note's edge when note-on and note-off arrive before its first sample.
    if(v.gatePending){
      if(!v.gate){v.env.Process(true);v.filterMod.Process(v.note,true);}
      v.gatePending=false;
    }
    const float env = v.env.Process(v.gate);
    v.filter.Set(v.filterMod.Process(v.note,v.gate,mod_[v.channel]/127.f*modDepth_+lfo.cutoff),resonance_,filterMix_);
    v.osc.SetPitchMultiplier(bendRatio_[v.channel]*vibrato*glide*std::exp2(static_cast<float>(osc1Octave_)));
    v.osc2.SetPitchMultiplier(bendRatio_[v.channel]*vibrato*glide*std::exp2(static_cast<float>(osc2Octave_)));
    v.sub.SetFreq(std::min(v.fundamental*bendRatio_[v.channel]*vibrato*glide*std::exp2(static_cast<float>(subOctave_)),sampleRate_*.45f));
    const auto one=v.osc.Process(),two=v.osc2.Process();
    const float sub=v.sub.Process();
    // Per-voice deterministic xorshift; no global RNG, allocation or shared lock.
    auto& random=v.noiseState; random^=random<<13;random^=random>>17;random^=random<<5;
    const float white=static_cast<float>(random>>8)*(2.f/16777216.f)-1.f;
    v.darkNoise+=noisePole_*(white-v.darkNoise);
    const float pink=v.pink.Process();
    const float source=noiseType_==2?pink:(noiseType_==1?v.darkNoise:white);
    v.noiseLow+=colorPole_*(source-v.noiseLow);
    const float colored=noiseColor_<0?v.noiseLow:source-v.noiseLow;
    const float noise=source+std::abs(noiseColor_)*(colored-source);
    const float center=sub*levels_[2]+noise*levels_[3];
    const StereoSample mixed{one.left*levels_[0]+two.left*levels_[1]+center,
                             one.right*levels_[0]+two.right*levels_[1]+center};
    const auto value=v.filter.Process(mixed);
    float velocity=v.velocity;
    if(voiceMode_!=0&& &v==&voices_[0]&&monoPitchValid_){monoVelocity_+=smoothing_*(v.velocity-monoVelocity_);velocity=monoVelocity_;}
    sum.left+=value.left*env*velocity;sum.right+=value.right*env*velocity;
    if (!v.gate && !v.env.IsRunning()) v.note = -1;
  }
  gain_ += smoothing_ * (targetGain_ - gain_);
  boost_ += smoothing_ * (targetBoost_ - boost_);
  sum.left*=lfo.amp*std::sqrt(1-lfo.pan);sum.right*=lfo.amp*std::sqrt(1+lfo.pan);
  sum=reverb_.Process(delay_.Process(chorus_.Process(sum)));
  const float scale=gain_*boost_/16.f;
  sum.left*=scale; sum.right*=scale;
  // Stereo-linked peak guard: instant attack, 80 ms recovery, zero latency.
  // At settled 0 dB boost use the historical path for old project recall.
  if(reverb_.IsDry() && delay_.IsDry() && chorus_.IsDry() && !alternateWave_ && lfo.pan==0 && targetBoost_==1.f && std::abs(boost_-1.f)<0.00001f &&
     levels_[1]<1.e-6f && levels_[2]<1.e-6f && levels_[3]<1.e-6f) {
    protection_=1;
  } else {
    const float peak=std::max(std::abs(sum.left),std::abs(sum.right));
    const float required=peak>0.98f ? 0.98f/peak : 1.f;
    protection_+=protectionRelease_*(1.f-protection_);
    protection_=std::min(protection_,required);
  }
  return {std::clamp(sum.left*protection_, -1.f, 1.f),
          std::clamp(sum.right*protection_, -1.f, 1.f)};
}
bool Synth::Held(int note) const {
  if(voiceMode_!=0){if(note<0||note>127)return false;for(int ch=0;ch<16;++ch)if(monoKeys_[ch*128+note].held)return true;return false;}
  for (const auto& v : voices_) if (v.note == note && v.held) return true;
  return false;
}
int Synth::ActiveVoices() const {
  int count = 0; for (const auto& v : voices_) if (v.note >= 0) ++count; return count;
}
}
