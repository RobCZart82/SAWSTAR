// SPDX-License-Identifier: MIT
#pragma once
#include <algorithm>
#include <cmath>
namespace sawstar {
template<class T> inline T FiniteClamp(T value,T low,T high,T fallback){return std::isfinite(value)?std::clamp(value,low,high):fallback;}
inline float SafeSampleRate(double rate){return static_cast<float>(FiniteClamp(rate,8000.,384000.,44100.));}
}
