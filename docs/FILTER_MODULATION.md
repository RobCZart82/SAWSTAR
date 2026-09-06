# Filter envelope and keyboard tracking

MAIN now has three rows: oscillator/filter, filter envelope, amp/output.
The shared flat keyboard and all three tabs remain available.

Each voice owns a separate DaisySP MIT ADSR for its filter. Env Amount spans
-96 to +96 semitones. Key Track spans 0–100%, with MIDI note 60 as the pivot:

`cutoff = baseHz * 2^((amount * envelope + tracking/100 * (note-60))/12)`

The result is limited to 20 Hz–min(20 kHz, 0.45*sampleRate). The envelope runs
every sample; frequency targets update every 16 samples, independently of host
block boundaries, then use the low-pass's existing 10 ms coefficient smoothing.
This intentionally softens very fast filter attacks. Processing has no allocation
or locks. Each update evaluates exp2/tan only as needed, not on every sample.

Idle voices retrigger from zero; live repeated/stolen voices retrigger from their
current level. Note-off and sustain pedal control both envelopes. Amp release
still determines voice lifetime: a longer filter release cannot extend the sound
after the amp envelope finishes. Envelope timing follows the existing DaisySP
ADSR curve (decay/release are time constants, not exact time-to-zero durations).

Defaults: Amount 0 st, Key Track 0%, Attack 10 ms, Decay 200 ms, Sustain 0,
Release 250 ms. Old projects receive these defaults; no filter modulation is
added to their sound. Appended IDs 11–16 preserve all prior IDs and mappings.
The state schema remains v1; current saves contain 17 records.

Try Cutoff 400 Hz, Filter Mix 100%, Env Amount +36 st, F Attack 10 ms,
F Decay 200 ms and F Sustain 0 for a pluck. Raise Key Track to 100% for
consistent relative brightness across octaves. Negative Amount closes the
filter during the attack; choose a higher base Cutoff to hear it clearly.

Tests cover tracking pivot/octaves, bipolar attack and release, decay, clamps,
zero-modulation audio identity, full-voice bounds, panic and old state migration.

## Validation — 2026-09-06

Tested build: 164bd5b7ebb5eb9cae7a6d134fbab562774522dc.
macOS run 34041371376 and Windows run 34041371356 passed Debug/Release
foundation checks, all seven test executables, and all 47 VST3 validator tests.

REAPER 7.79 on macOS loaded the old eleven-record project with existing values
intact and Amount/Key Track at zero. The final editor was checked after restarting
REAPER to unload its cached plugin binary. Values appear below knobs and can be
edited directly. The saved test project contains all seventeen records.

The 44.1 kHz stereo 24-bit render used Cutoff 404.39 Hz, Resonance 31.403%,
Filter Mix 100%, Amount +36 st, Key Track 100%, filter ADSR 10/200/0/250.
Peak was -23.08 dBFS with zero clipped samples. The separate listening preview
is normalized to -6 dBFS; plugin gain was not changed by normalization.
Packages are unsigned development builds. Manual Windows host testing is pending.
