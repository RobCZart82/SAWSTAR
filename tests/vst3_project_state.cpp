// SPDX-License-Identifier: MIT
#include "plugin/State.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <vector>

#define TRACE
[[maybe_unused]] constexpr int kHostWaveLab = 1;

namespace Steinberg {
using int32 = std::int32_t;
using int64 = std::int64_t;
constexpr int32 kResultOk = 0;
constexpr int32 kResultTrue = kResultOk;
constexpr int32 kResultFalse = 1;
constexpr int32 kInternalError = -2;

struct IBStream {
  enum IStreamSeekMode { kIBSeekSet };
  static constexpr auto unlimited = std::numeric_limits<std::size_t>::max();

  std::vector<std::uint8_t> bytes;
  std::size_t position = 0;
  std::size_t readLimit = unlimited;
  std::size_t readErrorOffset = unlimited;
  std::size_t writeLimit = unlimited;
  std::size_t writeErrorOffset = unlimited;
  int32 eof = kResultOk;
  bool reportShortEof = false;
  bool seekable = true;
  int seekCalls = 0;

  int32 read(void* buffer, int32 requested, int32* actual = nullptr) {
    if (actual) *actual = 0;
    if (position >= readErrorOffset) return kInternalError;
    if (position >= bytes.size()) return eof;
    const auto count = std::min({static_cast<std::size_t>(requested), bytes.size() - position,
                                 readLimit, readErrorOffset - position});
    std::memcpy(buffer, bytes.data() + position, count);
    position += count;
    if (actual) *actual = static_cast<int32>(count);
    return reportShortEof && count < static_cast<std::size_t>(requested) && position == bytes.size()
        ? eof : kResultOk;
  }

  int32 write(void* buffer, int32 requested, int32* actual = nullptr) {
    if (actual) *actual = 0;
    if (bytes.size() >= writeErrorOffset) return kInternalError;
    const auto count = std::min({static_cast<std::size_t>(requested), writeLimit - bytes.size(),
                                 writeErrorOffset - bytes.size()});
    const auto* data = static_cast<const std::uint8_t*>(buffer);
    bytes.insert(bytes.end(), data, data + count);
    if (actual) *actual = static_cast<int32>(count);
    return kResultOk;
  }

  int32 seek(int64 target, int32 mode, int64* actual = nullptr) {
    ++seekCalls;
    if (!seekable || mode != kIBSeekSet || target < 0 ||
        static_cast<std::uint64_t>(target) > bytes.size()) return kInternalError;
    position = static_cast<std::size_t>(target);
    if (actual) *actual = target;
    return kResultOk;
  }
};
} // namespace Steinberg

namespace {

void Check(bool condition, const char* message) {
  if (!condition) {
    std::cerr << message << '\n';
    std::exit(1);
  }
}

struct IByteChunk {
  std::vector<std::uint8_t> bytes;

  int PutBytes(const void* source, int count) {
    const auto* data = static_cast<const std::uint8_t*>(source);
    bytes.insert(bytes.end(), data, data + count);
    return Size();
  }

  std::uint8_t* GetData() { return bytes.data(); }
  const std::uint8_t* GetData() const { return bytes.data(); }
  int Size() const { return static_cast<int>(bytes.size()); }
};

struct IPlugVST3State;

struct IPlugProcessor {
  friend struct IPlugVST3State;
  virtual ~IPlugProcessor() = default;
  bool bypassed = false;

  bool GetBypassed() const { return bypassed; }

protected:
  void SetBypassed(bool value) { bypassed = value; }
};

struct IPlugVST3ControllerBase {
  virtual ~IPlugVST3ControllerBase() = default;
  bool controllerBypass = false;
  int updates = 0;

  template<class T>
  void UpdateParams(T*, int bypass) {
    controllerBypass = (bypass != 0);
    ++updates;
  }
};

struct PlugState {
  virtual ~PlugState() = default;
  sawstar::Snapshot sound = sawstar::DefaultSnapshot();
  float scale = 0.f;
  int restores = 0;
  bool serialize = true;

  int GetHost() const { return 0; }
  void OnRestoreState() { ++restores; }

  bool SerializeState(IByteChunk& chunk) const {
    if (!serialize) return false;
    const auto encoded = sawstar::EncodeHostState(sound, scale);
    return chunk.PutBytes(encoded.data(), static_cast<int>(encoded.size())) > 0;
  }

  int UnserializeState(const IByteChunk& chunk, int start) {
    if (start < 0 || start >= chunk.Size()) return -1;
    const auto consumed = sawstar::DecodeHostState(
        chunk.GetData() + start, static_cast<std::size_t>(chunk.Size() - start), sound, scale);
    return consumed ? start + static_cast<int>(consumed) : -1;
  }
};

struct CombinedPlug : PlugState, IPlugProcessor, IPlugVST3ControllerBase {};
struct ProcessorPlug : PlugState, IPlugProcessor {};
struct ControllerPlug : PlugState, IPlugVST3ControllerBase {};

#include "vst3_project_state.inc"

std::vector<std::uint8_t> WithBypass(std::vector<std::uint8_t> state, bool bypassed) {
  state.insert(state.end(), {static_cast<std::uint8_t>(bypassed), 0, 0, 0});
  return state;
}

void CheckRoundTrip(CombinedPlug& saved, const std::vector<std::uint8_t>& payload) {
  for (bool bypassed : {true, false}) {
    saved.bypassed = bypassed;
    Steinberg::IBStream stream;
    Check(IPlugVST3State::GetState(&saved, &stream), "VST3 state save failed");
    Check(stream.bytes == WithBypass(payload, bypassed), "VST3 saved wire format changed");
    stream.readLimit = 17;
    CombinedPlug loaded;
    loaded.bypassed = !bypassed;
    loaded.controllerBypass = !bypassed;
    Check(IPlugVST3State::SetState(&loaded, &stream), "VST3 state restore failed");
    Check(loaded.sound == saved.sound && loaded.scale == saved.scale,
          "VST3 restore lost sound or GUI scale");
    Check(loaded.GetBypassed() == bypassed && loaded.controllerBypass == bypassed,
          "Controller and processor failed to restore the stored bypass");
    Check(loaded.updates == 1 && loaded.restores == 1, "VST3 restore hooks must run once");
    Check(stream.seekCalls == 0, "The state reader requires a seekable host stream");
  }
}

void CheckLegacyRestores(const std::vector<std::uint8_t>& payload) {
  // Legacy 0.1.0 stores the five physical doubles without versioned records.
  constexpr std::array<double, 5> legacyValues{{-32., 42., 250., .5, 272.}};
  std::vector<std::uint8_t> legacy(legacyValues.size() * sizeof(double));
  std::memcpy(legacy.data(), legacyValues.data(), legacy.size());
  for (auto eof : {Steinberg::kResultOk, Steinberg::kResultFalse}) {
    for (const auto& input : {payload, WithBypass(payload, true), legacy, WithBypass(legacy, true)}) {
      Steinberg::IBStream stream;
      stream.bytes = input;
      stream.eof = eof;
      stream.reportShortEof = eof == Steinberg::kResultFalse;
      stream.seekable = false;
      CombinedPlug loaded;
      loaded.bypassed = true;
      loaded.controllerBypass = true;
      Check(IPlugVST3State::SetState(&loaded, &stream), "Legacy or trailer-less state was rejected");
      const bool isLegacy = input.size() <= legacy.size() + 4;
      const bool hasBypass = input.size() == (isLegacy ? legacy.size() : payload.size()) + 4;
      Check(loaded.GetBypassed() == hasBypass && loaded.controllerBypass == hasBypass,
            "Legacy or absent bypass was interpreted incorrectly");
      if (isLegacy) {
        Check(std::equal(legacyValues.begin(), legacyValues.end(), loaded.sound.begin()),
              "Legacy parameter values were lost");
        Check(loaded.scale == 0.f &&
              loaded.sound[static_cast<std::size_t>(sawstar::ParameterId::OutputBoost)] == 0.,
              "Legacy GUI scale or output-level defaults changed");
      } else {
        Check(loaded.sound[static_cast<std::size_t>(sawstar::ParameterId::OutputGain)] == -18.125 &&
              loaded.scale == 1.25f, "Trailer compatibility changed the versioned payload");
      }
      Check(loaded.updates == 1 && loaded.restores == 1,
            "VST3 restore hooks must run once");
    }
  }
}

void CheckDistributedRestore(const std::vector<std::uint8_t>& payload) {
  Steinberg::IBStream stream;
  stream.bytes = WithBypass(payload, true);
  ProcessorPlug processor;
  Check(IPlugVST3State::SetState(&processor, &stream) && processor.GetBypassed(),
        "Processor-only restore lost bypass");
  stream.position = 0;
  ControllerPlug controller;
  Check(IPlugVST3State::SetState(&controller, &stream) && controller.controllerBypass,
        "Controller-only restore lost bypass");
}

void CheckRejectedWrite(CombinedPlug& saved, std::size_t payloadSize) {
  Steinberg::IBStream stream;
  stream.writeErrorOffset = 0;
  Check(!IPlugVST3State::GetState(&saved, &stream) && stream.bytes.empty(),
        "Rejected payload write falsely reported success");
  stream.writeErrorOffset = payloadSize;
  Check(!IPlugVST3State::GetState(&saved, &stream), "Rejected bypass write falsely reported success");
  for (auto limit : {payloadSize - 1, payloadSize + 3}) {
    Steinberg::IBStream shortWrite;
    shortWrite.writeLimit = limit;
    Check(!IPlugVST3State::GetState(&saved, &shortWrite), "Short state write falsely reported success");
  }
  saved.serialize = false;
  Steinberg::IBStream failedSerialization;
  Check(!IPlugVST3State::GetState(&saved, &failedSerialization) && failedSerialization.bytes.empty(),
        "Failed serialization wrote a state");
}

void CheckRejectedRead(Steinberg::IBStream& stream, const char* reason) {
  CombinedPlug loaded;
  loaded.scale = .75f;
  loaded.bypassed = true;
  loaded.controllerBypass = true;
  const auto original = loaded.sound;
  Check(!IPlugVST3State::SetState(&loaded, &stream), reason);
  Check(loaded.sound == original && loaded.scale == .75f &&
        loaded.GetBypassed() && loaded.controllerBypass &&
        loaded.restores == 0 && loaded.updates == 0, "Invalid load partially changed project state");
}

void CheckRejectedReads(const std::vector<std::uint8_t>& payload) {
  Steinberg::IBStream stream;
  stream.bytes = WithBypass(payload, false);
  stream.readErrorOffset = payload.size();
  CheckRejectedRead(stream, "Read error after a valid payload was mistaken for EOF");
  for (std::size_t length : {1, 2, 3, 5}) {
    Steinberg::IBStream incompleteTrailer;
    incompleteTrailer.bytes = payload;
    incompleteTrailer.bytes.resize(payload.size() + length, 0);
    CheckRejectedRead(incompleteTrailer, "Wrong-sized bypass trailer was accepted");
  }
  Steinberg::IBStream invalidBypass;
  invalidBypass.bytes = WithBypass(payload, false);
  invalidBypass.bytes[payload.size()] = 2;
  CheckRejectedRead(invalidBypass, "Unknown bypass value was accepted");
  Steinberg::IBStream invalidVersion;
  invalidVersion.bytes = WithBypass(payload, false);
  invalidVersion.bytes[8] = 2;
  CheckRejectedRead(invalidVersion, "Unknown host state version was accepted");
}

} // namespace

int main() {
  CombinedPlug saved;
  saved.sound[static_cast<std::size_t>(sawstar::ParameterId::OutputGain)] = -18.125;
  saved.scale = 1.25f;
  const auto encoded = sawstar::EncodeHostState(saved.sound, saved.scale);
  const std::vector<std::uint8_t> payload(encoded.begin(), encoded.end());
  CheckRoundTrip(saved, payload);
  CheckLegacyRestores(payload);
  CheckDistributedRestore(payload);
  CheckRejectedWrite(saved, payload.size());
  CheckRejectedReads(payload);
}
