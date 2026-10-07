// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <cmath>
#include <cstddef>

namespace sawstar::experimental {
// Opt-in research only. Init prepares a shared immutable table; Process never
// initializes it, allocates, locks or calls libm tanh/exp. The polynomial is
// cubic Hermite interpolation with analytical endpoint derivatives.
class LookupTanh {
public:
  void Init() { coefficients_ = Table().data(); }
  double Process(double x) const {
    if (std::isnan(x)) return x;
    const double magnitude = std::abs(x);
    if (magnitude < 1e-8) return x;
    if (magnitude >= 16.) return std::copysign(1., x);
    const double position = magnitude * 128.;
    const auto index = static_cast<std::size_t>(position);
    const double t = position - static_cast<double>(index);
    const auto& c = coefficients_[index];
    const double y = ((c[3] * t + c[2]) * t + c[1]) * t + c[0];
    return std::copysign(y, x);
  }
  static constexpr std::size_t TableBytes = 2048 * 4 * sizeof(double);
private:
  using Coefficients = std::array<double, 4>;
  static const std::array<Coefficients, 2048>& Table() {
    static const auto table = [] {
      std::array<Coefficients, 2048> result{};
      constexpr double step = 1. / 128.;
      for (std::size_t i = 0; i < result.size(); ++i) {
        const double a = std::tanh(i * step), b = std::tanh((i + 1) * step);
        const double da = step * (1. - a * a), db = step * (1. - b * b);
        result[i] = {a, da, 3. * (b - a) - 2. * da - db,
                     2. * (a - b) + da + db};
      }
      return result;
    }();
    return table;
  }
  const Coefficients* coefficients_ = nullptr; // Init is required before Process.
};
}
