// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <cfenv>
#include <cstdint>
#include <ostream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#if defined(__SSE__) || defined(_M_X64)
#include <xmmintrin.h>
#endif
namespace fp_observation {
struct Snapshot {
  bool mxcsrAvailable=false;
  uint32_t raw=0;
  int cRounding=0;
  uint32_t Status() const{return raw&0x003fu;}
  uint32_t Controls() const{return raw&0xffc0u;}
};
inline Snapshot Capture(){
  Snapshot s;
#if defined(__SSE__) || defined(_M_X64)
  s.mxcsrAvailable=true;s.raw=_mm_getcsr();
#endif
  s.cRounding=std::fegetround();
  return s;
}
inline bool ControlDiff(Snapshot a,Snapshot b){return a.mxcsrAvailable!=b.mxcsrAvailable||
  (a.mxcsrAvailable&&a.Controls()!=b.Controls())||a.cRounding!=b.cRounding;}
inline bool StatusDiff(Snapshot a,Snapshot b){return a.mxcsrAvailable&&b.mxcsrAvailable&&a.Status()!=b.Status();}
inline void PrintSnapshot(std::ostream& out,const char* name,Snapshot s){
  out<<name<<" mxcsr_available="<<s.mxcsrAvailable;
  if(s.mxcsrAvailable)out<<" raw=0x"<<std::hex<<s.raw<<" status=0x"<<s.Status()<<" controls=0x"<<s.Controls()<<std::dec;
  else out<<" raw=NA status=NA controls=NA";
  out<<" c_rounding="<<s.cRounding<<'\n';
}
struct Frame {
  Snapshot referenceBefore,referenceAfter,candidateBefore,candidateAfter;
  const char* phase="main";int index=0;bool candidateFirst=false;
};
inline void PrintFrame(std::ostream& out,const Frame& f){
  out<<"fp_frame phase="<<f.phase<<" frame="<<f.index<<" order="<<(f.candidateFirst?"candidate-reference":"reference-candidate")<<'\n';
  if(f.candidateFirst){PrintSnapshot(out,"candidate_before",f.candidateBefore);PrintSnapshot(out,"candidate_after",f.candidateAfter);}
  PrintSnapshot(out,"reference_before",f.referenceBefore);PrintSnapshot(out,"reference_after",f.referenceAfter);
  if(!f.candidateFirst){PrintSnapshot(out,"candidate_before",f.candidateBefore);PrintSnapshot(out,"candidate_after",f.candidateAfter);}
}
struct Case {
  int waveform=0,filter=0,voice=0;double rate=0;bool fx=false,candidateFirst=false;
  std::array<Snapshot,7> boundaries{};
  std::array<uint64_t,3> frames{};
  uint64_t referenceStatus=0,candidateStatus=0,entryStatus=0;
  uint64_t referenceControls=0,candidateControls=0,entryControls=0,betweenCallsControls=0;
  uint64_t betweenFramesStatus=0,betweenFramesControls=0;
  Frame latest{};
  std::array<Frame,4> firstStatus{},firstControls{};
  unsigned statusStored=0,controlsStored=0;
  void Observe(const Frame& f,unsigned phase){
    const bool refStatus=StatusDiff(f.referenceBefore,f.referenceAfter),candStatus=StatusDiff(f.candidateBefore,f.candidateAfter);
    const bool entriesStatus=StatusDiff(f.referenceBefore,f.candidateBefore);
    const bool refControls=ControlDiff(f.referenceBefore,f.referenceAfter),candControls=ControlDiff(f.candidateBefore,f.candidateAfter);
    const bool entriesControls=ControlDiff(f.referenceBefore,f.candidateBefore);
    const bool gap=f.candidateFirst?ControlDiff(f.candidateAfter,f.referenceBefore):ControlDiff(f.referenceAfter,f.candidateBefore);
    const bool prior=frames[0]+frames[1]+frames[2]>0;
    const auto previous=latest.candidateFirst?latest.referenceAfter:latest.candidateAfter;
    const auto entering=f.candidateFirst?f.candidateBefore:f.referenceBefore;
    const bool outsideStatus=prior&&StatusDiff(previous,entering);
    const bool outsideControls=prior&&ControlDiff(previous,entering);
    referenceStatus+=refStatus;candidateStatus+=candStatus;entryStatus+=entriesStatus;
    referenceControls+=refControls;candidateControls+=candControls;entryControls+=entriesControls;betweenCallsControls+=gap;
    betweenFramesStatus+=outsideStatus;betweenFramesControls+=outsideControls;
    if((refStatus||candStatus||entriesStatus||outsideStatus)&&statusStored<firstStatus.size())firstStatus[statusStored++]=f;
    if((refControls||candControls||entriesControls||gap||outsideControls)&&controlsStored<firstControls.size())firstControls[controlsStored++]=f;
    latest=f;++frames.at(phase);
  }
};
inline void PrintCase(std::ostream& out,const Case& c){
  out<<"fp_case waveform="<<c.waveform<<" rate="<<c.rate<<" filter="<<c.filter<<" voice="<<c.voice<<" fx="<<c.fx
     <<" order="<<(c.candidateFirst?"candidate-reference":"reference-candidate")
     <<" main_frames="<<c.frames[0]<<" tail_frames="<<c.frames[1]<<" reset_frames="<<c.frames[2]<<'\n';
  const char* names[]={"case_begin","objects_created","reference_setup_done","candidate_setup_done",
    "before_reset","reference_reset_done","candidate_reset_done"};
  for(size_t i=0;i<c.boundaries.size();++i)PrintSnapshot(out,names[i],c.boundaries[i]);
  out<<"fp_counts reference_status="<<c.referenceStatus<<" candidate_status="<<c.candidateStatus<<" entry_status="<<c.entryStatus
     <<" reference_controls="<<c.referenceControls<<" candidate_controls="<<c.candidateControls<<" entry_controls="<<c.entryControls
     <<" between_calls_controls="<<c.betweenCallsControls<<" between_frames_status="<<c.betweenFramesStatus
     <<" between_frames_controls="<<c.betweenFramesControls<<'\n';
  out<<"fp_status_examples="<<c.statusStored<<" fp_control_examples="<<c.controlsStored<<" limit_each=4\n";
  for(unsigned i=0;i<c.statusStored;++i)PrintFrame(out,c.firstStatus[i]);
  for(unsigned i=0;i<c.controlsStored;++i)PrintFrame(out,c.firstControls[i]);
}
// Offline fixture only: fixed per-case samples, campaign storage reserved once.
// No printing, allocation or FP-mode writes occur in the measured render pair.
class Campaign {
  bool enabled_,printed_=false;std::vector<Case> completed_;
public:
  explicit Campaign(bool enabled):enabled_(enabled){if(enabled)completed_.reserve(480);}
  ~Campaign(){if(!printed_)try{Print(std::cerr,false);}catch(...){}}
  bool Enabled()const{return enabled_;}
  void Add(const Case& c){if(enabled_)completed_.push_back(c);}
  void Print(std::ostream& out,bool complete=true){
    if(!enabled_||printed_)return;
    printed_=true;
    out<<"fp_observation schema=1 passive=true ftz_daz_override=false state="<<(complete?"complete":"interrupted")<<" cases="<<completed_.size()<<'\n';
    for(const auto& c:completed_)PrintCase(out,c);
  }
};
inline void SelfTest(){
  const auto require=[](bool b){if(!b)throw std::runtime_error("FP observation self-test failed");};
  const Snapshot clean{true,0x1f80,FE_TONEAREST},sticky{true,0x1fb3,FE_TONEAREST},ftz{true,0x9f80,FE_TONEAREST};
  require(sticky.Status()==0x33&&sticky.Controls()==0x1f80&&StatusDiff(clean,sticky)&&!ControlDiff(clean,sticky));
  require(!StatusDiff(clean,ftz)&&ControlDiff(clean,ftz));
  Case c;Frame f{clean,sticky,sticky,sticky,"synthetic",197};c.Observe(f,0);
  require(c.referenceStatus==1&&c.candidateStatus==0&&c.referenceControls==0&&c.entryStatus==1);
  f={clean,ftz,ftz,ftz,"synthetic",198};c.Observe(f,0);
  require(c.referenceControls==1&&c.entryControls==1&&c.candidateControls==0);
  Case reverse;reverse.candidateFirst=true;
  reverse.Observe({sticky,sticky,clean,sticky,"synthetic",0,true},0);
  require(reverse.candidateStatus==1&&reverse.referenceStatus==0&&reverse.entryStatus==1&&reverse.betweenCallsControls==0);
  for(int i=0;i<20;++i)c.Observe(f,0);
  require(c.controlsStored==4&&c.statusStored<=4);
  std::ostringstream out;PrintCase(out,c);
  require(out.str().find("status=0x33 controls=0x1f80")!=std::string::npos);
  out.str("");PrintSnapshot(out,"unsupported",{false,0,FE_TONEAREST});
  require(out.str().find("raw=NA status=NA controls=NA")!=std::string::npos);
  const auto before=Capture(),after=Capture();
  require(before.mxcsrAvailable==after.mxcsrAvailable&&before.raw==after.raw&&before.cRounding==after.cRounding);
}
}
