// SPDX-License-Identifier: MIT
// Offline paired complete-engine study. Timings never gate CI.
#include "PremiumSynth.h"
#include "TanhPremiumSynth.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using Reference = sawstar::experimental_engine::Synth;
using Study = sawstar::experimental_tanh_engine::Synth;
struct Distribution {
  double median=0,p95=0,p99=0,worst=0;
  size_t over=0;
};
Distribution Summarize(std::vector<double> times) {
  if(times.empty())throw std::runtime_error("Empty timing distribution");
  for(double x:times)if(!std::isfinite(x)||x<=0)throw std::runtime_error("Invalid block timing");
  Distribution d;
  d.over=std::count_if(times.begin(),times.end(),[](double x){return x>100.;});
  std::sort(times.begin(),times.end());
  const size_t n=times.size();
  d.median=n%2?times[n/2]:(times[n/2-1]+times[n/2])/2.;
  d.p95=times[(95*n+99)/100-1];d.p99=times[(99*n+99)/100-1];d.worst=times.back();
  return d;
}
template<class S> void Setup(S& s,double rate,int mode) {
  s.Reset(rate);s.SetParameters(-6,5,100,.8,100);s.SetOutputBoost(18);
  s.SetSaw(25,70,70);s.SetOsc2(19,60,80);s.SetMixer(70,50,20,5,0,-1,0,0);
  s.SetFilter(1800,50,100);s.SetFilterCharacter(20,mode);
  // Explicit native units: delay tone Hz, reverb decay seconds, damping Hz.
  s.SetChorus(true,20,.3,30);s.SetDelay(true,15,250,30,2000,true,false,2,120);
  s.SetReverb(true,15,50,2,6000);
  for(int n=0;n<16;++n)s.Midi(0x90,48+n,100);
}
void Check(sawstar::StereoSample y) {
  if(!std::isfinite(y.left)||!std::isfinite(y.right)||
     std::abs(y.left)>.98001f||std::abs(y.right)>.98001f)
    throw std::runtime_error("Invalid protected engine output");
}
void Contract() {
  const auto d=Summarize({150,25,100,50});
  if(d.median!=75||d.p95!=150||d.p99!=150||d.worst!=150||d.over!=1)
    throw std::runtime_error("Distribution endpoints/strict budget");
  if(Summarize({1,2,3}).median!=2)throw std::runtime_error("Odd median");
  std::vector<double> rank(100);
  for(size_t n=0;n<rank.size();++n)rank[n]=double(n+1);
  const auto ranks=Summarize(rank);
  if(ranks.p95!=95||ranks.p99!=99||ranks.over!=0)
    throw std::runtime_error("Nearest rank percentiles");
  for(auto invalid:{std::vector<double>{},std::vector<double>{0},
      std::vector<double>{-1},std::vector<double>{NAN},std::vector<double>{INFINITY}}) {
    bool rejected=false;try{Summarize(invalid);}catch(const std::runtime_error&){rejected=true;}
    if(!rejected)throw std::runtime_error("Invalid distribution accepted");
  }
  double maximum=0;
  for(double rate:{48000.,96000.,192000.})for(int mode=0;mode<4;++mode) {
    auto a=std::make_unique<Reference>();auto b=std::make_unique<Study>();
    Setup(*a,rate,mode);Setup(*b,rate,mode);double energy=0;
    for(int n=0;n<4096;++n) {
      const auto x=a->ProcessStereo(),y=b->ProcessStereo();Check(x);Check(y);
      maximum=std::max({maximum,std::abs(double(x.left)-y.left),std::abs(double(x.right)-y.right)});
      energy+=double(x.left)*x.left+double(x.right)*x.right;
    }
    if(a->ActiveVoices()!=16||b->ActiveVoices()!=16||energy<=1e-12||maximum>2e-6)
      throw std::runtime_error("Paired sounding fixture contract");
  }
  std::cout<<"Paired deadline fixture and percentile contracts PASS; max difference "<<maximum<<'\n';
}
struct Result {
  std::vector<double> times;
  double peak=0,energy=0,sum=0;
};
template<class S> Result Measure(double rate,int mode,int buffer) {
  constexpr int blocks=256;
  auto s=std::make_unique<S>();Setup(*s,rate,mode);
  for(int n=0;n<static_cast<int>(rate/4);++n)Check(s->ProcessStereo());
  Result r;r.times.resize(blocks);
  std::vector<sawstar::StereoSample> output(buffer);
  for(auto& time:r.times) {
    const auto start=std::chrono::steady_clock::now();
    for(auto& y:output)y=s->ProcessStereo();
    time=100.*rate/buffer*std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
    // Validation and diagnostic arithmetic are outside each timed block.
    for(const auto y:output) {
      Check(y);r.peak=std::max({r.peak,std::abs(double(y.left)),std::abs(double(y.right))});
      r.energy+=double(y.left)*y.left+double(y.right)*y.right;r.sum+=double(y.left)+y.right;
    }
  }
  if(s->ActiveVoices()!=16||!std::isfinite(r.energy)||r.energy<=0||!std::isfinite(r.sum))
    throw std::runtime_error("Invalid measured sounding fixture");
  Summarize(r.times);return r;
}
void Row(std::ostream& summary,std::ostream& raw,const char* label,double rate,
         int mode,int buffer,int pair,const Result& r) {
  const auto d=Summarize(r.times);
  summary<<label<<','<<rate<<",16,"<<mode<<",20,1,"<<buffer<<','<<r.times.size()<<','<<pair
    <<','<<(pair%2?"study-first":"reference-first")<<','<<d.median<<','<<d.p95<<','<<d.p99
    <<','<<d.worst<<','<<d.over<<','<<r.peak<<','<<std::sqrt(r.energy/(2*r.times.size()*buffer))
    <<','<<r.sum<<'\n';
  for(size_t block=0;block<r.times.size();++block)
    raw<<label<<','<<rate<<','<<mode<<','<<buffer<<','<<pair<<','<<block<<','<<r.times[block]<<'\n';
}
void Benchmark(const std::filesystem::path& folder) {
  std::filesystem::create_directories(folder);
  std::ofstream summary(folder/"summary.csv"),raw(folder/"blocks.csv");
  if(!summary||!raw)throw std::runtime_error("Cannot open measurement files");
  summary<<std::setprecision(17);raw<<std::setprecision(17);
  const char* header="engine,rate,voices,filter_mode,drive_db,fx,buffer,blocks,pair,order,median_block_percent,p95_block_percent,p99_block_percent,worst_block_percent,over_budget_blocks,peak,rms,checksum";
  summary<<header<<'\n';raw<<"engine,rate,filter_mode,buffer,pair,block,block_percent\n";
  for(double rate:{48000.,96000.,192000.})for(int mode=0;mode<4;++mode)
    for(int buffer:{32,64,128})for(int pair=0;pair<4;++pair) {
      Result a,b;
      if(pair%2){b=Measure<Study>(rate,mode,buffer);a=Measure<Reference>(rate,mode,buffer);}
      else{a=Measure<Reference>(rate,mode,buffer);b=Measure<Study>(rate,mode,buffer);}
      Row(summary,raw,"reference",rate,mode,buffer,pair,a);
      Row(summary,raw,"study",rate,mode,buffer,pair,b);
    }
  summary.close();raw.close();
  if(!summary||!raw)throw std::runtime_error("Incomplete measurement write");
  std::ifstream report(folder/"summary.csv");std::cout<<report.rdbuf();
  if(!std::cout||report.bad())throw std::runtime_error("Cannot report measurement");
}
int main(int argc,char** argv) {
  try {
    if(argc==1)Contract();
    else if(argc==3&&std::string(argv[1])=="--deadline")Benchmark(argv[2]);
    else throw std::runtime_error("Usage: premium_tanh_deadline [--deadline output-directory]");
  }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
