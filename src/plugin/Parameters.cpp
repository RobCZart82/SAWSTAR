// SPDX-License-Identifier: MIT
#include "plugin/Parameters.h"
#include <algorithm>
#include <cmath>

namespace sawstar {
const ParameterSpec* FindParameter(std::uint32_t id) noexcept {
  for (const auto& spec : kParameters)
    if (static_cast<std::uint32_t>(spec.id) == id) return &spec;
  return nullptr;
}
double Sanitize(const ParameterSpec& spec, double value) noexcept {
  return std::isfinite(value) ? std::clamp(value, spec.minimum, spec.maximum) : spec.initial;
}
double Normalize(const ParameterSpec& spec, double physical) noexcept {
  const auto value = Sanitize(spec, physical);
  if (spec.mapping == Mapping::Logarithmic)
    return std::log(value / spec.minimum) / std::log(spec.maximum / spec.minimum);
  return (value - spec.minimum) / (spec.maximum - spec.minimum);
}
double Denormalize(const ParameterSpec& spec, double normalized) noexcept {
  if (!std::isfinite(normalized)) return spec.initial;
  const auto value = std::clamp(normalized, 0., 1.);
  if (spec.mapping == Mapping::Logarithmic)
    return spec.minimum * std::pow(spec.maximum / spec.minimum, value);
  return spec.minimum + value * (spec.maximum - spec.minimum);
}
} // namespace sawstar
