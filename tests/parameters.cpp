// SPDX-License-Identifier: MIT
#include "plugin/Parameters.h"
#include "gui/Pages.h"
#include <cmath>
#include <iostream>
#include <limits>

int main() {
  int failures = 0;
  auto check = [&](bool ok, const char* name) {
    if (!ok) { std::cerr << name << '\n'; ++failures; }
  };
  auto near = [](double a, double b) { return std::abs(a-b) <= 1.e-9 * (1. + std::abs(b)); };
  using namespace sawstar;
  // These IDs/keys are compatibility fixtures, independent of enum ordering.
  const std::array<std::string_view, 19> keys{
    "output.gain_db", "amp.attack_ms", "amp.decay_ms", "amp.sustain", "amp.release_ms", "saw.detune_cents", "saw.mix", "saw.width", "filter.cutoff_hz", "filter.resonance", "filter.mix", "filter.env_amount_st", "filter.key_track", "filter.attack_ms", "filter.decay_ms", "filter.sustain", "filter.release_ms", "performance.bend_range_st", "performance.mod_depth_st"};
  for (std::uint32_t id = 0; id < keys.size(); ++id) {
    const auto* spec = FindParameter(id);
    check(spec && spec->key == keys[id], "persisted parameter identity changed");
  }
  check(FindParameter(999) == nullptr, "unknown ID must fail lookup");
  for (const auto& spec : kParameters) {
    check(spec.minimum < spec.maximum, "range must be nonempty");
    check(spec.initial >= spec.minimum && spec.initial <= spec.maximum, "default outside range");
    check(near(Denormalize(spec, -1.), spec.minimum), "lower clamp");
    check(near(Denormalize(spec, 2.), spec.maximum), "upper clamp");
    check(Sanitize(spec, std::numeric_limits<double>::quiet_NaN()) == spec.initial, "NaN fallback");
    check(Denormalize(spec, std::numeric_limits<double>::infinity()) == spec.initial, "infinity fallback");
    for (int i = 0; i <= 100; ++i) {
      const auto normalized = i / 100.;
      check(near(Normalize(spec, Denormalize(spec, normalized)), normalized), "automation round trip");
    }
  }
  check(near(Denormalize(kParameters[1], 0.5), 100.), "time midpoint must be geometric");
  check(near(Denormalize(kParameters[0], 0.5), -30.), "output midpoint must be linear dB");
  check(gui::kPages[0].title == "MAIN" && gui::kPages[1].title == "ADVANCED" &&
        gui::kPages[2].title == "PRESETS", "page order contract");
  return failures == 0 ? 0 : 1;
}
