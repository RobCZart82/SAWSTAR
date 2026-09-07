# Parameter and state contract

The canonical implementation is `src/plugin/Parameters.h`.
IDs below are the intended iPlug2 parameter indices and must remain stable
in all future builds. Append new parameters; never reorder, reuse
or remove an ID. Retired parameters retain their slot and compatibility code.
The strings are stable preset keys. GUI control tags are a separate namespace.

| ID | Stable key | Display | Unit | Min | Max | Default | Mapping |
| -- | ---------- | ------- | ---- | --- | --- | ------- | ------- |
| 0 | output.gain_db | Output | dB | -60 | 0 | -12 | Linear dB |
| 1 | amp.attack_ms | Attack | ms | 1 | 10000 | 10 | Log |
| 2 | amp.decay_ms | Decay | ms | 1 | 10000 | 100 | Log |
| 3 | amp.sustain | Sustain | ratio | 0 | 1 | 0.7 | Linear |
| 4 | amp.release_ms | Release | ms | 1 | 10000 | 250 | Log |
| 5 | saw.detune_cents | Detune | cents | 0 | 50 | 20 | Linear |
| 6 | saw.mix | Mix | % | 0 | 100 | 0 | Linear |
| 7 | saw.width | Width | % | 0 | 100 | 75 | Linear |
| 8 | filter.cutoff_hz | Cutoff | Hz | 20 | 20000 | 12000 | Log |
| 9 | filter.resonance | Resonance | % | 0 | 100 | 0 | Linear |
| 10 | filter.mix | Filter Mix | % | 0 | 100 | 0 | Linear |

| 11 | filter.env_amount_st | Env Amount | st | -96 | 96 | 0 | Linear |
| 12 | filter.key_track | Key Track | % | 0 | 100 | 0 | Linear |
| 13 | filter.attack_ms | F Attack | ms | 1 | 10000 | 10 | Log |
| 14 | filter.decay_ms | F Decay | ms | 1 | 10000 | 200 | Log |
| 15 | filter.sustain | F Sustain | ratio | 0 | 1 | 0 | Linear |
| 16 | filter.release_ms | F Release | ms | 1 | 10000 | 250 | Log |

| 17 | performance.bend_range_st | Bend Range | st | 0 | 24 | 2 | Linear |
| 18 | performance.mod_depth_st | Mod Depth | st | 0 | 48 | 24 | Linear |

All nineteen are continuous and intended to be host-automatable. Sustain is stored
as a ratio and currently displayed as a ratio in the GUI. Output gain becomes
`pow(10, dB/20)` in DSP; -60 dB is not a mute switch. Milliseconds become seconds
at the engine boundary. Parameter conversion utilities clamp finite values
and replace NaN/infinity with the default. Unknown IDs fail lookup.

Host normalized values use [0,1]. Log time mapping is
`min * pow(max/min, normalized)`. The iPlug2 adapter must implement this exact
mapping so host automation, GUI and presets agree. Parameter utilities and iPlug2 registration are implemented. Time parameters
use iPlug2 ShapeExp, matching the logarithmic contract.

## State schema v1 (implemented)

`State.h/.cpp` encodes `SAWSTAR\0` (8 bytes), a little-endian u32 version (1),
a little-endian u32 payload length, then records of u32 parameter ID + IEEE-754
little-endian float64 physical value. Up to 64 records are accepted. The current
payload is 504 bytes; the total state is 520 bytes. Old seventeen-record v1
states (220 bytes) default IDs 17–18. Old eleven-record v1
states (148 bytes) default IDs 11–18. Old eight-record v1
states (112 bytes) remain supported and default IDs 8–18. Old five-record v1 states
(76 bytes) remain supported and use defaults for IDs 5–18. iPlug2 VST3 appends its own
4-byte bypass value, which is not part of the SAWSTAR payload.

Decode into a temporary snapshot. Bad magic/version, truncated lengths,
duplicate known IDs and non-finite known values fail without changing parameters.
Missing known IDs use defaults (except historical Level Boost = 0 dB), unknown IDs are skipped, finite values clamp.
The plugin then applies the validated snapshot using the framework parameter
lock and reset hooks. Live oscillator/envelope state is deliberately not saved.

Legacy 0.1.0 projects (five little-endian physical doubles, 40 bytes plus optional
VST3 bypass) migrate on read. New saves always use v1. Older plugin binaries
cannot read new v1 saves; keep the newer plugin when sharing new projects.
Host parameter IDs, mappings and automation lanes are unchanged. Load Init
uses the same parameter defaults via host gestures; its audio values serialize
through the same state codec when the host saves.

Automated tests include legacy byte fixtures, roundtrip, each truncated v1
length, bad version/magic/length, duplicate IDs, NaN, unknown IDs and bypass
trailers. Automation DSP tests change all five parameters at block sizes
1/32/512/2048 and sample rates 44.1/48/96 kHz. This does not claim sample-accurate
parameter automation: parameters are sampled once per block; output is smoothed.

## Source mixer extension

IDs 19–30 are append-only. The authoritative ranges/defaults are in
`src/plugin/Parameters.h`; see [source mixer](SOURCE_MIXER.md) for behavior.

| ID | Key | Range | Default |
| --- | --- | --- | --- |
| 19 | output.boost_db | 0.…24. | 18. |
| 20 | mixer.osc1 | 0.…100. | 100. |
| 21 | mixer.osc2 | 0.…100. | 0. |
| 22 | mixer.sub | 0.…100. | 0. |
| 23 | mixer.noise | 0.…100. | 0. |
| 24 | osc2.octave | -2.…2. | 0. |
| 25 | sub.octave | -2.…0. | -1. |
| 26 | noise.type | 0.…1. | 0. |
| 27 | osc2.detune_cents | 0.…50. | 20. |
| 28 | osc2.mix | 0.…100. | 0. |
| 29 | osc2.width | 0.…100. | 75. |
| 30 | osc1.octave | -2.…2. | 0. |

Absent Level Boost migrates to 0 dB for historical loudness; additional sources
default to silent. Octave and noise selectors are discrete host parameters.

## Filter character extension

| ID | Key | Range | Default |
| --- | --- | --- | --- |
| 31 | filter.drive_db | 0–24 dB | 0 |
| 32 | filter.mode | 0=LP12, 1=LP24, 2=HP12, 3=BP12 | 0 |

Older states default to Drive 0 / LP12. See [filter character](FILTER_CHARACTER.md).

## LFO and waveform extension

| ID | Key | Range / choices | Default |
| --- | --- | --- | --- |
| 33 | osc1.waveform | Saw / Square / Triangle / Sine | Saw |
| 34 | osc2.waveform | Saw / Square / Triangle / Sine | Saw |
| 35 | lfo.rate_hz | 0.05–20 Hz (log) | 1 |
| 36 | lfo.depth | 0–100% | 0 |
| 37 | lfo.shape | Sine / Triangle / Ramp / Square | Sine |
| 38 | lfo.target | Cutoff / Pitch / Amp / Pan | Cutoff |
| 39 | lfo.sync | Free Hz / Tempo Sync | Free Hz |
| 40 | lfo.division | 1/1, 1/2, 1/4, 1/8, 1/16, 1/32 | 1/4 |
| 41 | lfo.retrigger | Free phase / Retrigger first key | Free phase |

See [LFO and waveforms](LFO_WAVEFORMS.md) for route depths and sync semantics.

## Stereo chorus (append-only IDs 42-45)

| ID | Key | Range | Default |
|---|---|---|---|
| 42 | chorus.enabled | Off / On | Off |
| 43 | chorus.mix | 0-100% | 25% |
| 44 | chorus.rate_hz | 0.05-3 Hz | 0.3 Hz |
| 45 | chorus.depth | 0-100% | 35% |

Old states restore chorus Off. See [CHORUS.md](CHORUS.md).

## Delay: append-only IDs 46-53

See [DELAY.md](DELAY.md) for keys, ranges, defaults and tempo behavior. Old states restore Delay Off.

## Reverb: append-only IDs 54-58

See [REVERB.md](REVERB.md) for keys, ranges and defaults. Old states restore Reverb Off.

## Voice modes: append-only IDs 59-61

See [MONO_GLIDE.md](MONO_GLIDE.md). Default Poly / Glide 0 / Overlap only.
