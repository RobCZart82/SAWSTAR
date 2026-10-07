# Opt-in Drive normalization candidate

The next CPU candidate computes `1 / gain` once per stereo host frame and
multiplies each oversampled nonlinear phase by that reciprocal. The default
path retains `tanh(gain * x) / gain`. At non-unity gain this removes repeated
normalization divisions across two channels and two/four oversampled phases.

`FixedRatePremiumDrive` gains a fifth template argument, `ReciprocalGain`, whose
default is **false**. No persistent state or latency is added. The normal
research aliases, rate-scaled policy and production synth keep their existing
paths. `GainStudyEnginePremiumFilter` selects the candidate only in a separate
offline SIMD/4x engine probe.

## Numerical qualification

Reciprocal multiplication can round differently from division. The research
contract permits at most `2e-7` absolute output error and `1e-7` relative RMS
error, checked separately for each rate/scene as well as in aggregate. These
limits qualify an experiment; they do not establish audibility or shipping
acceptance. The selected character still needs its existing listening gates.

`premium_gain_normalization` covers 2x/4x, scalar/SIMD, std::tanh/ResearchTanh,
six rates from 8 to 384 kHz, loud noise, sine, impulse trains and very quiet
input. It exercises gain ramps, unity gain, snap, clear, copied live state,
invalid input/gain, reinitialization, channel isolation and the 32-sample peak.
A deliberately altered output must fail. Unity-gain output is required to be
bit-identical; object size must remain equal.

The local MSVC 19.44 x64 Release run compared **1,831,104 stereo frames** with
zero differing frames in this fixture, including the strengthened per-scene
checks. This observed identity is not a general guarantee for the changed
double arithmetic. All seven earlier Drive regressions and the four paired
study CTest cases passed. The default division path also remains bit-identical
to the independent frozen ring references.

When the engine dependency is available, the same test also checks 98,304
SIMD/4x adapter frames through Drive, filter and delayed dry mix, including mode/
mix changes, clear and invalid input. That adapter case is a fresh CI gate;
the local dependency-free run covers the Drive qualification above.

## First isolated CPU result

The shared paired tool has a separate `--normalization-benchmark` mode. It
compares the same-source division and reciprocal branches, with equal storage,
for scalar and SIMD at 2x/4x. It uses sixteen objects, 20 dB, three rates,
16,384 frames, 1,024 warmup frames and eight alternating pairs: 96 rows.
The reporter's `--normalization` mode requires zero storage difference and
records the comparison policy explicitly. Both reporter modes retain eighteen
malformed-data rejection controls. Numerical checks run before CI timing;
finite benchmark checksums keep both paths' outputs observable.

Local Windows x64 Release, SSE2; median candidate/reference wall-time ratios:

| Implementation | Rate | 2x | 4x |
| --- | ---: | ---: | ---: |
| Scalar | 48,000 | 0.980280 | 0.957323 |
| Scalar | 96,000 | 0.957093 | 0.965391 |
| Scalar | 192,000 | 0.965266 | 0.958027 |
| SSE2 | 48,000 | 0.902331 | 0.938059 |
| SSE2 | 96,000 | 0.900867 | 0.956809 |
| SSE2 | 192,000 | 0.901063 | 0.953553 |

The first isolated SIMD run shows roughly 4-10% shorter time. Some individual
pairs are slower, and this is a single local run without affinity or realtime
scheduling. It does not establish full-engine or cross-platform CPU savings.

[All local pairs](../experiments/premium_filter/measurements/2026-10-07-gain-local-windows.csv)
and [provenance](../experiments/premium_filter/measurements/2026-10-07-gain-local-windows-provenance.json)
retain the exact uncommitted measured source hashes, compiler description,
backend and ranges. No base commit SHA is assigned to the changed local files.

## Full-engine and platform gates

The separate `premium_gain_deadline_fixture` and `sawstar_premium_gain_deadline`
compare SIMD/4x division and SIMD/4x reciprocal engines. Both use std::tanh,
sixteen Poly voices, 20 dB, all four filter modes and FX at 48/96/192 kHz.
The existing sounding fixture protects finite output, voice count and the
`2e-6` peak-difference bound. The deadline tool records 288 summary rows and
73,728 raw block timings at buffers 32/64/128, four alternating pairs, with
p50/p95/p99/maximum and strict over-budget counts. Timing is descriptive.

Windows/macOS CI collects both isolated and complete-engine results, with
compiled factor/backend records, actual source SHA and compiler metadata.
The full-engine probe requires the pinned DaisySP dependency; its local build
was not performed in this source snapshot. Fresh CI supplies its compile and
fixture gates. Sanitizers cover the added numerical test and engine fixture.

The next milestones are complete-engine/cross-platform evaluation, repeated
target-machine and native REAPER deadlines, then high-rate quality/policy
acceptance. Only after those gates can the production filter, latency/state/
automation, factory presets, manuals and next release be finalized. No default
activation or production filter change follows from this opt-in candidate.

## Windows and macOS complete engine results

The #77 source `fb3f7df0594a33f0f454c94380a6b77f9caa46d7` passed its
Windows/macOS numerical and sanitizer gates. CI run `37613211515` compared
the fixed SIMD/4x engines; run `37613211507` measured isolated normalization.
The paired full-engine median time ratios below are the median of 48 scene
ratios per rate, covering four modes, three buffers and four pairs.

| Platform | Rate | Median candidate/reference time | Reference/study over-budget blocks |
| --- | ---: | ---: | ---: |
| Windows x64 | 48,000 | 0.982514 | 0 / 0 |
| Windows x64 | 96,000 | 0.978266 | 34 / 167 |
| Windows x64 | 192,000 | 0.977511 | 12,288 / 12,288 |
| macOS ARM64 | 48,000 | 0.986880 | 313 / 204 |
| macOS ARM64 | 96,000 | 0.990726 | 1,534 / 1,270 |
| macOS ARM64 | 192,000 | 0.982107 | 12,288 / 12,288 |

There are 12,288 timed blocks per path/rate. Every fixed-4x block at 192 kHz
exceeds the audio deadline on both runners. The Windows 96 kHz study has more
deadline misses despite its better median. The isolated macOS SIMD/4x median
is slower at 48 kHz (1.096853) and 192 kHz (1.091770). These negative results
remain part of the evaluation; isolated Drive savings do not imply safe
full-engine deadlines. Shared runner measurements do not establish a native
realtime or audibility guarantee.

The [Windows engine summary](../experiments/premium_filter/measurements/2026-10-07-gain-ci-windows-engine.csv)
and [metadata](../experiments/premium_filter/measurements/2026-10-07-gain-ci-windows-engine-metadata.json),
[macOS engine summary](../experiments/premium_filter/measurements/2026-10-07-gain-ci-macos-engine.csv)
and [metadata](../experiments/premium_filter/measurements/2026-10-07-gain-ci-macos-engine-metadata.json)
preserve the measured source and compiled factor records. Isolated Drive pairs
and byte-hash provenance are stored beside them. The 73,728 raw engine block
measurements per platform are retained in the corresponding 90-day CI artifacts.

## Rate scaled normalization experiment

`RateScaledGainPremiumDrive` selects the existing SIMD 4x path below the
normalized 176,400 Hz boundary and SIMD 2x at/above it, adding only reciprocal
normalization. The default rate-scaled aliases retain division. Selection
occurs on Init; latency remains 32 host samples. The distinct offline engine
probe compares against rate-scaled SIMD division, so both paths use the same
factor and the measured change is normalization alone.

The routed regression checks explicit fixed-factor reciprocal oracles at the
boundary and invalid sample rates, division error bounds, unity-gain identity,
copied live state, Clear, reinitialization, channel isolation and impulse delay.
The existing fixed-factor numerical qualification remains unchanged. A full
engine sounding fixture covers all four modes; a Windows/macOS paired study
records both actual compiled factors. Reporter regressions reject a wrong 2x/4x
record or an incorrect study/backend label.

Local macOS Release validation passes all 101 CTest cases. The three new
routing, complete-engine fixture and reporter checks also pass with ASan/UBSan.
The routed test compares 69,632 frames with the fixed-factor reciprocal oracle
and the unchanged division error bounds. The exact-head Windows/macOS study
has now completed; its results are recorded below.

This extends the experiment to the relevant high-rate path. It does not approve
the 2x quality compromise, activate a production filter, or close native CPU,
state/automation/preset and listening gates.


## Local Mac mini M1 combined policy result

Source `64cbb57602c799cf8bbc8c5413fb11fe53205dd0`, Apple M1/Macmini9,1,
Clang Release, NEON. Both paths use 4x at 48/96 kHz and 2x at 192 kHz.
This is a single offline desktop run without pinned affinity or realtime
priority, not a native REAPER callback test.

| Rate | Median paired time ratio | Reference/study median audio time | Reference/study over-budget blocks |
| --- | ---: | ---: | ---: |
| 48,000 | 0.988240 | 27.9203% / 27.5941% | 2 / 4 |
| 96,000 | 0.988059 | 55.6562% / 54.9922% | 142 / 134 |
| 192,000 | 0.988192 | 81.8501% / 80.8610% | 271 / 175 |

Ratios and median audio percentages aggregate 48 scene/pair observations per
rate. Miss counts cover 12,288 blocks per path/rate. The median improvement is
about 1.2%; deadline misses remain and the 48 kHz count increases. This does
not close the realtime gate, establish a cause for desktop scheduling spikes,
or approve the high-rate quality policy.

The [summary](../experiments/premium_filter/measurements/2026-10-07-rate-gain-local-m1-engine.csv),
[metadata](../experiments/premium_filter/measurements/2026-10-07-rate-gain-local-m1-engine-metadata.json)
and [source/compiler/measurement hashes](../experiments/premium_filter/measurements/2026-10-07-rate-gain-local-m1-provenance.json)
preserve the observation. All 73,728 raw blocks are also retained in the local
`outputs/premium-rate-gain-m1-20261007` measurement folder. Repeat on the same
machine and Windows, then perform native host acceptance with the approved
quality policy before production integration.

## Combined policy CI result

PR #78 passed all 31 checks and merged as `cafae21`; the three main workflows
also succeeded. Timing run `37632882450` measured source
`0d14778d8cb5d76b647ea9d4f5568fa989491814`, not the merge commit. Both paths
use 4x at 48/96 kHz and 2x at 192 kHz.

| Platform | Rate | Median paired time ratio | Reference/study over-budget blocks |
| --- | ---: | ---: | ---: |
| Windows x64 / SSE2 | 48,000 | 0.978889 | 0 / 2 |
| Windows x64 / SSE2 | 96,000 | 0.979202 | 251 / 186 |
| Windows x64 / SSE2 | 192,000 | 0.991276 | 12,288 / 12,288 |
| macOS ARM64 / NEON | 48,000 | 0.983474 | 7 / 23 |
| macOS ARM64 / NEON | 96,000 | 0.988705 | 813 / 750 |
| macOS ARM64 / NEON | 192,000 | 0.989857 | 631 / 798 |

Each ratio aggregates 48 scene/pair ratios. Each path/rate has 12,288 blocks.
The median improves, but every Windows 192 kHz block still misses its deadline;
macOS 192 kHz and both 48 kHz runs have more misses with the candidate. Shared
runner timing does not establish causation, native realtime acceptance or a
cross-machine guarantee. The combined policy CPU gate remains open.

The [Windows summary](../experiments/premium_filter/measurements/2026-10-07-rate-gain-ci-windows-engine.csv),
[metadata](../experiments/premium_filter/measurements/2026-10-07-rate-gain-ci-windows-engine-metadata.json)
and [artifact hashes](../experiments/premium_filter/measurements/2026-10-07-rate-gain-ci-windows-provenance.json),
and the [macOS summary](../experiments/premium_filter/measurements/2026-10-07-rate-gain-ci-macos-engine.csv),
[metadata](../experiments/premium_filter/measurements/2026-10-07-rate-gain-ci-macos-engine-metadata.json)
and [artifact hashes](../experiments/premium_filter/measurements/2026-10-07-rate-gain-ci-macos-provenance.json)
retain the source and compiled factor/backend records. Raw block measurements
remain in the run's 90-day artifacts.

## Combined policy high-rate complex-source qualification

`premium_gain_complex_controls` runs the combined SIMD/reciprocal candidate
at 176.4, 192 and 384 kHz against the independent 8x offline reference and
the existing fixed 4x path. It also runs rate-scaled SIMD **division** with
identical factors, source and controls to separate normalization rounding
from the rate-policy difference. The existing division-only complex-source
test remains a separate CTest case.

The grid contains 144 cases: four actual OSC1/OSC2/SUB per-voice source scenes,
four filter modes, steady 20/24 dB Drive and stepped Drive, with shared rapid
cutoff targets and 50% resonance. There are 1,024 warmup and 4,096 measured
frames per case: 589,824 measured stereo frames. The independent 8x reference
must first pass delay, silence, channel-isolation and delayed linear-passband
controls at each rate.

All earlier diagnostic ceilings are unchanged: 1% unfiltered difference from
4x/8x, 0.5% filtered difference from 8x and 0.1% 4x/8x convergence. Separately,
normalization must stay within `2e-7` peak / `1e-7` relative RMS before the
filter and `2e-6` peak / `1e-7` relative RMS after it, per case. These are
numerical diagnostic bounds, not an alias-spectrum or audibility threshold.

Local M1 Clang Release source `684b0eab60d02b18d19960b8f427888c71065677`:

| Rate | Maximum relative RMS vs 8x | Maximum filtered relative RMS vs 8x | Maximum 4x/8x relative RMS |
| --- | ---: | ---: | ---: |
| 176,400 | 0.366091% | 0.461440% | 0.011819% |
| 192,000 | 0.386792% | 0.490212% | 0.019081% |
| 384,000 | 0.192252% | 0.253998% | 0.008673% |

The filtered 192 kHz high-lead / HP12 / 24 dB case is close to the unchanged
0.5% diagnostic ceiling. Passing it does not approve the 2x character/aliasing
compromise. Normalization-vs-division peak and RMS differences were zero in
this local fixture, before and after filtering; this observation is not a
general bit-identity promise.

The [144 rows](../experiments/premium_filter/measurements/2026-10-07-rate-gain-complex-local-m1.csv)
and [source/measurement hashes](../experiments/premium_filter/measurements/2026-10-07-rate-gain-complex-local-m1-provenance.json)
retain the local result. All 102 Release CTest cases pass, and both complex
tests pass with ASan/UBSan. Windows/macOS research jobs also run the new
combined fixture before timing; their fresh exact-head gates remain required.

Next: repeat target-machine deadlines and resolve the high-rate cost/quality
policy, then native host acceptance and production latency/state/automation
integration. This qualification alone does not activate the filter or close
the preset/listening/release gates.
