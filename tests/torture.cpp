// SPDX-License-Identifier: MIT
#include "SynthTestRig.h"
#include "plugin/Parameters.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct Rng {
  explicit Rng(uint64_t seed) : state(seed ? seed : 0x9e3779b97f4a7c15ULL) {}
  uint64_t Next() {
    uint64_t x = state;
    x ^= x >> 12; x ^= x << 25; x ^= x >> 27;
    state = x;
    return x * 2685821657736338717ULL;
  }
  uint64_t Index(uint64_t n) { return n ? Next() % n : 0; }
  double Unit() { return static_cast<double>(Next() >> 11) * (1.0 / 9007199254740992.0); }
  bool Chance(uint64_t n, uint64_t d) { return Index(d) < n; }
  uint64_t state;
};

struct EventLog {
  static constexpr size_t Capacity = 32;
  std::array<std::string, Capacity> items{};
  size_t next = 0, count = 0;
  void Push(std::string s) {
    items[next] = std::move(s);
    next = (next + 1) % Capacity;
    count = std::min(count + 1, Capacity);
  }
  void Dump(std::ostream& out) const {
    const size_t start = (next + Capacity - count) % Capacity;
    for(size_t i = 0; i < count; ++i) out << "  " << items[(start + i) % Capacity] << '\n';
  }
};

struct Options {
  uint64_t seed = 0x53415753544152ULL;
  uint64_t steps = 10000;
  std::vector<float> rates{44100.f, 48000.f, 96000.f};
  bool verbose = false;
};

bool IsDiscrete(sawstar::ParameterId id) {
  using P = sawstar::ParameterId;
  switch(id) {
    case P::Osc2Octave: case P::SubOctave: case P::NoiseType: case P::Osc1Octave:
    case P::FilterMode: case P::Osc1Wave: case P::Osc2Wave:
    case P::LfoShape: case P::LfoTarget: case P::LfoSync: case P::LfoDivision: case P::LfoRetrigger:
    case P::ChorusEnabled: case P::DelayEnabled: case P::DelayMode: case P::DelaySync: case P::DelayDivision:
    case P::ReverbEnabled: case P::VoiceMode: case P::GlideMode: case P::NoiseSource:
    case P::Lfo2Shape: case P::Lfo2Target: case P::Lfo2Sync: case P::Lfo2Division: case P::Lfo2Retrigger:
    case P::Mod1Source: case P::Mod1Target: case P::Mod2Source: case P::Mod2Target:
    case P::Mod3Source: case P::Mod3Target: case P::Mod4Source: case P::Mod4Target:
    case P::ArpEnabled: case P::ArpMode: case P::ArpRate: case P::ArpOctaves: case P::ArpHold:
    case P::WidthEnabled: case P::SubWave:
      return true;
    default:
      return false;
  }
}

void MutateParameter(sawstar::Snapshot& values, Rng& rng, EventLog& log) {
  const size_t id = static_cast<size_t>(rng.Index(sawstar::kParameters.size()));
  const auto& spec = sawstar::kParameters[id];
  double value = spec.initial;
  if(IsDiscrete(spec.id)) {
    const int lo = static_cast<int>(std::ceil(spec.minimum));
    const int hi = static_cast<int>(std::floor(spec.maximum));
    value = lo + static_cast<int>(rng.Index(static_cast<uint64_t>(hi - lo + 1)));
  } else {
    switch(rng.Index(6)) {
      case 0: value = spec.minimum; break;
      case 1: value = spec.maximum; break;
      case 2: value = spec.initial; break;
      case 3: value = spec.minimum + (spec.maximum - spec.minimum) * .25; break;
      case 4: value = spec.minimum + (spec.maximum - spec.minimum) * .75; break;
      default: value = spec.minimum + (spec.maximum - spec.minimum) * rng.Unit(); break;
    }
  }
  values[id] = value;
  std::ostringstream s;
  s << "param " << id << " " << spec.key << '=' << std::setprecision(8) << value;
  log.Push(s.str());
}

void SendMidi(Rig& rig, int status, int a, int b, EventLog& log, const char* tag) {
  rig.midi(status, a, b);
  std::ostringstream s;
  s << tag << " status=0x" << std::hex << status << std::dec << " a=" << a << " b=" << b;
  log.Push(s.str());
}

void RandomEvent(Rig& rig, sawstar::Snapshot& values, Rng& rng, EventLog& log, float rate) {
  const int ch = static_cast<int>(rng.Index(16));
  const int note = static_cast<int>(rng.Index(128));
  const int velocity = 1 + static_cast<int>(rng.Index(127));
  switch(rng.Index(13)) {
    case 0:
    case 1:
      SendMidi(rig, 0x90 | ch, note, velocity, log, "note-on");
      break;
    case 2:
      SendMidi(rig, 0x80 | ch, note, static_cast<int>(rng.Index(128)), log, "note-off");
      break;
    case 3: {
      const int repeats = 1 + static_cast<int>(rng.Index(4));
      for(int i = 0; i < repeats; ++i) SendMidi(rig, 0x90 | ch, note, velocity, log, "repeat-on");
      const int offs = static_cast<int>(rng.Index(static_cast<uint64_t>(repeats + 1)));
      for(int i = 0; i < offs; ++i) SendMidi(rig, 0x80 | ch, note, 0, log, "repeat-off");
      break;
    }
    case 4:
      SendMidi(rig, 0xB0 | ch, 64, rng.Chance(1, 2) ? 127 : 0, log, "sustain");
      break;
    case 5:
      SendMidi(rig, 0xB0 | ch, 120, 0, log, "cc120");
      break;
    case 6:
      SendMidi(rig, 0xB0 | ch, 121, 0, log, "cc121");
      break;
    case 7:
      SendMidi(rig, 0xB0 | ch, 123, 0, log, "cc123");
      break;
    case 8:
      SendMidi(rig, 0xB0 | ch, 1, static_cast<int>(rng.Index(128)), log, "mod-wheel");
      break;
    case 9: {
      const int bend = static_cast<int>(rng.Index(16384));
      SendMidi(rig, 0xE0 | ch, bend & 127, bend >> 7, log, "pitch-bend");
      break;
    }
    case 10:
      SendMidi(rig, 0xD0 | ch, static_cast<int>(rng.Index(128)), 0, log, "pressure");
      break;
    case 11: {
      const int count = 2 + static_cast<int>(rng.Index(8));
      for(int i = 0; i < count; ++i) MutateParameter(values, rng, log);
      break;
    }
    default:
      rig.init(rate);
      rig.apply(values);
      log.Push("engine-reset");
      break;
  }
}

[[noreturn]] void Fail(const char* why, uint64_t seed, float rate, uint64_t step,
                       int blockSize, const EventLog& log) {
  std::cerr << "SAWSTAR TORTURE FAILURE: " << why
            << "\nseed=0x" << std::hex << seed << std::dec
            << " rate=" << rate << " step=" << step << " block=" << blockSize
            << "\nlast events:\n";
  log.Dump(std::cerr);
  std::exit(1);
}

void HashFloat(uint64_t& hash, float value) {
  uint32_t bits = 0;
  static_assert(sizeof(bits) == sizeof(value), "unexpected float size");
  std::memcpy(&bits, &value, sizeof(bits));
  hash ^= bits;
  hash *= 1099511628211ULL;
}

void ValidateSample(Rig& rig, const sawstar::StereoSample& value, uint64_t seed, float rate,
                    uint64_t step, int blockSize, const EventLog& log,
                    uint64_t& hash, float& peak) {
  const auto pre = rig.synth.PreFX();
  if(!std::isfinite(value.left) || !std::isfinite(value.right) ||
     !std::isfinite(pre.left) || !std::isfinite(pre.right))
    Fail("non-finite audio", seed, rate, step, blockSize, log);
  if(std::abs(value.left) > 1.00001f || std::abs(value.right) > 1.00001f)
    Fail("output exceeded safety bound", seed, rate, step, blockSize, log);
  peak = std::max({peak, std::abs(value.left), std::abs(value.right)});
  HashFloat(hash, value.left); HashFloat(hash, value.right);
}

void GlobalPanic(Rig& rig, uint64_t seed, float rate, uint64_t step,
                 int blockSize, EventLog& log, uint64_t& hash, float& peak) {
  log.Push("global-panic");
  for(int ch = 0; ch < 16; ++ch) rig.midi(0xB0 | ch, 120, 0);
  for(int i = 0; i < 4; ++i) {
    const auto value = rig.process();
    ValidateSample(rig, value, seed, rate, step, blockSize, log, hash, peak);
    if(rig.synth.ActiveVoices() != 0)
      Fail("active voice survived global CC120", seed, rate, step, blockSize, log);
    if(std::abs(value.left) > 1.e-6f || std::abs(value.right) > 1.e-6f)
      Fail("global CC120 left audible output", seed, rate, step, blockSize, log);
  }
}

void RunScenario(float rate, uint64_t seed, uint64_t steps, bool verbose) {
  static constexpr std::array<int, 17> blockSizes{{1,2,7,16,31,32,63,64,65,127,128,255,256,511,512,1024,2048}};
  Rig rig;
  rig.init(rate);
  auto values = sawstar::DefaultSnapshot();
  rig.apply(values);
  Rng rng(seed);
  EventLog log;
  uint64_t hash = 1469598103934665603ULL;
  uint64_t samples = 0;
  float peak = 0;

  for(uint64_t step = 0; step < steps; ++step) {
    const int block = blockSizes[static_cast<size_t>(rng.Index(blockSizes.size()))];
    if(rng.Chance(2, 5)) MutateParameter(values, rng, log);
    if(rng.Chance(1, 12)) {
      values[static_cast<size_t>(sawstar::ParameterId::VoiceMode)] = static_cast<double>(rng.Index(3));
      values[static_cast<size_t>(sawstar::ParameterId::ArpEnabled)] = static_cast<double>(rng.Index(2));
      values[static_cast<size_t>(sawstar::ParameterId::ArpHold)] = static_cast<double>(rng.Index(2));
      log.Push("targeted-mode-arp-retarget");
    }

    // Match the plugin wrapper: parameter targets are delivered once per block.
    rig.apply(values);
    const int eventAt = static_cast<int>(rng.Index(static_cast<uint64_t>(block)));
    for(int i = 0; i < block; ++i) {
      if(i == eventAt) {
        RandomEvent(rig, values, rng, log, rate);
        if(rng.Chance(1, 5)) RandomEvent(rig, values, rng, log, rate);
      }
      const auto value = rig.process();
      ValidateSample(rig, value, seed, rate, step, block, log, hash, peak);
      ++samples;
    }

    const int active = rig.synth.ActiveVoices();
    if(active < 0 || active > 16)
      Fail("ActiveVoices outside 0..16", seed, rate, step, block, log);
    if(rng.Chance(1, 257)) {
      const int probe = static_cast<int>(rng.Index(128));
      (void)rig.synth.Held(probe);
    }
    if(step && step % 509 == 0)
      GlobalPanic(rig, seed, rate, step, block, log, hash, peak);
  }

  GlobalPanic(rig, seed, rate, steps, 0, log, hash, peak);
  std::cout << "torture ok rate=" << rate
            << " seed=0x" << std::hex << seed << std::dec
            << " steps=" << steps << " samples=" << samples
            << " peak=" << peak << " hash=0x" << std::hex << hash << std::dec << '\n';
  if(verbose) log.Dump(std::cout);
}

Options Parse(int argc, char** argv) {
  Options o;
  for(int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    auto require = [&](const char* name) -> const char* {
      if(i + 1 >= argc) throw std::runtime_error(std::string("missing value for ") + name);
      return argv[++i];
    };
    if(arg == "--seed") o.seed = std::stoull(require("--seed"), nullptr, 0);
    else if(arg == "--steps") o.steps = std::stoull(require("--steps"), nullptr, 0);
    else if(arg == "--sample-rate") {
      const double sr = std::stod(require("--sample-rate"));
      if(!std::isfinite(sr) || sr < 8000. || sr > 384000.) throw std::runtime_error("sample rate must be 8000..384000");
      o.rates = {static_cast<float>(sr)};
    } else if(arg == "--verbose") o.verbose = true;
    else if(arg == "--help") {
      std::cout << "SAWSTAR torture test\n"
                   "  --seed N          deterministic seed (decimal or 0x...)\n"
                   "  --steps N         blocks per sample rate (default 10000)\n"
                   "  --sample-rate Hz  test one rate instead of 44.1/48/96 kHz\n"
                   "  --verbose         dump final event history\n";
      std::exit(0);
    } else throw std::runtime_error("unknown argument: " + arg);
  }
  if(o.steps == 0) throw std::runtime_error("--steps must be greater than zero");
  return o;
}

} // namespace

int main(int argc, char** argv) {
  try {
    const auto options = Parse(argc, argv);
    for(float rate : options.rates) {
      const uint64_t derived = options.seed ^
        (static_cast<uint64_t>(std::llround(rate * 100.0)) * 0x9e3779b97f4a7c15ULL);
      RunScenario(rate, derived, options.steps, options.verbose);
    }
    return 0;
  } catch(const std::exception& e) {
    std::cerr << "SAWSTAR torture setup error: " << e.what() << '\n';
    return 2;
  }
}
