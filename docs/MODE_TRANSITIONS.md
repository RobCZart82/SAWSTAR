# Voice-mode lifecycle fixes — 1.0.2-rc7

Private test candidate on `codex/mono-crossfade-preview`; no public release.
GUI geometry, parameter IDs, factory sounds, musical note priority and MIDI
counters are unchanged.

## Confirmed defect

After Poly -> Mono/Legato, the first Mono note used to mark the other Poly
slots inactive immediately. Their amplitude envelopes could remain at sustain.
A later Poly note reused the apparently idle slot with a soft retrigger and
inherited that old level instead of starting its requested Attack from zero.
The same immediate stop also discarded the outgoing Poly audio.

The new regression fails against rc6 and passes with this fix. At 48 kHz,
a separate diagnostic probe with 100 ms Attack found a first envelope value
of 0.8002304 in the stale slot, compared with 0.0009706 on a fresh start.
These are internal envelope values, not output dB or listening scores.

## Implementation

- A genuinely inactive voice starts with a cleared amplitude envelope. A voice
  still rendering keeps its soft retrigger and existing transition treatment.
- A mode change releases old voices and applies a 6 ms smoothstep fade in their
  existing slots. A second mode change cannot extend that retirement deadline.
- Starting a Mono note preserves outgoing Poly sources. Voice zero can enter
  the existing Mono tail pool at its actual Poly pitch and velocity; the other
  slots finish their fades in place.
- The retirement gain also covers an outstanding output correction. At its
  zero endpoint, the slot becomes inactive and its amplitude level is cleared.
- Panic and reset still clear the relevant sources immediately. Shared effects
  may continue their ordinary tails after a mode change.

There is no extra voice array, allocation, MIDI buffering or lookahead. Active
voice reporting can include retiring Poly slots for up to 6 ms after switching
to Mono. Those voices continue rendering briefly, so a transition is not free
of CPU work. The musical polyphony limit remains 16.

## Validation

At 44.1, 48 and 96 kHz, the new tests cover idle reuse after round-trips,
100/1000 ms Attack, outgoing-waveform continuation, all directed mode changes,
rapid changes at full capacity, natural release, reset and channel-local panic.
Existing voice transitions, Mono, overlapping notes, reset lifecycle, idle
starts, parameter transitions, engine audit and deterministic torture tests
passed locally. ASan/UBSan runs of mode transitions, voice transitions, Mono,
reset lifecycle and overlapping notes completed without a diagnostic.

The original 46-note exported phrase and its 16-note two-keys-per-chord reduction
were compared with rc6 in all three modes, with wheel values 0 and 31 at 44.1 kHz
(12 complete 14.7-second comparisons). Maximum absolute difference across output
and pre-FX samples was 7.46e-9; in Mono/Legato it was at most 2.33e-10. These tiny
floating-point differences are not evidence of a change to the reported burst.
The original MIDI, project and audio remain outside the repository, unmodified.

A local optimized 48 kHz timing probe (three alternating runs, 3 seconds of
audio per scene) measured median render-time/audio-duration ratios of 11.74%
for rc6 and 11.71% for rc7 with 16 held voices. An intentionally extreme scene
changing mode and sending a 16-note group every 10 ms measured 5.29% versus
7.45%: rc7 continues outgoing sources instead of cutting them. These are local
engine timing observations, not whole-system CPU or a worst-case host guarantee.

## Remaining investigation

This patch fixes a distinct mode-change lifecycle defect. It does **not** claim
to solve the reported low-note pop in a phrase played entirely in Mono.
The original priority path briefly returns from note 80 to held note 68 for
about 25.3 ms, then returns to 80; the later boundary has the analogous octave
shift. The full chord is reducible to two relevant keys per chord with identical
Mono output in the rc6 comparison. That minimal reproduction remains the next
focus for source/filter transition analysis and listening.
