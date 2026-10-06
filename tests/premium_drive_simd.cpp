// SPDX-License-Identifier: MIT
#include "../experiments/premium_filter/PremiumDrive.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
namespace {
using Frame=std::array<float,2>;
unsigned long long compared = 0;
template<unsigned Factor> void Contract() {
  using Candidate = sawstar::experimental::FixedRatePremiumDrive<Factor, true, false, true>;
  using Reference = sawstar::experimental::FixedRatePremiumDrive<Factor>;
  static_assert(sizeof(Candidate) == sizeof(Reference), "SIMD adds no persistent state");
  static_assert(Candidate::Latency == Reference::Latency, "Latency is unchanged");
  auto compare = [](Candidate& a, Reference& b, Frame x) {
    const auto actual = a.Process(x), expected = b.Process(x);
    if (std::memcmp(actual.data(), expected.data(), sizeof(actual)) != 0)
      throw std::runtime_error("SIMD FIR changed samples, factor=" + std::to_string(Factor)
        + ", compared=" + std::to_string(compared));
    ++compared;
  };
  for (double rate : {8000., 44100., 48000., 96000., 192000., 384000.}) {
    Candidate a; Reference b; a.Init(rate); b.Init(rate);
    uint32_t random = 1;
    for (int n = 0; n < 65536; ++n) {
      if (n % 101 == 0) { const double db = (n / 101) % 25; a.Set(db); b.Set(db); }
      if (n % 997 == 0) { a.SnapToTargets(); b.SnapToTargets(); }
      if (n % 4093 == 0) { a.Clear(); b.Clear(); }
      Frame x{};
      for (auto& v : x) {
        random = random * 1664525u + 1013904223u;
        const double scale = (n / 1024) % 3 == 0 ? 1e-8 : (n / 1024) % 3 == 1 ? 1.6 : 8.;
        v = static_cast<float>((random / 4294967296. - .5) * scale);
      }
      if (n % 1031 == 0) x[0] = std::numeric_limits<float>::quiet_NaN();
      if (n % 2053 == 0) x[1] = std::numeric_limits<float>::infinity();
      if (n == 30000) { a.Set(std::numeric_limits<double>::quiet_NaN()); b.Set(std::numeric_limits<double>::quiet_NaN()); }
      if (n == 20000) {
        // Nonzero ring positions must survive copying and repeated use.
        auto copyA = a; auto copyB = b;
        for (int i = 0; i < 512; ++i) compare(copyA, copyB, {.2f, -.3f});
        a = copyA; b = copyB;
      }
      compare(a, b, x);
    }
    for (double db : {0., 20., 24.}) for (int offset : {0, 31, 32, 33, 63, 64, 65, 127, 128, 255, 256}) {
      a.Init(rate); b.Init(rate); a.Set(db); b.Set(db); a.SnapToTargets(); b.SnapToTargets();
      for (int n = 0; n < 512; ++n)
        compare(a, b, {n == offset ? .8f : 0.f, n == offset + 1 ? -.6f : 0.f});
    }
    // Reinitialization at another rate erases both histories and gain targets.
    a.Init(rate == 48000 ? 192000 : 48000); b.Init(rate == 48000 ? 192000 : 48000);
    for (int n = 0; n < 256; ++n) compare(a, b, {0.f, -0.f});
  }
  std::cout << Factor << "x object bytes: " << sizeof(Reference) << " -> " << sizeof(Candidate) << '\n';
}

void KernelContract() {
#if defined(__clang__)
#pragma clang fp contract(off)
#endif
  std::array<double,16> taps{};
  std::array<double,48> history{};
  for(size_t i=0;i<taps.size();++i)taps[i]=std::sin(double(i+1)*.173);
  for(size_t i=0;i<history.size();++i)history[i]=std::cos(double(i+3)*.097)*double(i+1);
  std::array<double,4> paired{},straight{},wrong{};
  sawstar::experimental::detail::FirLanes4 p,s;
  for(unsigned i=0;i<16;i+=4) {
    p.Symmetric(taps.data()+i,history.data()+32-i,history.data()+i);
    s.Straight(taps.data()+i,history.data()+32-i);
    for(unsigned lane=0;lane<4;++lane) {
      const unsigned t=i+lane;
      paired[lane]+=taps[t]*(history[32-t]+history[t]);
      straight[lane]+=taps[t]*history[32-t];
      wrong[lane]+=taps[t]*history[29-i+lane]; // unreversed vector load
    }
  }
  const double a=(paired[0]+paired[1])+(paired[2]+paired[3]);
  const double b=(straight[0]+straight[1])+(straight[2]+straight[3]);
  const double c=(wrong[0]+wrong[1])+(wrong[2]+wrong[3]);
  const double actualA=p.Sum(),actualB=s.Sum();
  if(std::memcmp(&a,&actualA,sizeof(a))||std::memcmp(&b,&actualB,sizeof(b)))
    throw std::runtime_error("SIMD primitive ordering/rounding");
  if(std::abs(c-b)<1e-3)throw std::runtime_error("Kernel cannot reject reversed-lane bug");
}
volatile double checksum=0;
template<class D> double Measure(double rate,const std::vector<Frame>& input) {
  std::array<D,16> bank;
  for(auto& d:bank){d.Init(rate);d.Set(20);d.SnapToTargets();for(int n=0;n<1024;++n)d.Process(input[n]);}
  double sum=0;const auto start=std::chrono::steady_clock::now();
  for(auto x:input)for(auto& d:bank){const auto y=d.Process(x);sum+=y[0]+y[1];}
  const double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
  checksum=checksum+sum;return seconds;
}
template<unsigned Factor> void Benchmark() {
  std::vector<Frame> input(16384);
  for(int n=0;n<16384;++n)input[n]={float(.4*std::sin(n*.057)),float(.4*std::cos(n*.083))};
  using Scalar=sawstar::experimental::FixedRatePremiumDrive<Factor>;
  using Simd=sawstar::experimental::FixedRatePremiumDrive<Factor,true,false,true>;
  for(double rate:{48000.,96000.,192000.})for(int pair=0;pair<4;++pair) {
    double a,b;
    if(pair%2){b=Measure<Simd>(rate,input);a=Measure<Scalar>(rate,input);}
    else{a=Measure<Scalar>(rate,input);b=Measure<Simd>(rate,input);}
    if(!std::isfinite(a)||!std::isfinite(b)||a<=0||b<=0)throw std::runtime_error("Invalid SIMD timing");
    std::cout<<Factor<<','<<rate<<",16,20,"<<pair<<','<<a<<','<<b<<'\n';
  }
}
}
int main(int argc,char** argv) {
  try {
    if(argc==1) {
      KernelContract();Contract<2>();Contract<4>();
      std::cout<<compared<<" stereo frames bit-identical in SIMD fixture; backend "
               <<sawstar::experimental::detail::FirLanes4::Backend<<'\n';
    }else if(argc==2&&std::string(argv[1])=="--benchmark") {
      std::cout<<std::setprecision(17)<<"factor,rate,voices,drive_db,pair,reference_seconds,simd_seconds\n";
      Benchmark<2>();Benchmark<4>();
      if(!std::isfinite(checksum))throw std::runtime_error("Invalid SIMD checksum");
    }else throw std::runtime_error("Usage: premium_drive_simd [--benchmark]");
  }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
