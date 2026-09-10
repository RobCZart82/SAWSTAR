# DSP reset and input safety — 2026-09-10

Code revision: `a49a5e07d919b32443bb113d51ece727493d6a99`.

Noise Color now retains its target through engine Reset. When no voices are active, selecting the noise source or color establishes the starting sound immediately; live changes retain smoothing. Reset still clears playing voices and MIDI runtime state. This is not a promise that every engine setting survives Reset independently: the plugin continues to reapply its parameters.

Public DSP entry points now reject non-finite gain, envelope, filter, sub-frequency and pitch-multiplier inputs. Sample rates are bounded to 8–384 kHz, with 44.1 kHz used for NaN/infinity. Normal parameter ranges and preset IDs are unchanged.

Validation:

- All 32 local test executables passed with AddressSanitizer and UndefinedBehaviorSanitizer.
- New regression tests compare audible noise renders across Reset and exercise non-finite input followed by valid-input recovery.
- All six GitHub jobs passed: macOS/Windows Debug and Release foundations plus both VST3 builds. All 32 tests passed on both platforms; both plugins passed all 47 VST3 validator tests.
- Downloaded development packages were checked against GitHub artifact SHA-256 digests and ZIP integrity checks.
- No new REAPER listening test was performed for this package; an existing host session was left open.

Remaining lifecycle work: audit other smoothing targets around host Reset and extend own-code warning coverage in CI. Factory content and Templates remain a later phase described in PRESET_GUI_PLAN.md.
