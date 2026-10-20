// SPDX-License-Identifier: MIT
// A new offline executable: setup FP observation, never a production fix.
#include "PremiumNativeCandidateSynth.h"
#include "PremiumPreviewSynth.h"
#include "DispatchEngineSetup.h"
#include "FpObservation.h"
#include <array>
#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

using Reference=sawstar::premium_preview_engine::Synth;
using Candidate=sawstar::premium_native_candidate_engine::Synth;
using fp_observation::Snapshot;
static void Require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static bool Same(sawstar::StereoSample a,sawstar::StereoSample b){
  return std::memcmp(&a.left,&b.left,sizeof(float))==0&&
         std::memcmp(&a.right,&b.right,sizeof(float))==0;
}
struct Operation {const char* name=nullptr;Snapshot before{},after{};};
struct SetupTrace {
  bool enabled=false;unsigned calls=0,stored=0;
  std::array<Operation,64> operations{};
  template<class Function> void Apply(const char* name,Function&& function){
    Require(calls<operations.size(),"Setup operation capacity exceeded");
    ++calls;
    Snapshot before;
    if(enabled)before=fp_observation::Capture();
    function();
    if(enabled)operations[stored++]={name,before,fp_observation::Capture()};
  }
  void Print(std::ostream& out,const char* engine)const{
    for(unsigned i=0;i<stored;++i){
      out<<"setup_operation engine="<<engine<<" index="<<i<<" name="<<operations[i].name<<'\n';
      fp_observation::PrintSnapshot(out,"before",operations[i].before);
      fp_observation::PrintSnapshot(out,"after",operations[i].after);
    }
  }
};
// Forward unchanged argument types to the same real engine API. Only the
// optional snapshots and fixed-capacity log surround each completed call.
template<class Engine> struct ObservedSetup {
  Engine& engine;SetupTrace& trace;
#define FORWARD_SETUP(Method) \
  template<class... Args> void Method(Args&&... args){ \
    trace.Apply(#Method,[&]{engine.Method(std::forward<Args>(args)...);}); \
  }
  FORWARD_SETUP(Reset)
  FORWARD_SETUP(SetParameters)
  FORWARD_SETUP(SetOutputBoost)
  FORWARD_SETUP(SetSaw)
  FORWARD_SETUP(SetOsc2)
  FORWARD_SETUP(SetMixer)
  FORWARD_SETUP(SetFilter)
  FORWARD_SETUP(SetFilterCharacter)
  FORWARD_SETUP(SetFilterEnvelope)
  FORWARD_SETUP(SetVoiceMode)
  FORWARD_SETUP(SetPerformance)
  FORWARD_SETUP(SetLfo)
  FORWARD_SETUP(SetLfo2)
  FORWARD_SETUP(SetModulation)
  FORWARD_SETUP(SetChorus)
  FORWARD_SETUP(SetDelay)
  FORWARD_SETUP(SetReverb)
  FORWARD_SETUP(SetWaveforms)
  FORWARD_SETUP(Midi)
#undef FORWARD_SETUP
};
static void SelfTest(){
  fp_observation::SelfTest();
  Require(!Same({0.f,0.f},{-0.f,0.f})&&!Same({.1f,.1f},{.09f,.1f})&&
          Same({.1f,.1f},{.1f,.1f}),"Setup probe bit guard failed");
  SetupTrace disabled;int calls=0;
  disabled.Apply("synthetic",[&]{++calls;});
  Require(calls==1&&disabled.calls==1&&disabled.stored==0,"Coarse probe forwarded incorrectly");
  SetupTrace enabled;enabled.enabled=true;
  enabled.Apply("synthetic",[&]{++calls;});
  Require(calls==2&&enabled.calls==1&&enabled.stored==1&&
    std::strcmp(enabled.operations[0].name,"synthetic")==0,"Observed probe forwarded incorrectly");
  for(unsigned i=enabled.calls;i<64;++i)enabled.Apply("synthetic",[]{});
  bool rejected=false;
  try{enabled.Apply("overflow",[&]{++calls;});}catch(const std::runtime_error&){rejected=true;}
  Require(rejected&&calls==2&&enabled.stored==64,"Probe overflow did not reject before operation");
  std::cout<<"Setup FP probe self-test PASS\n";
}
int main(int argc,char** argv){
  try{
    Require(argc==2,"Usage: k1_setup_fp --self-test|--coarse|--observe|--candidate-first");
    if(std::strcmp(argv[1],"--self-test")==0){SelfTest();return 0;}
    const bool coarse=std::strcmp(argv[1],"--coarse")==0;
    const bool reverse=std::strcmp(argv[1],"--candidate-first")==0;
    Require(coarse||reverse||std::strcmp(argv[1],"--observe")==0,"Unknown setup probe mode");
    // No printing, formatting, allocation or status clearing between a setup
    // operation's snapshots. Engines are heap-allocated in the same order.
    const auto begin=fp_observation::Capture();
    auto reference=std::make_unique<Reference>();
    auto candidate=std::make_unique<Candidate>();
    const auto objectsCreated=fp_observation::Capture();
    SetupTrace referenceTrace,candidateTrace;
    referenceTrace.enabled=candidateTrace.enabled=!coarse;
    ObservedSetup<Reference> a{*reference,referenceTrace};
    ObservedSetup<Candidate> b{*candidate,candidateTrace};
    Snapshot referenceBefore,referenceAfter,candidateBefore,candidateAfter;
    const auto setupReference=[&]{
      referenceBefore=fp_observation::Capture();
      sawstar_test::ConfigureDispatchFixture(a,48000.,0,0,false,2);
      referenceAfter=fp_observation::Capture();
    };
    const auto setupCandidate=[&]{
      candidateBefore=fp_observation::Capture();
      sawstar_test::ConfigureDispatchFixture(b,48000.,0,0,false,2);
      candidateAfter=fp_observation::Capture();
    };
    if(reverse){setupCandidate();setupReference();}else{setupReference();setupCandidate();}
    Require(referenceTrace.calls==33&&candidateTrace.calls==33,"Incomplete setup API sequence");
    Require(reference->ActiveVoices()==16&&candidate->ActiveVoices()==16,"Setup did not create sounding voices");
    // Render order stays Reference then Candidate in all modes. Only setup
    // order changes in --candidate-first. These four frames are a smoke check.
    fp_observation::Frame firstFrame;
    for(int frame=0;frame<4;++frame){
      fp_observation::Frame snapshots;snapshots.index=frame;
      snapshots.referenceBefore=fp_observation::Capture();
      const auto x=reference->ProcessStereo();
      snapshots.referenceAfter=fp_observation::Capture();
      snapshots.candidateBefore=fp_observation::Capture();
      const auto y=candidate->ProcessStereo();
      snapshots.candidateAfter=fp_observation::Capture();
      if(frame==0)firstFrame=snapshots;
      Require(std::isfinite(x.left)&&std::isfinite(x.right)&&
              std::isfinite(y.left)&&std::isfinite(y.right),"Nonfinite setup probe audio");
      Require(Same(x,y)&&Same(reference->PreFX(),candidate->PreFX()),"Setup probe audio bit divergence");
      Require(std::abs(x.left)<=.98001f&&std::abs(x.right)<=.98001f,"Unprotected setup probe output");
    }
    std::cout<<"setup_probe schema=1 passive=true state=complete mode="<<(coarse?"coarse":"observed")
      <<" order="<<(reverse?"candidate-reference":"reference-candidate")
      <<" waveform=2 rate=48000 filter=0 voice=0 fx=0 steps_per_engine=33 rendered_frames=4\n";
    fp_observation::PrintSnapshot(std::cout,"case_begin",begin);
    fp_observation::PrintSnapshot(std::cout,"objects_created",objectsCreated);
    if(reverse){
      fp_observation::PrintSnapshot(std::cout,"candidate_setup_before",candidateBefore);
      fp_observation::PrintSnapshot(std::cout,"candidate_setup_after",candidateAfter);
    }
    fp_observation::PrintSnapshot(std::cout,"reference_setup_before",referenceBefore);
    fp_observation::PrintSnapshot(std::cout,"reference_setup_after",referenceAfter);
    if(!reverse){
      fp_observation::PrintSnapshot(std::cout,"candidate_setup_before",candidateBefore);
      fp_observation::PrintSnapshot(std::cout,"candidate_setup_after",candidateAfter);
    }
    if(reverse)candidateTrace.Print(std::cout,"candidate");
    referenceTrace.Print(std::cout,"reference");
    if(!reverse)candidateTrace.Print(std::cout,"candidate");
    fp_observation::PrintFrame(std::cout,firstFrame);
    std::cout<<"setup_probe_audio_identity PASS\n";
  }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
