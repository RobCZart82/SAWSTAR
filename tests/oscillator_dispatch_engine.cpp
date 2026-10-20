// SPDX-License-Identifier: MIT
// Synth API control timeline only: not VST3 automation or native acceptance.
#ifdef SAWSTAR_CALIBRATED_PREVIEW_PAIR
#include "PremiumNativeCandidateSynth.h"
#include "PremiumPreviewSynth.h"
#else
#include "DispatchPremiumSynth.h"
#include "LookupPremiumSynth.h"
#endif
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <sstream>
#include "NumericalComparison.h"
#ifdef SAWSTAR_CALIBRATED_PREVIEW_PAIR
using Reference = sawstar::premium_preview_engine::Synth;
using Study = sawstar::premium_native_candidate_engine::Synth;
#else
using Reference = sawstar::experimental_lookup_engine::Synth;
using Study = sawstar::experimental_dispatch_engine::Synth;
#endif
static_assert(sizeof(Reference)==sizeof(Study), "Dispatch engine adds no state");
int Waveform = 0;
void Require(bool ok, const char* why) { if (!ok) throw std::runtime_error(why); }
struct Error {
  double peak = 0, squared = 0, energy = 0;
  void Add(sawstar::StereoSample a, sawstar::StereoSample b) {
    for (auto pair : {std::pair<float, float>{a.left, b.left}, {a.right, b.right}}) {
      Require(std::isfinite(pair.first) && std::isfinite(pair.second), "Nonfinite engine comparison");
      Require(std::memcmp(&pair.first, &pair.second, sizeof(float)) == 0, "Dispatch engine bit identity");
      const double delta = double(pair.first) - pair.second;
      peak = std::max(peak, std::abs(delta)); squared += delta * delta;
      energy += double(pair.second) * pair.second;
    }
  }
  double Relative() const { Require(energy > 1e-12, "Silent engine fixture"); return std::sqrt(squared / energy); }
  void Check() const { Require(peak == 0 && Relative() == 0, "Engine dispatch identity"); }
};
template<class S> void Setup(S& s, double rate, int filter, int voice, bool fx) {
  s.Reset(rate); s.SetParameters(-12, 2, 50, .7, 10); s.SetOutputBoost(12);
  s.SetSaw(25, 70, 70); s.SetOsc2(19, 60, 80);
  s.SetMixer(70, 50, 20, 5, 0, -1, 0, 0);
  s.SetFilter(1800, 50, 100); s.SetFilterCharacter(20, filter);
  s.SetFilterEnvelope(60, 100, 1, 30, .3f, 40);
  s.SetVoiceMode(voice, 15, true); s.SetPerformance(24, 24);
  s.SetLfo(20, 70, 3, 0, false, 0, 136, true);
  s.SetLfo2(17, 50, 2, 1, false, 0, 136, false);
  s.SetModulation(0, 3, 0, 100); s.SetModulation(1, 4, 0, -50);
  s.SetModulation(2, 5, 1, 20);
  if (fx) {
    s.SetChorus(true, 20, .3, 30);
    s.SetDelay(true, 15, 250, 30, 2000, true, false, 2, 136);
    s.SetReverb(true, 15, 50, 2, 6000);
  }
  s.SetWaveforms(Waveform,Waveform);
  for (int note = 48; note < (voice == 0 ? 64 : 51); ++note) s.Midi(0x90, note, 100);
}
template<class S> void Event(S& s, int n, int filter, int voice) {
  if (n % 128 == 0) s.SetFilter(500 + 6000 * ((n / 128) % 8) / 7.f, 50, 100);
  if (n % 512 == 0) {
    const float db = (n / 512) % 3 * 12.f;
    s.SetFilterCharacter(db, filter);
    s.Midi(0xb0, 1, (n / 512) % 2 ? 127 : 0);
    s.Midi(0xd0, (n / 512) % 2 ? 127 : 0, 0);
  }
  if (n == 511) s.Midi(0xb0, 64, 127);
  if (n == 768) s.Midi(0x80, voice == 0 ? 48 : 50, 0);
  if (n == 1024) s.Midi(0x90, 36, 100); // Poly stealing or Mono fallback/Legato
  if (n == 1536) s.Midi(0xe0, 0, 127);
  if (n == 2048) { s.Midi(0x80, 36, 0); s.Midi(0xb0, 64, 0); }
  if (n == 2560) {
    for (int note = 48; note < (voice == 0 ? 64 : 51); ++note) s.Midi(0x80, note, 0);
    s.Midi(0xe0, 0, 64); s.Midi(0x90, 60, 80);
  }
  if (n == 3072) {
    s.SetWaveforms((Waveform+1)%4,(Waveform+2)%4);
    s.SetFilterCharacter(20, (filter + 1) % 4);
    s.SetModulation(0, 3, 1, -30); // changing source/destination weights
    s.SetFilter(12000, 50, 35);    // delayed dry/wet blend
    s.Midi(0x80, 60, 0); s.Midi(0x90, 60, 80);
  }
  if (n == 3584) s.Midi(0x80, 60, 0);
}
int main(int argc, char** argv) {
  try {
    if(argc==2&&std::strcmp(argv[1],"--fp-self-test")==0){
      fp_observation::SelfTest();std::cout<<"Passive FP observation self-test passed\n";return 0;
    }
    if(argc==2&&std::strcmp(argv[1],"--diagnostic-self-test")==0){
      std::ostringstream log;bool rejected=false;
      const fp_observation::Snapshot clean{true,0x1f80,FE_TONEAREST},sticky{true,0x1fb3,FE_TONEAREST};
      const fp_observation::Frame testFp{clean,sticky,sticky,sticky,"injected",197};
#ifdef SAWSTAR_NUMERICAL_TRACE
      sawstar::NumericalTrace reference{},candidate{};
      for(auto& voice:candidate.voices)voice.fill(1.f); // more than 16 differing fields
#endif
      try{numerical_comparison::Check({.1f,.1f},{.09f,.1f},{.2f,.2f},{.3f,.2f},
        {2,2,0,48000,true,"injected",197},log
#ifdef SAWSTAR_NUMERICAL_TRACE
        ,candidate,reference
#endif
        ,&testFp
        );}catch(const std::runtime_error&){rejected=true;}
      const auto text=log.str();
      Require(rejected&&text.find("phase=injected frame=197")!=std::string::npos&&
        text.find("output.right candidate_bits=")!=std::string::npos&&
        text.find("pre_fx.left candidate_bits=")!=std::string::npos&&
        text.find("reference_before mxcsr_available=1 raw=0x1f80")!=std::string::npos&&
        text.find("candidate_after mxcsr_available=1 raw=0x1fb3 status=0x33 controls=0x1f80")!=std::string::npos,"Incomplete failure diagnostics");
#ifdef SAWSTAR_NUMERICAL_TRACE
      Require(text.find("trace voice[0].envelope")!=std::string::npos&&
        text.find("trace_differences=256 trace_lines_limit=16")!=std::string::npos&&
        text.find("trace voice[1].")==std::string::npos,"Unbounded or missing trace diagnostics");
      // Signed zero is a bit difference; a sub-output internal divergence must
      // still be found even if both externally observed points remain equal.
      reference={};candidate={};candidate.voices[0][3]=-0.f;rejected=false;log.str("");
      try{numerical_comparison::Check({},{},{},{},{2,2,0,48000,true,"internal",0},log,candidate,reference);}
      catch(const std::runtime_error&){rejected=true;}
      Require(rejected&&log.str().find("trace voice[0].osc1.left")!=std::string::npos,"Missed internal-only bit divergence");
#endif
      log.str("");
      numerical_comparison::Check({},{},{},{},{2,2,0,48000,true,"equal",0},log
#ifdef SAWSTAR_NUMERICAL_TRACE
        ,reference,reference
#endif
        );
      Require(log.str().empty(),"Equal frames emitted failure diagnostics");
      std::cout<<"Numerical failure diagnostics self-test passed trace="
#ifdef SAWSTAR_NUMERICAL_TRACE
        <<"on\n";
#else
        <<"off\n";
#endif
      return 0;
    }
    int firstWave = 0, endWave = 4;bool observeFp=false,waveSpecified=false,candidateFirst=false;
    for(int arg=1;arg<argc;++arg){
      if(std::strcmp(argv[arg],"--fp-observe")==0&&!observeFp){observeFp=true;continue;}
      if(std::strcmp(argv[arg],"--candidate-first")==0&&!candidateFirst){candidateFirst=true;continue;}
      Require(std::strcmp(argv[arg],"--waveform")==0&&!waveSpecified&&arg+1<argc,
              "Usage: oscillator_dispatch_engine [--waveform 0|1|2|3] [--fp-observe [--candidate-first]]");
      const char* value=argv[++arg];
      Require(std::strlen(value)==1&&value[0]>='0'&&value[0]<='3',"Waveform must be exactly 0, 1, 2 or 3");
      firstWave=value[0]-'0';endWave=firstWave+1;waveSpecified=true;
    }
    Require(!candidateFirst||observeFp,"--candidate-first requires --fp-observe");
    fp_observation::Campaign fpCampaign(observeFp);
    // The bit identity guard must reject a deliberately altered signal.
    bool rejected = false;
    try {
      Error altered; altered.Add({.1f, .1f}, {.09f, .09f}); altered.Check();
    } catch (const std::runtime_error&) { rejected = true; }
    Require(rejected, "Engine comparison accepted altered audio");
    int cases = 0;
    std::cout << std::setprecision(17) << "waveform,rate,filter_mode,voice_mode,fx,output_peak_error,output_relative_rms,prefx_peak_error,prefx_relative_rms\n";
    for (Waveform=firstWave; Waveform<endWave; ++Waveform) for (double rate : {48000., 96000., 176400., 192000., 384000.})
      for (int filter = 0; filter < 4; ++filter) for (int voice = 0; voice < 3; ++voice)
        for (bool fx : {false, true}) {
          fp_observation::Case observation;observation.waveform=Waveform;observation.rate=rate;
          observation.filter=filter;observation.voice=voice;observation.fx=fx;
          observation.candidateFirst=candidateFirst;
          if(observeFp)observation.boundaries[0]=fp_observation::Capture();
          auto a = std::make_unique<Reference>(); auto b = std::make_unique<Study>();
          if(observeFp)observation.boundaries[1]=fp_observation::Capture();
          Setup(*a, rate, filter, voice, fx);
          if(observeFp)observation.boundaries[2]=fp_observation::Capture();
          Setup(*b, rate, filter, voice, fx);
          if(observeFp)observation.boundaries[3]=fp_observation::Capture();
          const auto renderPair=[&](const char* phase,int frame,unsigned phaseIndex){
            fp_observation::Frame fp;fp.phase=phase;fp.index=frame;fp.candidateFirst=candidateFirst;
            sawstar::StereoSample x,y;
            const auto reference=[&]{if(observeFp)fp.referenceBefore=fp_observation::Capture();x=a->ProcessStereo();if(observeFp)fp.referenceAfter=fp_observation::Capture();};
            const auto candidate=[&]{if(observeFp)fp.candidateBefore=fp_observation::Capture();y=b->ProcessStereo();if(observeFp)fp.candidateAfter=fp_observation::Capture();};
            if(candidateFirst){candidate();reference();}else{reference();candidate();}
            if(observeFp)observation.Observe(fp,phaseIndex);
            return std::pair<sawstar::StereoSample,sawstar::StereoSample>{x,y};
          };
          Require(a->ActiveVoices() == (voice == 0 ? 16 : 1) && a->ActiveVoices() == b->ActiveVoices(),
                  "Missing sounding voice fixture");
          Error out, pre;
          for (int n = 0; n < 4096; ++n) {
            Event(*a, n, filter, voice); Event(*b, n, filter, voice);
            const auto [x,y]=renderPair("main",n,0);
#ifdef SAWSTAR_NUMERICAL_TRACE
            const auto& trace=b->Trace();
            Require(numerical_comparison::Same({trace.stages[1][0],trace.stages[1][1]},b->PreFX())&&
              numerical_comparison::Same({trace.stages[7][0],trace.stages[7][1]},y),"Trace observation differs from actual audio");
            if(b->ActiveVoices())Require(std::any_of(trace.noiseState.begin(),trace.noiseState.end(),
              [](uint32_t state){return state!=0;}),"Trace did not capture active voices");
#endif
            numerical_comparison::Check(y,x,b->PreFX(),a->PreFX(),
              {Waveform,filter,voice,rate,fx,"main",n},std::cerr
#ifdef SAWSTAR_NUMERICAL_TRACE
              ,b->Trace(),a->Trace()
#endif
              ,observeFp?&observation.latest:nullptr
              );
            Require(std::max({std::abs(x.left), std::abs(x.right), std::abs(y.left), std::abs(y.right)}) <= .98001f,
                    "Unprotected engine output");
            out.Add(y, x); pre.Add(b->PreFX(), a->PreFX());
            Require(a->ActiveVoices() == b->ActiveVoices(), "Voice lifecycle mismatch");
            for (int note : {36, 48, 50, 60}) Require(a->Held(note) == b->Held(note), "MIDI ownership mismatch");
          }
          for (int n = 0; n < int(rate / 10); ++n) {
            const auto [x,y]=renderPair("tail",n,1);
            numerical_comparison::Check(y,x,b->PreFX(),a->PreFX(),
              {Waveform,filter,voice,rate,fx,"tail",n},std::cerr
#ifdef SAWSTAR_NUMERICAL_TRACE
              ,b->Trace(),a->Trace()
#endif
              ,observeFp?&observation.latest:nullptr
              );
            out.Add(y,x);pre.Add(b->PreFX(),a->PreFX());
          }
          Require(a->ActiveVoices() == 0 && b->ActiveVoices() == 0, "Release did not complete");
          out.Check(); pre.Check();
          std::cout << Waveform << ',' << rate << ',' << filter << ',' << voice << ',' << fx << ',' << out.peak << ',' << out.Relative()
                    << ',' << pre.peak << ',' << pre.Relative() << '\n';
          if(observeFp)observation.boundaries[4]=fp_observation::Capture();
          a->Reset(rate);
          if(observeFp)observation.boundaries[5]=fp_observation::Capture();
          b->Reset(rate);
          if(observeFp)observation.boundaries[6]=fp_observation::Capture();
          for (int n = 0; n < 64; ++n) {
            const auto [x,y]=renderPair("reset",n,2);
            Require(x.left == 0 && x.right == 0 && y.left == 0 && y.right == 0, "Reset silence mismatch");
          }
          fpCampaign.Add(observation);
          ++cases;
        }
    Require(cases == 120 * (endWave - firstWave), "Incomplete engine grid");
    fpCampaign.Print(std::cerr);
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}

