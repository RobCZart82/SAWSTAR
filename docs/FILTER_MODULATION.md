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
