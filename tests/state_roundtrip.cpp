// SPDX-License-Identifier: MIT
#include "plugin/Parameters.h"
#include "plugin/State.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

void Check(bool condition, const std::string& message)
{
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    std::exit(1);
  }
}

bool Near(double a, double b)
{
  const double tolerance =
    1.e-9 * std::max({1.0, std::abs(a), std::abs(b)});

  return std::abs(a - b) <= tolerance;
}

void CheckSnapshotNear(
  const sawstar::Snapshot& actual,
  const sawstar::Snapshot& expected,
  const char* context)
{
  for (size_t i = 0; i < actual.size(); ++i) {
    if (!Near(actual[i], expected[i])) {
      std::cerr
        << "FAIL: " << context
        << ", parameter index " << i
        << ", expected " << expected[i]
        << ", actual " << actual[i]
        << '\n';
      std::exit(1);
    }
  }
}

sawstar::Snapshot MakePhysicalFixture(double normalized)
{
  sawstar::Snapshot values{};

  for (size_t i = 0; i < values.size(); ++i) {
    const auto& spec = sawstar::kParameters[i];
    values[i] = sawstar::Denormalize(spec, normalized);
  }

  return values;
}

void TestPhysicalStateRoundtrip()
{
  const double fixtures[] = {
    0.0,
    0.125,
    0.25,
    0.5,
    0.75,
    0.875,
    1.0
  };

  for (const double normalized : fixtures) {
    const auto input = MakePhysicalFixture(normalized);
    const auto encoded = sawstar::EncodeState(input);

    sawstar::Snapshot decoded{};
    const auto consumed =
      sawstar::DecodeState(encoded.data(), encoded.size(), decoded);

    Check(
      consumed == encoded.size(),
      "complete state payload must be consumed");

    CheckSnapshotNear(
      decoded,
      input,
      "physical state roundtrip");
  }
}

void TestParameterBoundaryRoundtrip()
{
  for (size_t i = 0; i < sawstar::kParameters.size(); ++i) {
    const auto& spec = sawstar::kParameters[i];

    const double values[] = {
      spec.minimum,
      spec.initial,
      (spec.minimum + spec.maximum) * 0.5,
      spec.maximum
    };

    for (double physical : values) {
      physical = sawstar::Sanitize(spec, physical);

      const double normalized =
        sawstar::Normalize(spec, physical);

      const double restored =
        sawstar::Denormalize(spec, normalized);

      Check(
        std::isfinite(normalized),
        "normalized parameter must be finite");

      Check(
        std::isfinite(restored),
        "denormalized parameter must be finite");

      Check(
        Near(restored, physical),
        "parameter normalize/denormalize roundtrip");
    }
  }
}

void TestOutOfRangeValuesAreClamped()
{
  auto input = sawstar::DefaultSnapshot();

  for (size_t i = 0; i < input.size(); ++i) {
    const auto& spec = sawstar::kParameters[i];

    input[i] = spec.maximum + 100000.0;
    input[i] = sawstar::Sanitize(spec, input[i]);

    Check(
      Near(input[i], spec.maximum),
      "upper out-of-range value must clamp to maximum");

    input[i] = spec.minimum - 100000.0;
    input[i] = sawstar::Sanitize(spec, input[i]);

    Check(
      Near(input[i], spec.minimum),
      "lower out-of-range value must clamp to minimum");
  }
}

void TestNonFiniteValuesAreRejectedOrDefaulted()
{
  auto input = sawstar::DefaultSnapshot();

  for (size_t i = 0; i < input.size(); ++i) {
    const auto& spec = sawstar::kParameters[i];

    Check(
      Near(sawstar::Sanitize(
        spec,
        std::numeric_limits<double>::quiet_NaN()),
        spec.initial),
      "NaN must fall back to parameter initial value");

    Check(
      Near(sawstar::Sanitize(
        spec,
        std::numeric_limits<double>::infinity()),
        spec.initial),
      "positive infinity must fall back to parameter initial value");

    Check(
      Near(sawstar::Sanitize(
        spec,
        -std::numeric_limits<double>::infinity()),
        spec.initial),
      "negative infinity must fall back to parameter initial value");
  }
}

void TestOptionalBypassTrailer()
{
  const auto input = MakePhysicalFixture(0.5);
  const auto encoded = sawstar::EncodeState(input);

  std::vector<uint8_t> withTrailer(
    encoded.begin(),
    encoded.end());

  withTrailer.push_back(0);
  withTrailer.push_back(0);
  withTrailer.push_back(0);
  withTrailer.push_back(0);

  sawstar::Snapshot decoded{};

  const auto consumed =
    sawstar::DecodeState(
      withTrailer.data(),
      withTrailer.size(),
      decoded);

  Check(
    consumed == encoded.size(),
    "optional bypass trailer must not be included in consumed size");

  CheckSnapshotNear(
    decoded,
    input,
    "state roundtrip with bypass trailer");
}

void TestOutputRemainsUntouchedOnFailure()
{
  const auto input = MakePhysicalFixture(0.5);
  const auto encoded = sawstar::EncodeState(input);

  sawstar::Snapshot output{};
  output.fill(12345.6789);

  std::vector<uint8_t> truncated(
    encoded.begin(),
    encoded.begin() + 20);

  const auto consumed =
    sawstar::DecodeState(
      truncated.data(),
      truncated.size(),
      output);

  Check(
    consumed == 0,
    "truncated state must be rejected");

  for (double value : output) {
    Check(
      value == 12345.6789,
      "output must remain untouched after decode failure");
  }
}

} // namespace

int main()
{
  TestPhysicalStateRoundtrip();
  TestParameterBoundaryRoundtrip();
  TestOutOfRangeValuesAreClamped();
  TestNonFiniteValuesAreRejectedOrDefaulted();
  TestOptionalBypassTrailer();
  TestOutputRemainsUntouchedOnFailure();

  std::cout << "Full state roundtrip passed.\n";
  return 0;
}
