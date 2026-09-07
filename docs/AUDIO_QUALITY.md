# Audio quality and performance pass

This pass leaves GUI layout and factory sound design for later. Parameter IDs,
source gains, unison normalization, clean filter response and effect mix laws are
unchanged. No new third-party code is introduced.

## Output protection

The stereo-linked 0.98 sample-peak guard now covers every signal path, including
matrix-only amplitude/pan modulation with dry effects and zero Output Boost.
Previously that combination could select the historical bypass and reach the
final hard clamp. Existing sounds below the threshold retain their gain; loud
signals now receive the same protection regardless of routing. Gain recovery
remains 80 ms and processing adds no latency. This is sample-peak protection,
not a true-peak mastering limiter or loudness normalization.

Regression coverage exercises velocity-to-amplitude and velocity-to-pan on 16
coherent voices, in addition to the existing boosted/mixed/stereo output tests.

## Drive DC rejection

The first measurement exposed a roughly 0.0303 DC mean in the 16-voice driven,
effected scene. A 5 Hz DC blocker now removes saturation-induced offset before
the filter. Its contribution follows the existing Drive blend, leaving zero
Drive's clean path unchanged. Histories are cleared on voice reset and kept
warm while processing. This deliberately changes driven sounds at very low
frequencies; it is not a waveform loudness boost. Output tests verify DC rejection
with sustained driven input while checking that the signal remains audible.

## CPU work

Octave ratios are computed once per rendered sample rather than once per voice.
Filter drive skips the power function at zero drive and shares the normalization
calculation between stereo channels. Oscillator phases, envelope timing and
filter histories remain continuous; muted sources are not suspended.

`scripts/benchmark-audio.py` builds the previous engine and runs the exact same
workload on both versions, alternating three times on each Release CI runner.
The six workloads use 1/8/16 voices at 48 kHz, two unison oscillators, sub/noise,
a filter, and either dry or active chorus/delay/reverb. It prints sample peak,
stereo RMS, DC mean and render time as a percentage of the rendered duration.
This measures engine throughput with measurement overhead, not REAPER's CPU
meter or a worst-case real-time scheduling guarantee. Timing is observational;
there is no machine-dependent pass/fail CPU threshold.

The benchmark does not establish perceptual loudness equality across waveforms,
noise colors or presets. That balance still needs listening at matched levels.
Further work should be driven by measured bottlenecks and listening: automation
transients, high-register aliasing and dense-chord/effect-tail behavior.

## Measured results

Tested source: `2c359d8c0df8fa3d987721ac64cc4556205ce620`.
Median of three renders; percent of one second spent rendering one second.
Lower is better; the machines and background load differ between platforms.

| Runner | Scene | Voices | Baseline % | Current % |
|---|---|---:|---:|---:|
| macOS | dry_fx | 1 | 1.251 | 1.419 |
| macOS | dry_fx | 8 | 9.307 | 8.975 |
| macOS | dry_fx | 16 | 15.980 | 15.344 |
| macOS | full_fx | 1 | 1.720 | 1.660 |
| macOS | full_fx | 8 | 12.907 | 10.883 |
| macOS | full_fx | 16 | 23.265 | 22.747 |
| Windows | dry_fx | 1 | 1.530 | 1.536 |
| Windows | dry_fx | 8 | 10.852 | 10.113 |
| Windows | dry_fx | 16 | 23.708 | 22.028 |
| Windows | full_fx | 1 | 2.522 | 2.476 |
| Windows | full_fx | 8 | 14.833 | 13.644 |
| Windows | full_fx | 16 | 31.191 | 28.564 |

In the Windows 16-voice full-FX workload, median render cost decreased from
31.191% to 28.564% (about 8.4% relative). macOS timing varies substantially;
no general speedup claim is made. These are CI observations, not laptop figures.

The 16-voice driven/full-FX scene's DC mean fell from about +0.03035 to
-0.000657 (about 33 dB lower magnitude). RMS moved from -14.511 to -14.630 dBFS;
peak moved from -3.940 to -4.458 dBFS. The dry/zero-drive scene's reported peak,
RMS and DC remained identical within each platform at printed precision.
Raw repeated measurements are in [audio-quality-measurements.json](audio-quality-measurements.json).

- [macOS CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34157831982)
- [Windows CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34157831935)

Listening and REAPER validation of this new build remain pending. This is the
first quality/performance pass, not a declaration that the engine is final.

Both Release VST3 builds passed all 21 foundation tests and all 47 VST3 validator
checks. The added output regressions cover the matrix-only protection case and
Drive DC rejection at 44.1, 48 and 96 kHz.
