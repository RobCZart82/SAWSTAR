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

All eight are continuous and intended to be host-automatable. Sustain is stored
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
payload is 96 bytes; the total state is 112 bytes. Old five-record v1 states
(76 bytes) remain supported and use defaults for IDs 5–7. iPlug2 VST3 appends its own
4-byte bypass value, which is not part of the SAWSTAR payload.

Decode into a temporary snapshot. Bad magic/version, truncated lengths,
duplicate known IDs and non-finite known values fail without changing parameters.
Missing known IDs use defaults, unknown IDs are skipped, finite values clamp.
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
