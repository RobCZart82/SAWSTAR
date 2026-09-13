# Prepared idle filter start — 1.0.2 rc5 experiment

This is a test-branch change, not a published release or a claim that every audible pop is fixed. GUI, parameter IDs, preset format and MIDI event timing are unchanged.

## Behavior

For an actually idle voice, targets are snapped after its note-specific pitch and cutoff are known. Eligible low-pass voices now estimate filter history from a **64-sample virtual-period preview and 16 harmonics**. This is bounded work, independent of the audio sample rate. The estimate includes the two TPT stages and the drive DC-blocker history. Preview oscillators and the sub source are copies: their real phase, the dry signal and amplitude envelope are not advanced.

The preview excludes DC because a partial, detuned cycle cannot reliably estimate a sustained offset. It approximates an initial tonal state; it is not an exact periodic solution for detuned unison or nonlinear drive. Source partials above actual Nyquist are omitted.

The new path is limited to LP12/LP24 with the lowest active source fundamental above 1.5 times the current cutoff. Noise, nonstationary leaky-triangle oscillator sources, clipped source frequencies, dry bypass and the other filter regions keep the previous entry path. Reused, stolen, mono/legato and transition-tail voices do not run this preparation. It allocates no heap memory, uses no locks and adds no lookahead latency.

## Why not ordinary preroll?

A 5–30 ms oscillator/filter preroll reduced the target transient, but piled too much work into the first audio callback. Caching drive calculations and fixed-shape bulk rendering helped only partly. Solving an approximate periodic state from an integer-length cycle was faster, but a broad shape sweep exposed false DC/history estimates, particularly with a cold triangle integrator. That general solution was not enabled.

## Local measurements

Apple M1, optimized standalone engine, original saved error-preset state and extracted MIDI. These are deterministic engine observations, not an assertion of sample-identical REAPER MIDI interpretation or a listening test.

| Original error project, Mono | rc4 | rc5 |
|---|---:|---:|
| Peak in first 10 ms after first chord | 0.005972284 | 0.002348685 |
| Peak at 100–130 ms after first chord | 0.001907733 | 0.001940375 |
| Later 5.710018–5.735294 s transition peak | 0.025171600 | 0.025171600 |

The first peak falls about 61% versus rc4. It is still about 1.21 times the later comparison-window peak. The later transition is sample-identical in that window: this change does not claim to fix that remaining musical/voice-priority transition. The original project/MIDI was not modified.

The checks also cover 96 note/phase combinations and 48 natural idle restarts. The latter's highest note-82 first-window peak is 0.00296296. In a separate 864-case shape/rate/drive sweep, 288 cases take the new path. One case has a >10% relative increase in first-window peak: 0.00000911425 to 0.0000116465 (about -99 dBFS after the change). Relative peaks at tiny levels are not perceptual click tests. Other regions deliberately remain unchanged.

`tests/filter_start.cpp` compares against independently settled filters over 72 rate/cutoff/resonance/mode cases (maximum absolute error 2.38419e-7 for the known two-harmonic reference), checks snapped source preview/continuation sample identity, and checks eligibility/rejection guards. The exact reference uses drive=0; nonlinear-drive quality is assessed separately in the engine sweep, not claimed exact by this test.

## CPU bounds of these measurements

The 16-voice onset benchmark uses two full unison banks, sub, multiple shapes, 0/24 dB drive and 64 audio frames; 40 timed runs per case after discarding the first. At 44.1/48/96 kHz, all measured cases fit the block duration. At 96 kHz the worst case median is 0.338 ms, and the largest observed individual time is 0.576 ms, against a 0.667 ms block.

At **192 kHz / 64 frames**, 3 of 18 cases exceed the 0.333 ms block at the measured 95th percentile. This setting is not verified for maximum-polyphony onset without an underrun. Larger buffers provide more headroom; these standalone timing samples do not guarantee a deadline on other CPUs or in a busy host. Continuous audio CPU and worst-case onset CPU are different measurements.

## Remaining verification

A/B listening in REAPER is still required. The original later mono note-priority transition remains audible according to the earlier user report and is outside this idle-state change. Triangle/noise start handling and very high-rate/small-buffer onset remain separate work; the fallback is intentional. No public release should be inferred from a successful build.

Local AddressSanitizer/UndefinedBehaviorSanitizer checks passed for prepared-state math, idle starts, overlapping notes, voice transitions and the deterministic torture test. Host validation and listening remain separate.
