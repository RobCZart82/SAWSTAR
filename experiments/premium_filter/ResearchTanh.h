// SPDX-License-Identifier: MIT
#pragma once
#include <cmath>

namespace sawstar::experimental {
// Scalar research only. Not used by PremiumDrive or the production engine.
// Algebraic tanh using one exp; a tiny-input expansion avoids cancellation.
inline double ResearchTanh(double x) {
  const double a = std::abs(x);
  if (a < 1e-5) return x * (1. - x * x / 3.);
  if (a >= 20.) return std::copysign(1., x);
  const double q = std::exp(-2. * a);
  return std::copysign((1. - q) / (1. + q), x);
}
}
