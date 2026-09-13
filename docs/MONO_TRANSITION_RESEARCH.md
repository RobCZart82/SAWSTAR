# Mono transition investigation — 1.0.2 rc6

2026-09-13. Baseline: `fec8e9a2e966439bc3887afca0e5349de0405f04`.
The user reports that the original first-chord onset is now much better, almost
inaudible, but low-register changes remain more noticeable. The measurements
below are offline observations of the saved error preset/MIDI at 44.1 kHz;
they are not listening tests or proof that every reported pop has one cause.

## Verified first-chord Glide defect — fixed in source

`SelectMono` made a pitch valid immediately on Note On. A second Note On at
the same sample could therefore start Glide from that first, unrendered note.
This happened on a completely fresh chord, in both Mono and Legato, before
there was any audible pitch to glide from.

The engine now separately records whether a mono pitch has actually rendered.
Reset and mode changes clear this history. Silent gaps keep real history so
Always Glide between separated notes still works. Note counts, last-note
priority, envelopes and preset parameters are unchanged.

With the saved patch, a same-sample first chord and 3 ms Glide, its first 10 ms
peak was **0.078711**, versus **0.002349** with Glide off. After the fix both
have the latter peak. The 15/120 ms cases previously had peaks of 0.108754 /
0.116597 and now also match Glide off. These are signal peaks, not perceptual
loudness ratings. A chord and a single highest-note strike retain their small
pre-existing continuity-correction difference; that is not changed here.

The new regression fails on rc5 and passes after the fix: first-chord output
must exactly match Glide off at 44.1/48/96 kHz, Mono/Legato, both Glide modes,
and 3/15/120 ms. It also exercises reused/reset fixtures and held-key retention.
Existing mono tests still verify real overlapping and separated-note glide.

The Glide-only change is sample-identical to rc5 with the original saved
Glide=0 patch. The complete rc6 changes below additionally improve short
transitions. None of these findings is a listening-test claim.

## Later low-note fallback — still under investigation

The critical interval is 5.710018–5.735294 seconds: the selected mono note
returns 80 → 68 → 80. The shorter note is physically held for about 25.276 ms
after the upper notes release. In the octave-lower control this is 68 → 56 → 68.

New trace data establishes that the **actual cutoff is about 233.022 Hz**,
including modulation, throughout this interval. The 100 Hz preset base value
must not be confused with the modulated cutoff. Filter coefficients, drive
(3 dB) and wet mix (100%) remain constant. The amplitude envelope stays near
0.8006 on fallback, and the first output-sample delta is zero.

At the lower transposition OSC1 changes 415.3 → 207.65 Hz and SUB changes
207.65 → 103.83 Hz. Thus substantially more tonal energy enters the passband.
The largest output excursion is 10.68 ms after the change (15.74 ms in the
original register), not at the first switched sample. Over a tapered 85 ms
diagnostic window, about 99.97% of the lower-register output energy is below
500 Hz. This is the whole musical signal, not an isolated artifact spectrum.
It supports investigating a short bass burst, but does not alone prove the
absence of a perceptible click or an implementation defect.

## Isolated experiments — not enabled in the product

32 variant/register renders compared the existing 6 ms tail with 12/24 ms
tails, prepared incoming LP history, no continuity correction, and a small
forced glide. Longer fades and prepared history did not substantially lower
the deep-register peak. For the original register, 24 ms fading lowers the
25 ms-window peak from 0.025172 to 0.024267, but the following 60 ms contains
a peak of 0.024827: much of the apparent improvement shifts in time. The
forced-glide prototype also exposed the first-chord defect above and is not
a production recommendation. No general minimum attack/release was added.

18 duration controls varied only the diagnostic late-release interval.
With the lower register, a 25 ms interval peaks at 0.118044; a 100 ms held
return reaches a similar 0.117571. This is consistent with much of the large
excursion being the lower note itself. These controls alter musical timing;
they are not edits to the user's original project or a proposed MIDI filter.

## Overlapping transition tails — fixed in rc6

The old single transition tail was replaced if another pitch change arrived
before its 6 ms fade finished. The replacement snapshot was of the incoming
voice, although most audible output could still come from the preceding tail.
The final-sample correction could not preserve that mixed waveform.

The engine now retains up to **four outgoing branches**, with each branch's
current amplitude share and original 6 ms deadline. A new retarget captures
only the current voice's remaining audible share. Old deadlines are never
extended; musical polyphony is still 16 voices. Storage and iteration counts
are fixed; no heap allocation, lock or MIDI lookahead is added.

If extreme event density exhausts all four branches, the smallest component
can be discarded and the existing short output correction is used. This is a
bounded fallback, not exact preservation of arbitrarily many simultaneous
transitions. Channel-local CC120 clears only that channel's branches, including
when voice zero has already stopped. Reset, mode changes and final releases
are covered by regression tests.

## Natural waveform edges — fixed in rc6

A complete outgoing mixture already supplies the natural next sample. Starting
a new 3 ms correction to force that sample equal to the **previous** one can
mistake a normal saw/square edge for an unwanted discontinuity. It introduces
an artificial offset lasting much longer than that edge.

A fully preserved mixture now continues naturally. A correction already in
progress keeps decaying; a new correction is started only when the mixture
could not be retained, including pool exhaustion. Poly voice-stealing behavior
is unchanged. Tests now compare a retarget's first sample with an independently
continued reference voice, rather than requiring an unnatural flat sample.
The exact comparison uses equal velocities to isolate waveform continuity
from the intentional velocity smoothing.

In a 192-case one-sample excursion sweep (44.1/96 kHz, notes 36/48/60,
12/36-semitone jumps, four waveforms and four phase offsets), maximum absolute
pre-FX difference from an uninterrupted reference was **2.58775** on rc5,
**1.28833** with only multiple tails, and **0.00349194** with both fixes.
Maximum RMS difference fell from 1.96038 to 0.00160752. RMS improved in 170
cases; the largest increase among the other cases was 2.54e-8. These are
short-window waveform differences, not dB values or listening scores.

The committed regression expands this to 576 cases with 48 kHz and both Mono
and Legato. It requires absolute pre-FX error below 0.01 for the first 0.5 ms
after return. It fails on the preceding implementation and passes on rc6.

## Saved-patch controls and remaining limits

The same error preset and 44.1 kHz render were used; only diagnostic MIDI copies
change the short lower-priority note's duration. The peak window includes that
interval and 40 ms after the upper note returns.

| Lower-register return duration | rc5 peak | rc6 peak |
|---|---:|---:|
| 1 ms | 0.0883791 | 0.0441101 |
| 2 ms | 0.0774510 | 0.0445343 |
| 4 ms | 0.0932179 | 0.0792927 |
| 25.276 ms | 0.1180437 | 0.1180437 |
| 100 ms | 0.1175714 | 0.1175714 |

The 1/2 ms peaks fall about 50%/42.5%. In the original register they fall
about 47%/45%. This is not a blanket reduction of every transient: the lower
12 ms control peak increases about 1.5%, and the original-register 50 ms
control about 1.3%. Some phase-dependent peaks move within a waveform.

For the unmodified full 16-second MIDI, Poly remains sample-identical to rc5.
Mono and Legato have maximum absolute output difference 0.000317392; the
first chord is sample-identical, and the critical 25.276 ms-window peak remains
0.025171600. **The reported longer low-note thump remains unresolved.** Its
last-note priority and MIDI timing have not been silently changed.

Nine optimized local engine test groups pass, including deterministic torture
at 44.1/48/96 kHz (10,000 steps and 3,056,063 samples per rate).
AddressSanitizer + UndefinedBehaviorSanitizer pass for voice transitions,
mono, overlapping notes and reset lifecycle. Host/CI results are recorded
separately; this document does not claim a public release or completed listening
validation.

## CPU observations

Apple M1, optimized standalone engine, 64-frame blocks; four waveforms, two
full seven-source banks plus sub and drive. In separate before/after runs,
ordinary Mono medians stay around 23–25 microseconds and 16-voice Poly around
302–305 microseconds. Artificially changing notes every 16 samples raises the
Mono median from about 60 to 119 microseconds, as more outgoing branches must
render. The pool makes that additional work finite; typical playing does not
keep four transitions active continuously.

At 96 kHz the worst measured dense-Mono 95th percentile is 124.5 microseconds;
16-voice Poly is 322.3 microseconds. Individual wall-clock outliers can exceed
the audio block even on unchanged code, and these measurements are not a host
real-time guarantee. The existing 192 kHz / 64-frame maximum-load limitation
remains; see [idle-start CPU observations](PREPARED_FILTER_START.md).

## Technical references

JUCE's [sine synthesizer tutorial](https://juce.com/tutorials/tutorial_sine_synth/)
explains pitch smoothing and discrete-change artifacts. Its example does not
establish a universal MIDI-glide policy or a suitable fixed fade time here.

Wishnick's [Time-Varying Filters for Musical Applications, DAFx-14](https://www.dafx.de/paper-archive/2014/dafx14_aaron_wishnick_time_varying_filters_for_.pdf)
distinguishes numerical stability from audible artifact behavior and discusses
state-based transient minimization. It provides useful background, but our
critical traced coefficients are constant: its coefficient-change experiments
are not direct proof of this particular diagnosis. No source code was copied.
