// SPDX-License-Identifier: MIT
#include "presets/Library.h"
#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>

void Check(bool ok, const char* why) {
  if (!ok) throw std::runtime_error(why);
}

// Make an external/older payload independently of the encoder's sanitation.
void RawValue(sawstar::StateBytes& bytes, sawstar::ParameterId id, double value) {
  uint64_t bits;
  std::memcpy(&bits, &value, sizeof(bits));
  const size_t offset = 20 + 12 * static_cast<size_t>(id);
  for (size_t i = 0; i < 8; ++i) bytes[offset + i] = static_cast<uint8_t>(bits >> (8 * i));
}

int main() {
  using namespace sawstar;
  fs::path root;
  try {
    auto bytes = EncodeState(DefaultSnapshot());
    RawValue(bytes, ParameterId::VoiceMode, 1.5);
    RawValue(bytes, ParameterId::Osc1Wave, 2.4);
    RawValue(bytes, ParameterId::Osc2Octave, -.5);
    RawValue(bytes, ParameterId::ArpRate, 3.7);
    RawValue(bytes, ParameterId::AmpSustain, .123456789123);
    RawValue(bytes, ParameterId::BendRange, 2.3456789);
    Snapshot loaded{};
    Check(DecodeState(bytes.data(), bytes.size(), loaded) == bytes.size(), "External fractional preset rejected");
    Check(loaded[59] == 2 && loaded[33] == 2 && loaded[24] == -1 && loaded[85] == 4,
          "External discrete preset values disagree with framework values");
    Check(loaded[3] == .123456789123 && loaded[17] == 2.3456789,
          "Continuous preset precision changed");

    // Model the GUI round-trip that previously immediately added a modified '*'.
    Snapshot applied{};
    for (size_t i = 0; i < applied.size(); ++i)
      applied[i] = Denormalize(kParameters[i], Normalize(kParameters[i], loaded[i]));
    Check(SnapshotsMatch(loaded, applied), "Freshly loaded preset appears modified");
    auto edited = applied; edited[59] = 1;
    Check(!SnapshotsMatch(loaded, edited), "Real discrete control edit is hidden");
    auto trailer = std::vector<uint8_t>(bytes.begin(), bytes.end()); trailer.resize(trailer.size() + 4);
    Snapshot host{};
    Check(DecodeState(trailer.data(), trailer.size(), host) == bytes.size() && host == loaded,
          "Host bypass trailer compatibility changed");

    for (const auto& spec : kParameters) {
      if (!spec.discrete) continue;
      Check(std::round(spec.minimum) == spec.minimum && std::round(spec.maximum) == spec.maximum &&
            std::round(spec.initial) == spec.initial, "Invalid discrete parameter metadata");
      for (int integer = static_cast<int>(spec.minimum); integer < spec.maximum; ++integer) {
        const double halfway = integer + .5;
        for (double value : {std::nextafter(halfway, -INFINITY), halfway,
                             std::nextafter(halfway, INFINITY)}) {
          const double expected = std::round(value);
          const double actual = Sanitize(spec, value);
          Check(actual == expected && Sanitize(spec, actual) == actual,
                "Discrete boundary rounding or idempotence differs from framework");
        }
      }
      Check(Sanitize(spec, spec.minimum - 1000) == spec.minimum &&
            Sanitize(spec, spec.maximum + 1000) == spec.maximum, "Discrete clamp changed");
      Check(Sanitize(spec, NAN) == spec.initial && Sanitize(spec, INFINITY) == spec.initial,
            "Non-finite fallback changed");
    }

    root = fs::temp_directory_path() / ("sawstar-canonical-" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(root);
    const auto external = root / "External.sawstar";
    { std::ofstream out(external, std::ios::binary); out.write(
        reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())); }
    Check(ReadUserPreset(external) == loaded, "File load bypasses canonical state decode");
    auto noncanonical = loaded; noncanonical[59] = 1.5; noncanonical[24] = -.5;
    const auto selection = SavePresetSelection(root / "Saved.sawstar", noncanonical, root);
    Check(selection.active && selection.saved == ReadUserPreset(selection.path) &&
          selection.saved == loaded, "Saved preset selection disagrees with saved file");
    Check(EncodeState(selection.saved) == EncodeState(loaded), "Canonical re-encoding is unstable");
    fs::remove_all(root);
    std::cout << "Discrete preset canonicalization and clean selection passed.\n";
  } catch (const std::exception& e) {
    if (!root.empty()) fs::remove_all(root);
    std::cerr << e.what() << '\n'; return 1;
  }
}
