// SPDX-License-Identifier: MIT
#include "presets/Library.h"
#include <chrono>
#include <cstring>
#include <iostream>
#include <limits>

namespace {
using namespace sawstar;
using Bytes = std::vector<uint8_t>;
void Check(bool ok, const char* message) {
  if (!ok) throw std::runtime_error(message);
}
void Put32(Bytes& bytes, size_t pos, uint32_t value) {
  for (size_t i = 0; i < 4; ++i) bytes[pos + i] = uint8_t(value >> (8 * i));
}
void PutDouble(Bytes& bytes, size_t pos, double value) {
  uint64_t bits;
  std::memcpy(&bits, &value, sizeof(bits));
  for (size_t i = 0; i < 8; ++i) bytes[pos + i] = uint8_t(bits >> (8 * i));
}
Bytes Partial(const Bytes& full, size_t records) {
  Bytes bytes(full.begin(), full.begin() + 16 + 12 * records);
  Put32(bytes, 12, uint32_t(12 * records));
  return bytes;
}
void Write(const fs::path& path, const Bytes& bytes) {
  std::ofstream out(path, std::ios::binary);
  out.write(reinterpret_cast<const char*>(bytes.data()), std::streamsize(bytes.size()));
  Check(bool(out), "Fixture write failed");
}
} // namespace

int main() {
  fs::path root;
  try {
    root = fs::temp_directory_path() / ("sawstar-strict-" +
      std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(root);
    auto wanted = DefaultSnapshot(); wanted[0] = -23.5; wanted[8] = 1734.;
    const auto encoded = EncodeState(wanted);
    const Bytes full(encoded.begin(), encoded.end());
    size_t serial = 0;
    auto reject = [&](const Bytes& bytes) {
      auto output = wanted;
      Check(DecodePresetFile(bytes.data(), bytes.size(), output) == 0 && output == wanted,
            "Rejected file changed the output snapshot");
      const auto path = root / ("Bad" + std::to_string(serial++) + ".sawstar");
      Write(path, bytes);
      bool rejected = false;
      try { (void)ReadUserPreset(path); }
      catch (const std::runtime_error&) { rejected = true; }
      Check(rejected, "File loader accepted a malformed or host-only payload");
    };
    auto accept = [&](const Bytes& bytes) {
      Snapshot host{}, file{};
      Check(DecodeState(bytes.data(), bytes.size(), host) == bytes.size(), "Valid host fixture rejected");
      Check(DecodePresetFile(bytes.data(), bytes.size(), file) == bytes.size() && file == host,
            "File policy changed valid sound values");
      const auto path = root / ("Good" + std::to_string(serial++) + ".sawstar");
      Write(path, bytes);
      Check(ReadUserPreset(path) == host, "Valid file differs from decoded state");
      return path;
    };
    const auto good = accept(full);
    // Every historical non-empty versioned prefix remains importable.
    for (size_t records = 1; records <= kParameters.size(); ++records)
      accept(Partial(full, records));
    auto unknown = Partial(full, 1); Put32(unknown, 16, 999);
    auto mixed = full; mixed.resize(mixed.size() + 12);
    Put32(mixed, 12, uint32_t(mixed.size() - 16));
    Put32(mixed, mixed.size() - 12, 999); PutDouble(mixed, mixed.size() - 8, 1.);
    accept(mixed); // Future fields accompany known settings, without changing them.
    auto fractional = full; PutDouble(fractional, 20 + 59 * 12, 1.5);
    accept(fractional);

    reject(Bytes(40, 'A')); reject(Bytes(44, 'A'));
    reject(Partial(full, 0)); reject(unknown);
    auto trailer = full; trailer.insert(trailer.end(), {'A', 'B', 'C', 'D'}); reject(trailer);
    auto legacy = Bytes(40);
    for (size_t i = 0; i < 5; ++i) PutDouble(legacy, 8 * i, wanted[i]);
    reject(legacy); auto legacyTrailer = legacy; legacyTrailer.resize(44); reject(legacyTrailer);
    // Host migration and the wrapper's consumed-byte/trailer contract are intact.
    Snapshot host{};
    for (const auto& bytes : {legacy, legacyTrailer})
      Check(DecodeState(bytes.data(), bytes.size(), host) == 40 && host[0] == wanted[0],
            "Legacy host state compatibility changed");
    Check(DecodeState(trailer.data(), trailer.size(), host) == full.size() && host == wanted,
          "Host bypass trailer compatibility changed");
    for (const auto& bytes : {Partial(full, 0), unknown})
      Check(DecodeState(bytes.data(), bytes.size(), host) == bytes.size(),
            "Permissive future host state compatibility changed");

    auto malformed = full; malformed[8] = 2; reject(malformed);
    malformed = full; malformed[0] = 'X'; reject(malformed);
    malformed = full; Put32(malformed, 28, 0); reject(malformed);
    malformed = full; PutDouble(malformed, 20, std::numeric_limits<double>::quiet_NaN()); reject(malformed);
    malformed = full; PutDouble(malformed, 20, std::numeric_limits<double>::infinity()); reject(malformed);
    malformed = full; Put32(malformed, 12, 3073); reject(malformed);
    for (size_t length = 0; length < full.size(); ++length)
      reject(Bytes(full.begin(), full.begin() + length));
    auto output = wanted;
    Check(!DecodePresetFile(nullptr, full.size(), output) && output == wanted,
          "Null file data changed output");

    // Import skips bad files, copies the valid one, and never auto-plays it.
    const auto bad = root / "Text.sawstar"; Write(bad, Bytes(40, 'A'));
    const auto library = root / "Library";
    const auto report = ImportPresets({bad, good}, library);
    Check(report.failed == 1 && report.imported == 1 && ListUserPresets(library).size() == 1,
          "A rejected file blocked valid imports or entered the library");
    Check(ReadUserPreset(ListUserPresets(library).front()) == wanted,
          "Import changed a valid sound");
    fs::remove_all(root);
    std::cout << "Strict preset files and unchanged host migration passed\n";
  } catch (const std::exception& e) {
    if (!root.empty()) { std::error_code ignored; fs::remove_all(root, ignored); }
    std::cerr << e.what() << '\n'; return 1;
  }
}
