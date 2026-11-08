// SPDX-License-Identifier: MIT
#pragma once
#include "engine/FilterStateTrace.h"
#include "NumericalComparison.h"
#include <memory>
#include <sstream>

namespace filter_state_comparison {
inline bool Same(const sawstar::FilterStateFrame& a,const sawstar::FilterStateFrame& b){
  for(size_t v=0;v<16;++v){
    const auto& x=a.voices[v];const auto& y=b.voices[v];
    if(x.size!=y.size)return false;
    for(size_t i=0;i<x.size;++i){
      const auto& f=x.fields[i];const auto& g=y.fields[i];
      if(f.bits!=g.bits||f.index!=g.index||f.kind!=g.kind||
         (f.name!=g.name&&std::strcmp(f.name,g.name)!=0))return false;
    }
  }
  return true;
}
class History {
  struct Entry {
    sawstar::FilterStateFrame reference,candidate;
    numerical_comparison::Context at{};
    fp_observation::Frame fp{};
  };
  // Heap allocation once per grid case, never per sample or inside the engines.
  std::unique_ptr<std::array<Entry,6>> entries_=std::make_unique<std::array<Entry,6>>();
  size_t next_=0,count_=0;
public:
  void Observe(const sawstar::FilterStateFrame& candidate,const sawstar::FilterStateFrame& reference,
               const numerical_comparison::Context& at,const fp_observation::Frame& fp,
               bool outerMismatch,std::ostream& log){
    auto& entry=(*entries_)[next_];entry.at=at;entry.fp=fp;
    entry.reference.CopyFrom(reference);entry.candidate.CopyFrom(candidate);
    next_=(next_+1)%6;count_=std::min(count_+1,size_t(6));
    if(!outerMismatch&&Same(candidate,reference))return;
    log<<"filter_state_first_divergence waveform="<<at.waveform<<" rate="<<at.rate
       <<" filter_mode="<<at.filter<<" voice_mode="<<at.voice<<" fx="<<at.fx
       <<" phase="<<at.phase<<" frame="<<at.frame<<" history_frames="<<count_
       <<" preceding_limit=5 localization_only=1\n";
    for(size_t i=0;i<count_;++i){
      const auto& e=(*entries_)[(next_+6-count_+i)%6];
      log<<"filter_history phase="<<e.at.phase<<" frame="<<e.at.frame<<'\n';
      fp_observation::PrintFrame(log,e.fp);
      for(size_t v=0;v<16;++v){
        const auto& c=e.candidate.voices[v];const auto& r=e.reference.voices[v];
        log<<"filter_voice="<<v<<" candidate_fields="<<c.size<<" reference_fields="<<r.size<<'\n';
        for(size_t f=0;f<std::max(c.size,r.size);++f){
          const auto print=[&](const char* role,const auto& snapshot){
            if(f>=snapshot.size){log<<role<<" missing\n";return;}
            const auto& field=snapshot.fields[f];
            log<<role<<" voice="<<v<<' '<<field.name<<'['<<field.index<<"] kind="<<field.kind
               <<" bits=0x"<<std::hex<<std::setw(field.kind=='f'?8:16)<<std::setfill('0')
               <<field.bits<<std::dec<<std::setfill(' ')<<'\n';
          };
          print("candidate",c);print("reference",r);
        }
      }
    }
    // The caller then prints the ordinary output/pre-FX/voice diagnostics too.
    throw std::runtime_error("Named filter state first divergence (localization, not root-cause proof)");
  }
};
inline void SelfTest(){
  const auto require=[](bool ok){if(!ok)throw std::runtime_error("Filter state history self-test failed");};
  auto r=std::make_unique<sawstar::FilterStateFrame>();auto c=std::make_unique<sawstar::FilterStateFrame>();
  History history;std::ostringstream log;fp_observation::Frame fp{};
  r->voices[0].Add("injected.accumulator",0,0.);
  for(int n=0;n<8;++n){c->CopyFrom(*r);history.Observe(*c,*r,{3,1,0,48000,true,"main",n},fp,false,log);}
  require(log.str().empty());
  c->voices[0].Clear();c->voices[0].Add("injected.accumulator",0,-0.);
  bool rejected=false;
  try{history.Observe(*c,*r,{3,1,0,48000,true,"main",8},fp,false,log);}catch(const std::runtime_error&){rejected=true;}
  const auto text=log.str();
  require(rejected&&text.find("history_frames=6")!=std::string::npos&&
    text.find("filter_history phase=main frame=3\n")!=std::string::npos&&
    text.find("filter_history phase=main frame=2\n")==std::string::npos&&
    text.find("bits=0x8000000000000000")!=std::string::npos);
  // Field schema and count matter independently of bits. Inactive voices clear.
  c->CopyFrom(*r);c->voices[0].fields[0].name="different.semantic.field";require(!Same(*c,*r));
  c->CopyFrom(*r);c->voices[0].fields[0].index=1;require(!Same(*c,*r));
  c->Clear();require(!Same(*c,*r));r->Clear();require(Same(*c,*r));
  sawstar::NamedStateTrace<1> bounded;bounded.Add("a",0,1);rejected=false;
  try{bounded.Add("b",0,2);}catch(const std::runtime_error&){rejected=true;}require(rejected);
}
}
