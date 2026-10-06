// SPDX-License-Identifier: MIT
#include "../experiments/premium_filter/PremiumDrive.h"
#include <algorithm>
#include <chrono>
#include <complex>
#include <cstdint>
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

using Complex = std::complex<double>;
constexpr size_t SpectrumSize = 2048;
std::vector<Complex> Spectrum(const std::vector<double>& samples) {
  if(samples.size()!=SpectrumSize)throw std::runtime_error("Invalid spectrum size");
  std::vector<Complex> a(samples.begin(),samples.end());
  for(size_t i=1,j=0;i<a.size();++i) {
    size_t bit=a.size()>>1;
    for(;j&bit;bit>>=1)j^=bit;
    j^=bit;if(i<j)std::swap(a[i],a[j]);
  }
  for(size_t len=2;len<=a.size();len<<=1) {
    const Complex step=std::polar(1.,-6.283185307179586/len);
    for(size_t base=0;base<a.size();base+=len) {
      Complex w=1.;
      for(size_t j=0;j<len/2;++j) {
        const Complex even=a[base+j],odd=w*a[base+j+len/2];
        a[base+j]=even+odd;a[base+j+len/2]=even-odd;w*=step;
      }
    }
  }
  return a;
}
double SpectralPeak(const std::vector<double>& difference) {
  const auto bins=Spectrum(difference);double peak=0;
  for(size_t k=0;k<=SpectrumSize/2;++k) {
    const double scale=k==0||k==SpectrumSize/2?1.:2.;
    peak=std::max(peak,scale*std::abs(bins[k])/SpectrumSize);
  }
  return peak;
}
void SpectrumControls() {
  // Independent coherent sine, DC and Nyquist controls exercise every FFT stage
  // and the one-sided endpoint normalization. Perturbed candidates must fail.
  for(int bin:{0,1,257,1023,1024}) {
    std::vector<double> x(SpectrumSize);
    for(size_t n=0;n<x.size();++n)x[n]=1e-4*std::cos(6.283185307179586*bin*n/x.size());
    const double peak=SpectralPeak(x);
    if(std::abs(peak-1e-4)>1e-12||peak<=2e-8)
      throw std::runtime_error("Spectrum negative control failed");
  }
  if(SpectralPeak(std::vector<double>(SpectrumSize,0.))!=0.)
    throw std::runtime_error("Spectrum zero control failed");
}
template<unsigned Factor> void Stress() {
  for(double rate:{8000.,44100.,48000.,96000.,192000.,384000.})
    for(double amplitude:{1e-8,.25,4.})for(int signal=0;signal<4;++signal)for(int mod=0;mod<2;++mod) {
      Full<Factor> a;Study<Factor> b;a.Init(rate);b.Init(rate);
      a.Set(24);b.Set(24);a.SnapToTargets();b.SnapToTargets();
      std::array<std::vector<double>,2> difference{
        std::vector<double>(SpectrumSize),std::vector<double>(SpectrumSize)};
      double peak=0,error=0,energy=0;uint32_t random=0x6d2b79f5u;
      for(size_t n=0;n<2*SpectrumSize;++n) {
        if(mod) {
          const double db=12.+12.*std::sin(n*.079);
          a.Set(db);b.Set(db);
          if(n%257==0){a.SnapToTargets();b.SnapToTargets();}
        }
        if(n==777){a.Clear();b.Clear();}
        random^=random<<13;random^=random>>17;random^=random<<5;
        double x=0;
        switch(signal) {
          case 0:x=std::sin(6.283185307179586*839*n/SpectrumSize);break;
          case 1:x=std::sin(6.283185307179586*(.003*n+.000049*n*n));break;
          case 2:x=(double(random)/4294967295.)*2.-1.;break;
          case 3:x=n%97==0?1.:(n%97==1?-1.:0.);break;
        }
        const Frame input{float(amplitude*x),float(amplitude*(.6*x+.4*std::cos(n*.391)))};
        const auto y=a.Process(input),z=b.Process(input);
        for(size_t ch=0;ch<2;++ch) {
          if(!std::isfinite(y[ch])||!std::isfinite(z[ch]))throw std::runtime_error("Stress nonfinite");
          const double d=double(z[ch])-y[ch];peak=std::max(peak,std::abs(d));
          error+=d*d;energy+=double(y[ch])*y[ch];
          if(n>=SpectrumSize)difference[ch][n-SpectrumSize]=d;
        }
      }
      const double relative=energy>0?std::sqrt(error/energy):std::sqrt(error);
      const double spectral=std::max(SpectralPeak(difference[0]),SpectralPeak(difference[1]));
      if(peak>2e-7||relative>2e-7||spectral>2e-8)
        throw std::runtime_error("Stress waveform/spectrum limit");
      // Fork live FIR histories. Each copied path must reproduce its own source.
      auto ac=a;auto bc=b;
      for(int n=0;n<129;++n) {
        const Frame input{float(.2*std::sin(n*.11)),float(.3*std::cos(n*.07))};
        if(a.Process(input)!=ac.Process(input)||b.Process(input)!=bc.Process(input))
          throw std::runtime_error("Stress live history copy");
      }
      // A poisoned left input must not reset the warmed right channel.
      auto cleanA=a;auto cleanB=b;
      const auto ya=a.Process({std::numeric_limits<float>::infinity(),.125f});
      const auto yb=b.Process({std::numeric_limits<float>::quiet_NaN(),.125f});
      const auto ca=cleanA.Process({0,.125f}),cb=cleanB.Process({0,.125f});
      if(ya[0]!=0.f||yb[0]!=0.f||ya[1]!=ca[1]||yb[1]!=cb[1])
        throw std::runtime_error("Stress channel isolation");
      a.Clear();b.Clear();
      for(int n=0;n<129;++n)if(a.Process({0,0})!=Frame{}||b.Process({0,0})!=Frame{})
        throw std::runtime_error("Stress clear silence");
      std::cout<<"stress,"<<Factor<<','<<rate<<','<<amplitude<<','<<signal<<','<<mod
               <<','<<peak<<','<<relative<<','<<spectral<<'\n';
    }
  Full<Factor> a;Study<Factor> b;a.Init(48000);b.Init(48000);
  int peakA=-1,peakB=-1;double maxA=0,maxB=0;
  for(int n=0;n<129;++n) {
    const Frame input{n==0?1.f:0.f,0.f};const auto y=a.Process(input),z=b.Process(input);
    if(std::abs(y[0])>maxA){maxA=std::abs(y[0]);peakA=n;}
    if(std::abs(z[0])>maxB){maxB=std::abs(z[0]);peakB=n;}
    if(y!=z||y[1]!=0.f)throw std::runtime_error("Study unity latency response");
  }
  if(peakA!=Full<Factor>::Latency||peakB!=Study<Factor>::Latency)
    throw std::runtime_error("Study impulse peak latency");
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
  if(argc==1){std::cout<<"factor,rate,amplitude,modulation,peak_error,relative_rms,spectral_difference\n";Contract<2>();Contract<4>();SpectrumControls();Stress<2>();Stress<4>();}
  else if(argc==2&&std::string(argv[1])=="--benchmark"){
    std::cout<<"factor,rate,voices,drive_db,pair,reference_seconds,study_seconds\n";Benchmark<2>();Benchmark<4>();
    if(!std::isfinite(checksum))throw std::runtime_error("Invalid checksum");
  }else throw std::runtime_error("Usage: premium_drive_tanh [--benchmark]");
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
