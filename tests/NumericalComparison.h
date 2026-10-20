// SPDX-License-Identifier: MIT
#pragma once
#include "dsp/SevenSaw.h"
#include <cfenv>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <string>
#if defined(__SSE__)
#include <xmmintrin.h>
#endif
#ifdef SAWSTAR_NUMERICAL_TRACE
#include "engine/NumericalTrace.h"
#endif
namespace numerical_comparison {
inline uint32_t Bits(float value){uint32_t bits;std::memcpy(&bits,&value,sizeof bits);return bits;}
inline bool Same(sawstar::StereoSample a,sawstar::StereoSample b){return Bits(a.left)==Bits(b.left)&&Bits(a.right)==Bits(b.right);}
struct Context {int waveform,filter,voice;double rate;bool fx;const char* phase;int frame;};
#ifdef SAWSTAR_NUMERICAL_TRACE
template<class Emit> size_t Differences(const sawstar::NumericalTrace& candidate,const sawstar::NumericalTrace& reference,Emit emit){
  size_t count=0;
  const auto field=[&](const std::string& name,uint32_t c,uint32_t r){if(c!=r){if(count<16)emit(name,c,r);++count;}};
  // Only create field names on an actual difference, not on every audio sample.
  for(size_t v=0;v<16;++v){
    if(candidate.notes[v]!=reference.notes[v])field("voice["+std::to_string(v)+"].note",uint32_t(candidate.notes[v]),uint32_t(reference.notes[v]));
    if(candidate.noiseState[v]!=reference.noiseState[v])field("voice["+std::to_string(v)+"].noise_state",candidate.noiseState[v],reference.noiseState[v]);
    for(size_t f=0;f<16;++f){const auto c=Bits(candidate.voices[v][f]),r=Bits(reference.voices[v][f]);
      if(c!=r)field("voice["+std::to_string(v)+"]."+sawstar::NumericalTrace::VoiceNames[f],c,r);}
  }
  for(size_t s=0;s<8;++s)for(size_t ch=0;ch<2;++ch){const auto c=Bits(candidate.stages[s][ch]),r=Bits(reference.stages[s][ch]);
    if(c!=r)field(std::string(sawstar::NumericalTrace::StageNames[s])+(ch?".right":".left"),c,r);}
  for(size_t f=0;f<6;++f){const auto c=Bits(candidate.controls[f]),r=Bits(reference.controls[f]);
    if(c!=r)field("control["+std::to_string(f)+"]",c,r);}
  return count;
}
#endif
inline void Check(sawstar::StereoSample candidate,sawstar::StereoSample reference,
                  sawstar::StereoSample candidatePre,sawstar::StereoSample referencePre,
                  const Context& at,std::ostream& log
#ifdef SAWSTAR_NUMERICAL_TRACE
                  ,const sawstar::NumericalTrace& candidateTrace,const sawstar::NumericalTrace& referenceTrace
#endif
                  ){
  const bool audioMismatch=!Same(candidate,reference)||!Same(candidatePre,referencePre)||
    !std::isfinite(candidate.left)||!std::isfinite(candidate.right)||
    !std::isfinite(reference.left)||!std::isfinite(reference.right)||
    !std::isfinite(candidatePre.left)||!std::isfinite(candidatePre.right)||
    !std::isfinite(referencePre.left)||!std::isfinite(referencePre.right);
#ifdef SAWSTAR_NUMERICAL_TRACE
  const auto traceCount=Differences(candidateTrace,referenceTrace,[](const std::string&,uint32_t,uint32_t){});
  if(!audioMismatch&&!traceCount)return;
#else
  if(!audioMismatch)return;
#endif
  log<<std::setprecision(17)<<"Numerical mismatch waveform="<<at.waveform<<" rate="<<at.rate
     <<" filter_mode="<<at.filter<<" voice_mode="<<at.voice<<" fx="<<at.fx
     <<" phase="<<at.phase<<" frame="<<at.frame<<" fp_rounding="<<std::fegetround();
#if defined(__SSE__)
  log<<" mxcsr=0x"<<std::hex<<_mm_getcsr()<<std::dec;
#endif
  log<<'\n';
  const auto pair=[&](const char* name,float c,float r){log<<name<<" candidate_bits=0x"<<std::hex<<Bits(c)
    <<" reference_bits=0x"<<Bits(r)<<std::dec<<" candidate="<<c<<" reference="<<r<<'\n';};
  // Always log both points before throwing, even when output differs first.
  pair("output.left",candidate.left,reference.left);pair("output.right",candidate.right,reference.right);
  pair("pre_fx.left",candidatePre.left,referencePre.left);pair("pre_fx.right",candidatePre.right,referencePre.right);
#ifdef SAWSTAR_NUMERICAL_TRACE
  Differences(candidateTrace,referenceTrace,[&](const std::string& name,uint32_t c,uint32_t r){
    log<<"trace "<<name<<" candidate_bits=0x"<<std::hex<<c<<" reference_bits=0x"<<r<<std::dec<<'\n';});
  log<<"trace_differences="<<traceCount<<" trace_lines_limit=16\n";
#endif
  throw std::runtime_error("Dispatch engine bit identity (see numerical diagnostics)");
}
}
