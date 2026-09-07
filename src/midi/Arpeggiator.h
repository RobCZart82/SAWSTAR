// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace sawstar {
// Audio-thread owned. Timing is in beats; first held key starts the phrase.
class Arpeggiator {
 struct Key {bool down=false,latched=false;int velocity=0;uint64_t order=0;};
 struct Note {int pitch=0,channel=0,velocity=0,octave=0;uint64_t order=0;};
public:
 void Init(double sr){rate_=std::isfinite(sr)&&sr>=8000?sr:44100;keys_={};pedal_={};count_=physical_=0;active_=-1;on_=wasOn_=hold_=running_=false;mode_=0;division_=2;octaves_=1;phase_=0;step_=order_=0;seed_=0x793a15u;fresh_=true;}
 template<class Send> void Clear(Send send){
  for(int ch=0;ch<16;++ch){send(0xb0|ch,64,0);send(0xb0|ch,123,0);}
  keys_={};pedal_={};count_=physical_=0;active_=-1;phase_=0;step_=0;fresh_=true;
 }
 template<class Send> void Set(bool on,int mode,int division,float gate,int octaves,float swing,bool hold,double bpm,bool running,Send send){
  const int m=std::clamp(mode,0,4),o=std::clamp(octaves,1,4);
  if(on!=on_){
   Stop(send);for(int ch=0;ch<16;++ch){send(0xb0|ch,64,0);send(0xb0|ch,123,0);}
   for(auto& k:keys_)k.latched=false;pedal_={};on_=on;fresh_=true;step_=0;
   if(!on_)for(int i=0;i<2048;++i)if(keys_[i].down)send(0x90|(i/128),i%128,keys_[i].velocity);
  }
  const bool rebuild=m!=mode_||o!=octaves_||hold_!=hold;
  mode_=m;octaves_=o;division_=std::clamp(division,0,7);
  gate_=Safe(gate,5,100)*.01;swing_=Safe(swing,0,75)*.01;
  bpm_=std::isfinite(bpm)&&bpm>0?std::clamp(bpm,1.,1000.):120;
  if(hold_&&!hold)for(int i=0;i<2048;++i)if(!keys_[i].down&&!pedal_[i/128])keys_[i].latched=false;
  hold_=hold;
  if(on_&&running_&&!running)Clear(send);
  running_=running;
  if(rebuild||on_!=wasOn_){Rebuild();wasOn_=on_;}
  if(!count_)Stop(send);
 }
 template<class Send> void Midi(int status,int note,int value,Send send){
  if(note<0||note>127||value<0||value>127)return;
  const int ch=status&15,kind=status&240,index=ch*128+note;
  if(kind==0x90&&value>0){
   if(on_&&hold_&&physical_==0){for(auto& k:keys_)k.latched=false;fresh_=true;step_=0;}
   auto& k=keys_[index];if(!k.down)++physical_;k.down=true;k.latched=false;k.velocity=value;k.order=++order_;
   if(count_==0){fresh_=true;step_=0;}Rebuild();
  }else if(kind==0x80||(kind==0x90&&value==0)){
   auto& k=keys_[index];if(k.down){--physical_;k.down=false;k.latched=on_&&(hold_||pedal_[ch]);}Rebuild();
   if(on_&&!count_)Stop(send);
  }else if(kind==0xb0&&note==64){
   pedal_[ch]=value>=64;
   if(!pedal_[ch]&&!hold_)for(int i=ch*128;i<(ch+1)*128;++i)if(!keys_[i].down)keys_[i].latched=false;
   Rebuild();if(on_&&!count_)Stop(send);
   if(on_)return;
  }else if(kind==0xb0&&(note==120||note==123||note==121)){
   pedal_[ch]=false;
   for(int i=ch*128;i<(ch+1)*128;++i){auto& k=keys_[i];if(note!=121){if(k.down)--physical_;k=Key{};}else if(!k.down)k.latched=false;}
   Rebuild();if(on_&&active_>=0&&active_/128==ch)Stop(send);
   // Do not let the synth sustain arp steps after resetting controller state.
   if(on_)send(0xb0|ch,64,0);
  }
  if(!on_ || (kind!=0x90&&kind!=0x80))send(status,note,value);
 }
 template<class Send> void Process(Send send){
  if(!on_||!count_)return;
  if(fresh_||phase_+1.e-12>=duration_){
   if(fresh_)phase_=0;else phase_=std::max(0.,phase_-duration_);
   Stop(send);const int n=Choose();const auto& note=notes_[n];
   active_=note.channel*128+note.pitch;send(0x90|note.channel,note.pitch,note.velocity);
   static constexpr double beats[]={1,.5,.25,.125,1./3,1./6,.75,.375};
   duration_=beats[division_]*(1+(step_%2?-swing_:swing_));
   ++step_;fresh_=false;
  }
  if(active_>=0&&phase_+1.e-12>=duration_*gate_)Stop(send);
  phase_+=bpm_/(60*rate_);
 }
private:
 static float Safe(float v,float lo,float hi){return std::isfinite(v)?std::clamp(v,lo,hi):lo;}
 template<class Send> void Stop(Send send){if(active_>=0){send(0x80|(active_/128),active_%128,0);active_=-1;}}
 void Rebuild(){count_=0;for(int i=0;i<2048;++i)if(keys_[i].down||keys_[i].latched)for(int octave=0;octave<octaves_;++octave){int pitch=i%128+octave*12;if(pitch<=127)notes_[count_++]={pitch,i/128,keys_[i].velocity,octave,keys_[i].order};}
  std::sort(notes_.begin(),notes_.begin()+count_,[this](const Note& a,const Note& b){if(mode_==4){if(a.octave!=b.octave)return a.octave<b.octave;if(a.order!=b.order)return a.order<b.order;}if(a.pitch!=b.pitch)return a.pitch<b.pitch;return a.channel<b.channel;});
  if(!count_){fresh_=true;step_=0;}
 }
 int Choose(){if(mode_==3){seed_^=seed_<<13;seed_^=seed_>>17;seed_^=seed_<<5;return seed_%count_;}
  if(mode_==2&&count_>1){const auto p=step_%(2*count_-2);return p<uint64_t(count_)?int(p):2*count_-2-int(p);}
  const int p=step_%count_;return mode_==1?count_-1-p:p;
 }
 std::array<Key,2048> keys_{};std::array<Note,8192> notes_{};std::array<bool,16> pedal_{};
 int count_=0,physical_=0,active_=-1,mode_=0,division_=2,octaves_=1;
 bool on_=false,wasOn_=false,hold_=false,running_=false,fresh_=true;
 uint64_t step_=0,order_=0;uint32_t seed_=0x793a15u;
 double rate_=44100,bpm_=120,phase_=0,duration_=.25,gate_=.8,swing_=0;
};
}
