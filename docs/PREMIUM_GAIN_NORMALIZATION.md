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
