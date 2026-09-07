# Mono / Legato / Glide

ADVANCED adds Voice Mode (Poly / Mono / Legato), Glide (0-2000 ms), and Glide
Mode (Always / Overlap only). All old states and eight factory presets keep
Poly and zero glide. These are three append-only parameter IDs 59-61.

- Poly retains the original 16-voice behavior; Glide has no effect.
- Mono uses one voice; each new note or fallback restarts amp/filter envelopes.
- Legato uses one voice; overlapping physical keys continue the current envelopes.
  A separate phrase restarts envelopes, including when only sustain pedal remains.
- Last pressed physical key wins across MIDI channels. Releasing it returns to
  the most recent remaining physical key, then to a pedal-latched key if any.
  Velocity follows the selected key with smoothing. Pitch bend and modulation
  use the selected note's MIDI channel; pedal state remains channel-specific.
- Glide is a fixed-duration linear ramp in semitones for both OSCs and SUB.
  Overlap only glides on overlapping notes and fallback; Always also glides from
  the last pitch between phrases. The very first note starts at its own pitch.
  Zero glide snaps to the target. A time change applies on the next transition
  (setting zero completes an ongoing glide immediately).

Filter key tracking follows the destination MIDI note; it does not slide with
oscillator pitch. Noise is unpitched. FX tails continue after note release.
Switching Voice Mode releases the current phrase and discards held-key history;
play fresh notes after switching. Starting a mono note ends residual poly voices.
Repeated same-key note-ons retain one held entry, as in the existing poly mode.
CC64/121/123 and CC120 clear/release the appropriate channel state. CC120 stops
sources immediately; existing effect tails follow their existing behavior.

State stores parameters, not held keys, pedal state or a running glide. Fixed
key storage and per-sample pitch ramps allocate no memory in the audio callback.
Start SuperSaw One with Legato, Glide 120 ms, Overlap only for a sliding lead.
The preset remains unchanged until the user saves an edited sound.

IDs/defaults: 59 voice.mode = Poly; 60 voice.glide_ms = 0;
61 voice.glide_mode = Overlap only.

## Validation — 2026-09-07

Tested code `154bb141b680c914124911b63c354f20b204df38`:
[macOS CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34137756744) and
[Windows CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34137756808)
passed all 17 tests in Debug/Release and all 47 VST3 validator checks.
Regression coverage includes note-on/off and mode changes before the first
sample, repeated notes, channel isolation, sustain/controller reset/panic,
measured glide pitch, mono/legato envelope behavior and Poly audio identity.
Pitch/playing tests run at 44.1/48/96 kHz. Manual REAPER playing, UI and
project-restart checks for this build remain pending.
