# Parameter and state contract

The canonical implementation is `src/plugin/Parameters.h`.
IDs below are the intended iPlug2 parameter indices and must remain stable
when host integration is added. Append new parameters; never reorder, reuse
or remove an ID. Retired parameters retain their slot and compatibility code.
The strings are stable preset keys. GUI control tags are a separate namespace.

| ID | Stable key | Display | Unit | Min | Max | Default | Mapping |
| -- | ---------- | ------- | ---- | --- | --- | ------- | ------- |
| 0 | output.gain_db | Output | dB | -60 | 0 | -12 | Linear dB |
| 1 | amp.attack_ms | Attack | ms | 1 | 10000 | 10 | Log |
| 2 | amp.decay_ms | Decay | ms | 1 | 10000 | 100 | Log |
| 3 | amp.sustain | Sustain | ratio | 0 | 1 | 0.7 | Linear |
| 4 | amp.release_ms | Release | ms | 1 | 10000 | 250 | Log |

All five are continuous and intended to be host-automatable. Sustain is stored
as a ratio and displayed as a percentage in the GUI. Output gain becomes
`pow(10, dB/20)` in DSP; -60 dB is not a mute switch. Milliseconds become seconds
at the engine boundary. Parameter conversion utilities clamp finite values
and replace NaN/infinity with the default. Unknown IDs fail lookup.

Host normalized values use [0,1]. Log time mapping is
`min * pow(max/min, normalized)`. The iPlug2 adapter must implement this exact
mapping so host automation, GUI and presets agree. Parameter utilities and iPlug2 registration are implemented. Time parameters
use iPlug2 ShapeExp, matching the logarithmic contract.

## State schema (specified, not implemented)

Use schema version 1 with a `SAWSTAR` magic marker, a bounded payload length
and records keyed by stable parameter ID. Encode fixed-width fields and
explicit endianness; never dump a C++ struct or enum memory layout. Store
physical parameter values, not transient oscillator or envelope state.

Restore into a temporary snapshot before publishing it to the audio thread.
Reject bad magic, unsupported future versions, truncated payloads, duplicate
known IDs and non-finite values without changing live state. Unknown records
may be skipped only after their length is validated. Missing known parameters
use defaults; finite out-of-range values clamp. Bound record count and payload
size. Add a migration function when changing schema and keep fixture states.

Host save/recall and the Init preset must share one canonical state path.
Preset names and learning text are metadata, not audio parameters. Editor tab
selection is optional editor state and must not affect the sound. The development shell currently uses iPlug2 native parameter state persistence
(PLUG_DOES_STATE_CHUNKS=0). The custom codec described above is not implemented;
its eventual introduction needs backward migration from framework parameter state.
