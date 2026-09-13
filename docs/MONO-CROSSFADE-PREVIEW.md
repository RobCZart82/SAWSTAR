# Mono crossfade experiment — 1.0.2-rc2

Unreleased comparison build, extending the rc1 voice-transition fixes.
No GUI layout, parameter IDs, factory presets, note priority, or envelope
retrigger rules change.

## Implementation

A running Mono/Legato voice is copied before a transition changes its targets.
The snapshot includes both oscillator banks, sub/noise generators, amplifier and
filter envelopes, oscillator phases, and filter integrator history. It continues
rendering at the previous pitch while the new voice takes over over 6 ms.
Smoothstep blend weights sum to one; this avoids the gain bump an equal-power
crossfade can create with correlated sources. The existing 3 ms continuity
correction handles the exact splice and rapid retargeting of a mixed output.

There is one fixed transition slot. It does not consume a musical voice or
allocate memory on the audio thread. A dense MIDI group snapshots only once
before its next rendered sample; later transitions replace rather than accumulate
tails. A mode change releases the tail, reset clears it, and channel panic
silences the matching tail. The first attack from idle has no crossfade.

This is a crossfade, not lookahead: no audio buffers or MIDI events are delayed.
The new tone still becomes dominant gradually, which can affect articulation.
For part of a Mono transition two voice paths are evaluated; it is not a claim
of zero CPU cost. Poly notes do not create transition snapshots.

## Engine-only comparison with rc1

The supplied MIDI/preset reproduction is useful for relative comparison, but is
not sample-identical to the user's REAPER render. Original media are not modified
or added to the repository. Measurements below use 44.1 kHz, linear amplitude:

| Transition | Peak, 5.710–5.736 s | RMS, first 6 ms |
| --- | ---: | ---: |
| rc1 correction only | 0.0427751 | 0.0184214 |
| 3 ms crossfade | 0.0408875 | 0.0170998 |
| **6 ms crossfade** | **0.0312070** | **0.0099416** |
| 10 ms crossfade | 0.0312070 | 0.0055841 |

The selected 6 ms version reduces the measured burst peak by about 27% (2.74 dB)
and onset RMS by about 46% (5.36 dB). The 10 ms version does not further reduce
the peak in that window, so it is not selected. The 25 ms held-note fallback still
exists. This build does **not** claim to eliminate the entire audible bass burst.

For Resonant Bass CutOff, Resonant Pluck CutOff, and SuperSaw CutOff, a 25-note/s
overlapping phrase was compared in Poly, Mono and Legato. The first idle attack
was sample-identical in all nine cases, and all Poly output was sample-identical.
Mono/Legato phrase RMS ratios were 0.973–0.986 relative to rc1 (about -0.24 to
-0.12 dB); this is not a perceptual guarantee that every pluck is unchanged.

## Regression and listening requirements

The four-octave transition test compares the first 0.5 ms with an independently
continued old sine. Maximum pre-FX deviation is under 0.05 at 44.1/48/96 kHz;
rc1 deviates by over 1.5 and fails this regression. This checks actual waveform
continuation rather than only first-sample equality.

Tests also cover dense seven-sample retargeting, release shorter than the
crossfade, mode changes during a tail, and panic. Existing short-note, note-count,
glide, sustain, Poly reuse, reset, and torture tests remain applicable.

Host listening should compare the same project and preset with rc1 and rc2,
especially overlapping bass/pluck notes, glide, and fast repeated notes. Do not
normalize away the transient differences. GUI geometry and musical gate rules
are deliberately unchanged; any further envelope-rule change needs its own
comparison.


## rc3: release fallback and repeated-note articulation

The user heard improvement in rc2 but still heard the pop. Inserting short gaps
between MIDI phrases eliminated most of it. rc3 makes two targeted changes;
it does not lengthen the 6 ms crossfade or move any GUI control.

* Mono Note Off fallback now preserves the amplitude and filter envelope stages.
  Previously SelectMono forced a fresh attack even without a new Note On. A
  still-held key can therefore return without an extra envelope accent. Pitch
  fallback and last-note priority remain active; rc3 does not discard held notes.
* A repeated Note On on the selected Legato key explicitly retriggers. Adjacent
  same-pitch notes now sound identical for Off/On and On/Off ordering (with zero
  glide), using the existing matched-note counters. Different-pitch overlapping
  Legato notes retain their existing envelope behavior.

This deliberately changes articulation in those two situations. It does not
claim to remove every transient. The original project contains a real 25 ms
lower-note fallback, which remains audible as a pitch event.

In the same 44.1 kHz engine-only reproduction window (5.710018–5.736 seconds),
rc2 peak/RMS were 0.031207 / 0.015068, and rc3 0.025172 / 0.012175 (about 19%
lower peak and RMS). This comparison is not a perceptual pass/fail criterion.
The initial attack remains sample-identical to rc2 and starts with a zero sample;
the user's tiny transport-start click remains an open host/listening check.

Regression tests isolate repeated-note event ordering in Poly, Mono and Legato,
and amplitude/filter envelope fallback at identical pitch on separate channels.
All run at 44.1, 48 and 96 kHz. Existing mono, overlapping-note, short-note,
idle-start, reset, parameter transition, engine audit and 10,000-step torture
checks pass locally. Fixtures stay on the heap to avoid Windows Debug stack
exhaustion (the rc2 test-only follow-up also passed Windows CI).


## rc4: isolated cold-filter entry experiment

Offline controls confirmed that source phase and cold filter history strongly
influence the original first-note transient. Pre-running the source/filter for
10 ms before opening the unchanged amp envelope reduced first-10-ms peak from
0.026777 to 0.001989, but this is not a suitable direct implementation: it packs
extra rendering work into the note-on callback and advances source phase.
A phase offset also changed the result, without establishing a generally correct
phase for every waveform/preset. Neither control is shipped.

rc4 instead adds a 0.5 ms smoothstep entry into the filter's wet excitation on
truly idle voice starts. The amplitude envelope, dry path, MIDI timing, GUI and
running-voice retarget rules are unchanged. There is no pre-render loop,
lookahead, allocation or added sample buffering. The first half-millisecond of
the wet-path articulation deliberately changes; listening must judge pluck
sharpness. This is not a claim of perceptually identical attack.

LowPass::BeginNote explicitly requests the entry. Clear alone still clears
history without changing the general filter's input behavior. The entry state
belongs to each filter instance and is included in the existing voice copy.

At 44.1 kHz with the original preset/MIDI, first-10-ms output peak changed from
0.026777 (rc3) to 0.005972 (rc4), about 78% lower. The 100–130 ms post-onset
comparison window is sample-identical. The 5.710–5.736 second fallback window
retains its 0.025172 peak: rc4 specifically does NOT solve that remaining event.

The idle-start regression compares a cold LP24 saw entry against the controlled
entry at 44.1/48/96 kHz; it also checks exact dry bypass and settled-response
agreement. Filter, filter-character, filter-modulation, mono, repeated-note,
short-note, idle-start, reset, parameter transition, engine-audit and torture
checks passed locally. The next listening comparison should first compare the
initial chord alone; then check that bass/pluck attacks remain suitably sharp.
