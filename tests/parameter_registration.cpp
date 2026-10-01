// SPDX-License-Identifier: MIT
#include "plugin/Parameters.h"
#include "plugin/State.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

namespace {
void Check(bool condition, const std::string& message) {
  if (!condition) { std::cerr << message << '\n'; std::exit(1); }
}
bool Near(double a, double b) {
  return std::abs(a - b) <= 1.e-10 * std::max({1., std::abs(a), std::abs(b)});
}

// Registration-only recorder. The production loop is extracted unchanged;
// choosing which parameter is stepped is not duplicated in this test.
// Constrain/shape semantics match iPlug2's pinned d54f690 IPlugParameter.
struct IParam {
  enum { kFlagsNone = 0, kFlagStepped = 0x2 };
  struct Shape {
    bool logarithmic;
    explicit Shape(bool exponential = false) : logarithmic(exponential) {}
  };
  struct ShapeExp : Shape { ShapeExp() : Shape(true) {} };
  double minimum = 0, maximum = 0, initial = 0, step = 0, value = 0;
  int flags = 0, registrations = 0;
  bool logarithmic = false;

  void InitDouble(const char*, double defaultValue, double minValue,
                  double maxValue, double stepValue, const char*,
                  int parameterFlags, const char*, const Shape& shape = Shape{}) {
    ++registrations;
    minimum = minValue; maximum = std::max(maxValue, minValue + stepValue);
    initial = defaultValue; step = stepValue; flags = parameterFlags;
    logarithmic = shape.logarithmic; Set(defaultValue);
  }
  void InitInt(const char* name, int defaultValue, int minValue, int maxValue,
               const char* unit, int parameterFlags, const char* group) {
    InitDouble(name, defaultValue, minValue, maxValue, 1., unit,
               parameterFlags | kFlagStepped, group);
  }
  void InitEnum(const char* name, int defaultValue, int count, const char* unit,
                int parameterFlags, const char* group, const char*, ...) {
    InitInt(name, defaultValue, 0, count - 1, unit,
            parameterFlags | kFlagStepped, group);
  }
  bool GetStepped() const { return (flags & kFlagStepped) != 0; }
  double Constrain(double physical) const {
    return std::clamp(GetStepped() ? std::round(physical / step) * step : physical,
                      minimum, maximum);
  }
  double FromNormalized(double normalized) const {
    const double physical = logarithmic
        ? std::exp(std::log(minimum) + normalized * std::log(maximum / minimum))
        : minimum + normalized * (maximum - minimum);
    return Constrain(physical);
  }
  double ToNormalized(double physical) const {
    physical = Constrain(physical);
    return std::clamp(logarithmic
        ? (std::log(physical) - std::log(minimum)) / std::log(maximum / minimum)
        : (physical - minimum) / (maximum - minimum), 0., 1.);
  }
  void Set(double physical) { value = Constrain(physical); }
  void SetNormalized(double normalized) { Set(FromNormalized(normalized)); }
};

struct Registration {
  std::array<IParam, sawstar::kParameters.size()> parameters;
  IParam* GetParam(int id) {
    Check(id >= 0 && size_t(id) < parameters.size(), "Invalid registered ID");
    return &parameters[size_t(id)];
  }
  void Initialize() {
#include "parameter_registration_hook.inc"
  }
};

void SetWireValue(sawstar::StateBytes& bytes, size_t id, double physical) {
  std::uint64_t bits = 0;
  std::memcpy(&bits, &physical, sizeof(bits));
  for (size_t byte = 0; byte < 8; ++byte)
    bytes[20 + id * 12 + byte] = static_cast<std::uint8_t>(bits >> (8 * byte));
}
} // namespace

int main() {
  using namespace sawstar;
  Registration registration; registration.Initialize();
  size_t steppedCount = 0;
  for (size_t id = 0; id < kParameters.size(); ++id) {
    const auto& spec = kParameters[id];
    auto& param = registration.parameters[id];
    const std::string context(spec.key);
    Check(param.registrations == 1, "Registration count changed: " + context);
    Check(param.minimum == spec.minimum && param.maximum == spec.maximum &&
          param.initial == spec.initial, "Registration bounds/default differ: " + context);
    Check(param.GetStepped() == spec.discrete,
          "Persisted discreteness differs from real registration: " + context);
    Check(param.logarithmic == (spec.mapping == Mapping::Logarithmic),
          "Registration mapping differs: " + context);
    if (param.GetStepped()) {
      ++steppedCount;
      Check(param.step == 1., "Unsupported discrete registration step: " + context);
    }
    const double physicalValues[] = {
      spec.minimum - 100., spec.minimum, spec.initial, spec.maximum,
      spec.maximum + 100., spec.minimum + .49, spec.minimum + .5,
      spec.minimum + .51, (spec.minimum + spec.maximum) * .5,
      -1.5, -.5, .5, 1.5, 2.4, 3.7,
    };
    for (double physical : physicalValues) {
      Check(Near(Sanitize(spec, physical), param.Constrain(physical)),
            "Physical sanitization differs from iPlug constraint: " + context);
      Check(Near(Normalize(spec, physical), param.ToNormalized(physical)),
            "Physical normalization differs from iPlug: " + context);
    }
    for (double normalized : {-1., 0., .125, .25, .5, .625, .75, .875, 1., 2.}) {
      Check(Near(Denormalize(spec, normalized), param.FromNormalized(normalized)),
            "Denormalization differs from real registered parameter: " + context);
    }
  }
  Check(steppedCount > 0 && steppedCount < kParameters.size(),
        "Registration fixture lost discrete or continuous coverage");

  // Simulate loading a pre-existing noncanonical preset, not one already
  // canonicalized by the new encoder. Saved values must equal applied values.
  auto bytes = EncodeState(DefaultSnapshot());
  for (size_t id = 0; id < kParameters.size(); ++id) {
    const auto& spec = kParameters[id];
    const double physical = spec.discrete ? spec.minimum + .5
        : spec.minimum + (spec.maximum - spec.minimum) * .123456789;
    SetWireValue(bytes, id, physical);
  }
  Snapshot saved{};
  Check(DecodeState(bytes.data(), bytes.size(), saved) == bytes.size(),
        "Pre-existing fractional preset could not be decoded");
  Snapshot applied{};
  for (size_t id = 0; id < kParameters.size(); ++id) {
    auto& param = registration.parameters[id];
    param.SetNormalized(Normalize(kParameters[id], saved[id]));
    applied[id] = param.value;
  }
  Check(SnapshotsMatch(saved, applied),
        "User preset marked dirty immediately after framework application");
  std::cout << kParameters.size() << " production registrations, " << steppedCount
            << " discrete parameters: canonical preset remains clean\n";
}
