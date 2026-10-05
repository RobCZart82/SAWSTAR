// SPDX-License-Identifier: MIT
#include "../experiments/premium_filter/PremiumDrive.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
using Frame = std::array<float, 2>;
template<unsigned Factor> using Full = sawstar::experimental::FixedRatePremiumDrive<Factor>;
template<unsigned Factor> using Study = sawstar::experimental::FixedRatePremiumDrive<Factor, true, true>;
volatile double checksum = 0;
template<unsigned Factor> void Contract() {
  for (double rate : {8000.,44100.,48000.,96000.,192000.,384000.})
    for (double amplitude : {1e-6,.1,.75}) for (int modulation : {0,1}) {
      Full<Factor> a; Study<Factor> b; a.Init(rate); b.Init(rate);
      a.Set(20); b.Set(20); a.SnapToTargets(); b.SnapToTargets();
      double error=0,energy=0,peak=0; std::vector<double> difference(8192);
      for (int n=0;n<8192;++n) {
        if (modulation && n%128==0) { const double db=(n/128)%3*12; a.Set(db);b.Set(db); }
        if (n==3333) { a.Clear();b.Clear(); }
        const Frame x{static_cast<float>(amplitude*(.7*std::sin(n*.173)+.3*std::sin(n*.031))),
                      static_cast<float>(amplitude*std::cos(n*.117))};
        const auto y=a.Process(x),z=b.Process(x);
        for(int ch=0;ch<2;++ch) {
          if(!std::isfinite(z[ch])) throw std::runtime_error("Study nonfinite");
          const double d=double(z[ch])-y[ch]; error+=d*d;energy+=double(y[ch])*y[ch];peak=std::max(peak,std::abs(d));
          if(ch==0)difference[n]=d;
        }
      }
      if(peak>2e-7 || std::sqrt(error/energy)>2e-7)throw std::runtime_error("Study waveform error");
      // Coherent DFT projections of the difference, including low and high bins.
      // This bounds selected-bin changes, not absolute alias energy.
      double spectral=0;
      for(int bin:{1,17,257,1023,2047,4095}) {
        double real=0,imag=0;
        for(int n=0;n<8192;++n) { const double phase=6.283185307179586*bin*n/8192;
          real+=difference[n]*std::cos(phase);imag-=difference[n]*std::sin(phase); }
        spectral=std::max(spectral,2*std::hypot(real,imag)/8192);
      }
      if(spectral>2e-8)throw std::runtime_error("Study spectral difference");
      const auto copyA=a;const auto copyB=b;a=copyA;b=copyB;
      for(int n=0;n<128;++n) {
        const Frame input{n==0?std::numeric_limits<float>::quiet_NaN():0.f,.1f};
        const auto y=a.Process(input),z=b.Process(input);
        if(y[0]!=0||z[0]!=0||!std::isfinite(z[1])||std::abs(double(y[1])-z[1])>2e-7)
          throw std::runtime_error("Study copy/channel recovery");
      }
      a.Init(48000);b.Init(48000);
      for(int n=0;n<64;++n)if(a.Process({0,0})!=b.Process({0,0}))throw std::runtime_error("Study reset");
      std::cout<<Factor<<','<<rate<<','<<amplitude<<','<<modulation<<','<<peak<<','<<std::sqrt(error/energy)<<','<<spectral<<'\n';
    }
}
template<class D> double Measure(double rate,const std::vector<Frame>& input) {
  std::array<D,16> bank;for(auto& d:bank){d.Init(rate);d.Set(20);d.SnapToTargets();}
  for(int n=0;n<1024;++n)for(auto& d:bank)d.Process(input[n]);
  double sum=0;const auto start=std::chrono::steady_clock::now();
  for(auto x:input)for(auto& d:bank){const auto y=d.Process(x);sum+=y[0]+y[1];}
  const double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();checksum=checksum+sum;return seconds;
}
template<unsigned Factor> void Benchmark() {
  std::vector<Frame> input(16384);for(int n=0;n<16384;++n)input[n]={float(.4*std::sin(n*.057)),float(.4*std::cos(n*.083))};
  for(double rate:{48000.,96000.,192000.})for(int pair=0;pair<8;++pair){
    double a,b;if(pair%2){b=Measure<Study<Factor>>(rate,input);a=Measure<Full<Factor>>(rate,input);}
    else{a=Measure<Full<Factor>>(rate,input);b=Measure<Study<Factor>>(rate,input);}
    if(!std::isfinite(a)||!std::isfinite(b)||a<=0||b<=0)throw std::runtime_error("Invalid timing");
    std::cout<<Factor<<','<<rate<<",16,20,"<<pair<<','<<a<<','<<b<<'\n';
  }
}
int main(int argc,char** argv){try{
  if(argc==1){std::cout<<"factor,rate,amplitude,modulation,peak_error,relative_rms,spectral_difference\n";Contract<2>();Contract<4>();}
  else if(argc==2&&std::string(argv[1])=="--benchmark"){
    std::cout<<"factor,rate,voices,drive_db,pair,reference_seconds,study_seconds\n";Benchmark<2>();Benchmark<4>();
    if(!std::isfinite(checksum))throw std::runtime_error("Invalid checksum");
  }else throw std::runtime_error("Usage: premium_drive_tanh [--benchmark]");
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
