// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#if defined(__SSE2__) || defined(_M_X64)
#include <emmintrin.h>
#define SAWSTAR_FIR_SSE2 1
#elif defined(__aarch64__) && defined(__clang__)
#include <arm_neon.h>
#define SAWSTAR_FIR_NEON 1
#endif

namespace sawstar::experimental::detail {
// Four independent double sums; reduce only as (s0+s1)+(s2+s3).
// newest points to the first (descending) sample, oldest is ascending.
// Every load consumes four valid contiguous doubles. No padding is read.
class FirLanes4 {
public:
#if defined(SAWSTAR_FIR_SSE2)
  static constexpr const char* Backend = "SSE2";
#elif defined(SAWSTAR_FIR_NEON)
  static constexpr const char* Backend = "NEON";
#else
  static constexpr const char* Backend = "scalar-fallback";
#endif
  void Symmetric(const double* taps,const double* newest,const double* oldest) {
#if defined(__clang__)
#pragma clang fp contract(off)
#endif
#if defined(SAWSTAR_FIR_SSE2) || defined(SAWSTAR_FIR_NEON)
    const auto p0=Mul(Load(taps),Add(Reverse(Load(newest-1)),Load(oldest)));
    const auto p1=Mul(Load(taps+2),Add(Reverse(Load(newest-3)),Load(oldest+2)));
    lo_=Add(lo_,p0);hi_=Add(hi_,p1);
#else
    for(unsigned lane=0;lane<4;++lane)
      scalar_[lane]+=taps[lane]*(newest[-int(lane)]+oldest[lane]);
#endif
  }
  void Straight(const double* taps,const double* newest) {
#if defined(__clang__)
#pragma clang fp contract(off)
#endif
#if defined(SAWSTAR_FIR_SSE2) || defined(SAWSTAR_FIR_NEON)
    const auto p0=Mul(Load(taps),Reverse(Load(newest-1)));
    const auto p1=Mul(Load(taps+2),Reverse(Load(newest-3)));
    lo_=Add(lo_,p0);hi_=Add(hi_,p1);
#else
    for(unsigned lane=0;lane<4;++lane)scalar_[lane]+=taps[lane]*newest[-int(lane)];
#endif
  }
  double Sum() const {
    std::array<double,4> x{};
#if defined(SAWSTAR_FIR_SSE2)
    _mm_storeu_pd(x.data(),lo_);_mm_storeu_pd(x.data()+2,hi_);
#elif defined(SAWSTAR_FIR_NEON)
    vst1q_f64(x.data(),lo_);vst1q_f64(x.data()+2,hi_);
#else
    x=scalar_;
#endif
    return (x[0]+x[1])+(x[2]+x[3]);
  }
private:
#if defined(SAWSTAR_FIR_SSE2)
  using Vec=__m128d;
  static Vec Load(const double* p){return _mm_loadu_pd(p);}
  static Vec Reverse(Vec x){return _mm_shuffle_pd(x,x,1);}
  static Vec Add(Vec a,Vec b){return _mm_add_pd(a,b);}
  static Vec Mul(Vec a,Vec b){return _mm_mul_pd(a,b);}
  Vec lo_=_mm_setzero_pd(),hi_=_mm_setzero_pd();
#elif defined(SAWSTAR_FIR_NEON)
  using Vec=float64x2_t;
  static Vec Load(const double* p){return vld1q_f64(p);}
  static Vec Reverse(Vec x){return vextq_f64(x,x,1);}
  static Vec Add(Vec a,Vec b){return vaddq_f64(a,b);}
  static Vec Mul(Vec a,Vec b){return vmulq_f64(a,b);}
  Vec lo_=vdupq_n_f64(0.),hi_=vdupq_n_f64(0.);
#else
  std::array<double,4> scalar_{};
#endif
};
}
#undef SAWSTAR_FIR_SSE2
#undef SAWSTAR_FIR_NEON
