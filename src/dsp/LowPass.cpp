// SPDX-License-Identifier: MIT
#include "dsp/LowPass.h"
#include <algorithm>
#include <cmath>
#include <complex>
namespace sawstar {
namespace {
float Safe(float v,float lo,float hi){return std::isfinite(v)?std::clamp(v,lo,hi):lo;}
}
void LowPass::Init(float sampleRate){
  rate_=SafeSampleRate(sampleRate);
  startupLength_=std::max(2,static_cast<int>(rate_*.0005));
  dcPole_=std::exp(-2*3.14159265358979323846*5/rate_);
  slew_=1-std::exp(-1/(0.01*rate_));cutoff_=-1;
  mode_=0;weights_={{1,0,0,0}};drive_=targetDrive_=0;
  Set(12000,0,0);g_=targetG_;k_=targetK_;mix_=0;Clear();
}
void LowPass::Clear(){startupRemaining_=0;dcInput_.fill(0);dcOutput_.fill(0);ic1_.fill(0);ic2_.fill(0);cascade1_.fill(0);cascade2_.fill(0);}
void LowPass::SetCharacter(float driveDb,int mode){targetDrive_=Safe(driveDb,0,24);mode_=std::clamp(mode,0,3);}
void LowPass::Set(float hz,float resonance,float mix){
  hz=Safe(hz,20,static_cast<float>(std::min(20000.,rate_*0.45)));
  if(hz!=cutoff_){cutoff_=hz;targetG_=std::tan(3.14159265358979323846*hz/rate_);}
  targetK_=2-1.9*Safe(resonance,0,100)*0.01; // Q=0.5 ... 10; no self-oscillation.
  targetMix_=Safe(mix,0,100)*0.01;
}
StereoSample LowPass::Process(StereoSample input){
  g_+=slew_*(targetG_-g_);k_+=slew_*(targetK_-k_);mix_+=slew_*(targetMix_-mix_);
  if(targetMix_==0 && mix_<1e-9)mix_=0;
  drive_+=slew_*(targetDrive_-drive_);
  if(targetDrive_==0 && drive_<1e-9)drive_=0;
  for(size_t i=0;i<weights_.size();++i)weights_[i]+=slew_*((static_cast<int>(i)==mode_?1.:0.)-weights_[i]);
  const double driveGain=drive_==0?1.:std::pow(10.,drive_/20.);
  const double driveDenominator=drive_==0?1.:std::tanh(driveGain);
  const double driveBlend=std::min(1.,drive_/6.);
  const double a=1/(1+g_*(g_+k_)),cascadeA=1/(1+g_*(g_+2.));
  // An idle oscillator starts without pre-history. Ease its first half
  // millisecond into the filter to reduce cold-state excitation; dry audio
  // and the amplitude envelope remain untouched. No lookahead or warmup loop.
  double startup=1.;
  if(startupRemaining_>0){const double u=double(startupLength_-startupRemaining_)/(startupLength_-1);
    startup=u*u*(3-2*u);--startupRemaining_;}
  std::array<float,2> samples{{input.left,input.right}};
  for(size_t ch=0;ch<2;++ch){
    if(!std::isfinite(samples[ch])){ic1_[ch]=ic2_[ch]=cascade1_[ch]=cascade2_[ch]=dcInput_[ch]=dcOutput_[ch]=0;samples[ch]=0;continue;}
    const double dry=samples[ch];
    double driven=drive_==0?dry:dry+driveBlend*(std::tanh(dry*driveGain)/driveDenominator-dry);
    // Remove saturation-induced DC at 5 Hz. Follow the drive blend so zero
    // drive retains the exact clean path, while histories remain warm.
    const double dcFree=driven-dcInput_[ch]+dcPole_*dcOutput_[ch];
    dcInput_[ch]=driven;dcOutput_[ch]=std::abs(dcFree)<1.e-24?0:dcFree;
    driven+=driveBlend*(dcFree-driven);
    driven*=startup;
    const double v1=a*(ic1_[ch]+g_*(driven-ic2_[ch]));
    const double v2=ic2_[ch]+g_*v1;
    ic1_[ch]=2*v1-ic1_[ch];ic2_[ch]=2*v2-ic2_[ch];
    // Keep long decays out of the denormal range without depending on host flags.
    if(std::abs(ic1_[ch])<1e-24)ic1_[ch]=0;
    if(std::abs(ic2_[ch])<1e-24)ic2_[ch]=0;
    // A non-resonant second stage gives LP24 its extra slope without squaring resonance.
    const double c1=cascadeA*(cascade1_[ch]+g_*(v2-cascade2_[ch]));
    const double c2=cascade2_[ch]+g_*c1;
    cascade1_[ch]=2*c1-cascade1_[ch];cascade2_[ch]=2*c2-cascade2_[ch];
    if(std::abs(cascade1_[ch])<1e-24)cascade1_[ch]=0;
    if(std::abs(cascade2_[ch])<1e-24)cascade2_[ch]=0;
    const double high=driven-k_*v1-v2;
    // k*v1 gives unity at center frequency for the band-pass across the Q range.
    const double wet=weights_[0]*v2+weights_[1]*c2+weights_[2]*high+weights_[3]*k_*v1;
    samples[ch]=static_cast<float>(dry+mix_*(wet-dry));
  }
  return {samples[0],samples[1]};
}
}

namespace sawstar {
bool LowPass::WantsPreparedStart(float fundamental) const {
  return std::isfinite(fundamental) && fundamental > 1.5f * cutoff_
      && fundamental < rate_ * .45 && mode_ < 2 && targetMix_ > 0;
}
bool LowPass::PrepareStart(const StereoSample (&input)[kStartPreviewSamples], float fundamental){
  if (!std::isfinite(fundamental) || fundamental <= 0 || fundamental >= rate_ * .45) return false;
  for (const auto& sample : input)
    if (!std::isfinite(sample.left) || !std::isfinite(sample.right)) return false;
  // A short virtual-period preview supplies the first 16 harmonics. Each
  // harmonic's state solves r*x = A*x + B*u at the REAL sample rate. This
  // avoids running milliseconds of throwaway samples on the first callback.
  // DC is deliberately excluded: a detuned/rounded partial period is not a
  // reliable estimate of a sustained offset. This is an initial-state estimate,
  // not an exact steady-state solution for a beating unison or nonlinear drive.
  constexpr int N=64,H=16;
  // Compile-time table: no first-use trigonometric initialization on the audio thread.
  static constexpr double cosine[N]={
    1,0.99518472667219693,0.98078528040323043,0.95694033573220882,
    0.92387953251128674,0.88192126434835505,0.83146961230254524,0.77301045336273699,
    0.70710678118654757,0.63439328416364549,0.55557023301960229,0.47139673682599781,
    0.38268343236508984,0.29028467725446233,0.19509032201612833,0.09801714032956077,
    6.123233995736766e-17,-0.098017140329560645,-0.19509032201612819,-0.29028467725446216,
    -0.38268343236508973,-0.4713967368259977,-0.55557023301960196,-0.63439328416364538,
    -0.70710678118654746,-0.77301045336273699,-0.83146961230254535,-0.88192126434835494,
    -0.92387953251128674,-0.95694033573220882,-0.98078528040323043,-0.99518472667219682,
    -1,-0.99518472667219693,-0.98078528040323043,-0.95694033573220894,
    -0.92387953251128685,-0.88192126434835505,-0.83146961230254546,-0.7730104533627371,
    -0.70710678118654768,-0.63439328416364582,-0.55557023301960218,-0.47139673682599792,
    -0.38268343236509034,-0.29028467725446244,-0.19509032201612866,-0.098017140329560451,
    -1.8369701987210297e-16,0.09801714032956009,0.1950903220161283,0.29028467725446205,
    0.38268343236509,0.47139673682599759,0.55557023301960184,0.6343932841636456,
    0.70710678118654735,0.77301045336273666,0.83146961230254524,0.88192126434835483,
    0.92387953251128652,0.95694033573220882,0.98078528040323032,0.99518472667219693
  };
  using Complex=std::complex<double>;
  double source[2][N]{};
  const double gain=drive_==0?1:std::pow(10.,drive_/20.),den=drive_==0?1:std::tanh(gain),blend=std::min(1.,drive_/6.);
  for(int n=0;n<N;++n){const double sample[2]={input[n].left,input[n].right};for(int ch=0;ch<2;++ch){const double dry=sample[ch];source[ch][n]=drive_==0?dry:dry+blend*(std::tanh(dry*gain)/den-dry);}}
  double state[2][6]{};
  // TPT integrator states BEFORE the sample; the third value is current LP output.
  auto svf=[](Complex u,Complex r,double g,double k){
    const double a=1/(1+g*(g+k)),b=2*a*g,c=2*a*g*g;
    const Complex m11=r-(2*a-1),m22=r-(1-c),det=m11*m22+b*b;
    const Complex x1=(b*m22-b*c)/det*u,x2=(m11*c+b*b)/det*u;
    return std::array<Complex,3>{x1,x2,a*g*x1+(1-a*g*g)*x2+a*g*g*u};
  };
  for(int h=1;h<=H;++h){const double w=6.2831853071795864769*fundamental*h/rate_;if(w>=3.14159265358979323846)break;
    const Complex r{std::cos(w),std::sin(w)};
    // Both channels share the same transfer; compute it once per harmonic.
    const Complex dcIn=1./r,dcOut=(1.-1./r)/(r-dcPole_);
    const Complex u=1.+blend*(-dcIn+dcPole_*dcOut);
    const auto first=svf(u,r,g_,k_),second=svf(first[2],r,g_,2.);
    const Complex transfer[6]={dcIn,dcOut,first[0],first[1],second[0],second[1]};
    Complex coefficients[2]{};
    for(int n=0;n<N;++n){const int idx=(h*n)&63;
      const Complex basis(cosine[idx],-cosine[(idx+48)&63]);
      coefficients[0]+=source[0][n]*basis;coefficients[1]+=source[1][n]*basis;}
    for(int ch=0;ch<2;++ch)for(int i=0;i<6;++i)
      state[ch][i]+=(transfer[i]*coefficients[ch]*(2./N)).real();
  }
  for(int ch=0;ch<2;++ch)for(int i=0;i<6;++i)if(!std::isfinite(state[ch][i])||std::abs(state[ch][i])>16)return false;
  for(int ch=0;ch<2;++ch){dcInput_[ch]=state[ch][0];dcOutput_[ch]=state[ch][1];ic1_[ch]=state[ch][2];ic2_[ch]=state[ch][3];cascade1_[ch]=state[ch][4];cascade2_[ch]=state[ch][5];}
  startupRemaining_=0;return true;
}
}
