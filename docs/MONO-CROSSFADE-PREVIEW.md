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
