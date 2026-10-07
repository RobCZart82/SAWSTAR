# Opt-in lookup saturation qualification

The earlier component measurements identify Drive/FIR/nonlinearity as a larger
isolated cost than the linear filter. This candidate changes only the tanh
evaluation. `FixedRatePremiumDrive` has a sixth, default-false `Lookup` argument;
default research aliases and the production engine do not select it. The existing
std::tanh and exp-based research tests retain their original contracts.

`LookupTanh` interpolates tanh with cubic Hermite polynomials on 2,048 intervals
of width 1/128 over [0,16]. Endpoint values use std::tanh; derivatives are
`1 - tanh(x)^2`. Four polynomial coefficients per interval occupy **64 KiB**
in one shared immutable table per binary. Init prepares the table, using the
standard thread-safe static initialization; Process does not initialize it,
allocate, lock or call tanh/exp. Each opted-in Drive retains one table pointer
(8 bytes on the measured 64-bit platform). Existing default object sizes
still satisfy the frozen-ring storage regressions.

Below magnitude 1e-8 the candidate returns x, preserving signed zero and tiny
inputs. At/above 16 it returns signed unity; NaN stays NaN. The polynomial
uses the original sign. This is a small approximation, not a mathematical or
bit-identical replacement for libm on every input. FIR coefficients, gain
slew, oversampling factors and the 32-host-sample latency are unchanged.

## Numerical gates

`premium_lookup_scalar_contract` tests 1,000,001 dense points on [-20,20],
every interval boundary and its adjacent representable doubles, powers of two
from the smallest subnormal through exponent 1023 with both signs, special
values, both branch boundaries, oddness, sampled monotonicity, copies/reinit
and shared-table determinism. Eight concurrent first Init calls exercise
actual shared-table construction; the Linux TSan job includes this case.

The separate scalar diagnostic bounds are `1e-10` absolute and `1e-8` relative
error vs std::tanh. Local Clang Release maxima were `3.9634962e-11` absolute
and `1.7287513e-10` relative. These are finite sampled controls, not a formal
uniform proof or an audibility threshold. The earlier ResearchTanh `5e-15`
contract is not weakened or replaced by this candidate.

`premium_lookup_drive_contract` compiles the existing 360-case Drive test
with the same SIMD FIR and reciprocal normalization on both paths. Only
lookup vs std::tanh differs. It covers 2x/4x, six rates 8–384 kHz, quiet to
loud sine/multitone/chirp/noise/impulse input, steady and modulated Drive,
clear, live copies, reinit, nonfinite channel recovery, channel isolation and
unity-gain impulse delay. The original `2e-7` peak/relative-RMS bounds and
`2e-8` normalized spectral-difference bound stay unchanged.

Local maxima:

| Cases | Peak difference | Relative RMS | Spectral difference |
| --- | ---: | ---: | ---: |
| 72 steady/stepped cases | 1.4901161e-8 | 2.4690566e-9 | 4.3952075e-12 |
| 288 modulation/signal cases | 5.9604645e-8 | 3.5503709e-9 | 5.8207661e-11 |

The first set projects six selected difference bins; the second checks every
one-sided FFT bin, including DC/Nyquist. These are spectra of the **difference**
from the existing Drive, not a measurement of its absolute alias energy.
Known DC/bin/Nyquist perturbations and a zero control validate the FFT guard.

All **105 local Release CTest cases pass**. The two added numerical cases and
reporter controls also pass ASan/UBSan. The subsequent #80 exact-head Windows/macOS, sanitizer and TSan checks all
passed. It merged as `bfa27e6`; that main also passed all three normal workflows.
These functional checks do not close the realtime or native-host gates.

## First isolated CPU result

Measured source `0eb50598b8193aad82b3dcb319d529dc907389ed`, local Mac mini M1,
Clang Release, NEON. Tests/compilation had finished before timing; no pinned
affinity or realtime priority. Eight alternating pairs per scene:

- Scalar: 65,536 precomputed inputs, sixteen timed passes, 0/12/20/24 dB gain.
  Median lookup/std::tanh ratios: 0.417245 / 0.392248 / 0.380938 / 0.380738.
- Drive: sixteen independent instances, 20 dB, 1,024 warmup and 16,384 timed
  stereo frames. The fixed normalized inputs are `.4*sin(n*.057)` and
  `.4*cos(n*.083)`, identical between paths. Both paths use SIMD and reciprocal
  normalization; table initialization and all allocation are outside timing.
  Checksum accumulation remains inside timing.

| Factor | 48 kHz | 96 kHz | 192 kHz |
| --- | ---: | ---: | ---: |
| 2x | 0.822794 | 0.819372 | 0.819542 |
| 4x | 0.901960 | 0.898657 | 0.901477 |

The complete isolated Drive time falls roughly 18% at 2x and 10% at 4x in
this run. All 48 Drive pair ratios were below one (range 0.810935–0.914152).
This is not a full-engine, callback deadline, cross-platform or listening
result. The scalar speedup does not translate directly into Drive savings;
the real Drive bypasses nonlinear evaluation at unity gain.

The [scalar pairs](../experiments/premium_filter/measurements/2026-10-07-lookup-local-m1-scalar.csv),
[Drive pairs](../experiments/premium_filter/measurements/2026-10-07-lookup-local-m1-drive.csv),
[metadata/source and measurement hashes](../experiments/premium_filter/measurements/2026-10-07-lookup-local-m1-metadata.json)
and [360 numerical rows](../experiments/premium_filter/measurements/2026-10-07-lookup-local-m1-drive-contracts.txt)
retain the observation. Compiler and full CTest logs remain in local
`outputs/premium-lookup-m1-20261007`. The reporter requires all 32 scalar and
48 Drive pairs and rejects 24 malformed-grid controls. Windows/macOS isolated
CI jobs retain numerical logs, exact-head source, compiler metadata, raw pairs
and report for 90 days; timings are descriptive, never pass/fail thresholds.

## Rate-scaled full-engine qualification, 2026-10-07

Measured code: `15f0a02b815b28f99270fdc213e2d5f138833ed3`. The new
`RateScaledLookupPremiumDrive` is a distinct opt-in alias. Both the candidate
and the std::tanh control use SIMD FIR, reciprocal gain, 4x below normalized
176,400 Hz / 2x at or above, and the existing 32-sample delay. The routed
candidate stores two additional table pointers (one in each fixed-factor
member); the shared table remains 64 KiB per binary. It is initialized before
warmup/timing. No factor or table construction occurs inside Process.

CMake builds the current Synth twice in different namespaces, with
`RateScaledGainEnginePremiumFilter` and `RateScaledLookupEnginePremiumFilter`.
Production sources, plugin, parameter IDs, GUI and Classic sound are unchanged.
The following additional contracts pass locally:

- Routing: fixed-factor lookup oracle is bit-identical, including invalid-rate
  normalization, 176.4 kHz boundary, clear, copies, reinit across both factors,
  nonfinite channel input and exact unity-gain impulse delay. Comparison to
  std::tanh keeps `2e-7` peak / `1e-7` relative-RMS bounds.
- Engine: 120 cases across 48/96/176.4/192/384 kHz, four filter modes,
  Poly/Mono/Legato and FX off/on. Poly starts with 16 voices. Shared control
  timelines cover 0/12/24 dB Drive, changing cutoff and dry/wet mix, 50%
  resonance, both LFOs, modulation rerouting, wheel/bend/pressure, sustain,
  stealing, fallback, same-note retrigger, release and reset silence.
- Both protected output and PreFX are compared; PreFX includes global LFO
  modulation and precedes effects/output protection. Each requires `2e-6`
  peak and `1e-7` relative-RMS error, audible reference energy and matching
  MIDI/voice state. An intentionally altered signal must fail the guard.
  This is a Synth API timeline, not VST3 automation or adapter lifecycle.
- The 144 high-rate OSC1/OSC2/SUB cases are also rerun against the independent
  8x Drive and fixed 4x control, including rapid cutoff and stepped Drive.
  The prior 1% unfiltered / 0.5% filtered / 0.1% 4x-to-8x ceilings remain.

| Control | Maximum peak difference | Maximum relative RMS |
| --- | ---: | ---: |
| Full engine output vs std::tanh engine | 1.4901161e-8 | 2.1051801e-8 |
| Engine PreFX vs std::tanh engine | 2.3841858e-7 | 1.9740532e-8 |
| Complex unfiltered vs std::tanh Drive | 2.9802322e-8 | 2.1477082e-9 |
| Complex filtered vs std::tanh Drive | 2.9802322e-8 | 1.0873274e-8 |

The worst filtered independent-8x relative error remains **0.490212%**,
192 kHz high-lead/HP12/24 dB, near the 0.5% diagnostic ceiling. This confirms
small added numerical error; it is not absolute alias-energy or listening
acceptance of the rate policy. **109/109 local Release CTest** cases and all
four new C++ ASan/UBSan cases plus the Python reporter case pass.

## First paired full-engine CPU observation

Local Macmini9,1 / Apple M1, Apple Clang 21.0.0, Release `-O3 -DNDEBUG`,
NEON, deployment target 11.0. All compilation/tests finished before timing.
No pinned affinity, realtime priority or native host was used. Both paths
hold 16 Poly voices, 20 dB Drive, 50% resonance and full FX. Controls are
stationary during timing; the modulation contracts above are not CPU tests.

The shared deadline harness measures four alternating pairs per scene,
48/96/192 kHz, four modes, buffers 32/64/128. Each path warms for 0.25 s,
then records 256 blocks. Allocation/Init are outside timing; sample buffer
stores remain inside and signal checks are outside. There are 36,864 timed
blocks per path. Every raw block and recomputed percentile/count is checked.

| Rate | Buffer | Median paired p50 ratio | Median paired p99 ratio | Reference misses /4096 | Lookup misses /4096 |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 48000 | 32 | 0.855033 | 0.842875 | 1 | 0 |
| 48000 | 64 | 0.854658 | 0.848240 | 0 | 0 |
| 48000 | 128 | 0.854770 | 0.861711 | 1 | 0 |
| 96000 | 32 | 0.857302 | 0.857978 | 2 | 0 |
| 96000 | 64 | 0.857673 | 0.863235 | 0 | 0 |
| 96000 | 128 | 0.858912 | 0.884183 | 0 | 0 |
| 192000 | 32 | 0.873506 | 0.890165 | 42 | 16 |
| 192000 | 64 | 0.873162 | 0.879498 | 10 | 11 |
| 192000 | 128 | 0.871986 | 0.872452 | 5 | 1 |

Medians improve roughly 13–15% in this observation. At 192 kHz total misses
fall from 57 to 28 of 12,288 blocks, but the 64-sample scene aggregate worsens
from 10 to 11. Worst observed block time remains 227.475% of the audio budget
(control 349.725%). These negative results remain; the CPU gate is open.

Permanent data: [deadline summaries](../experiments/premium_filter/measurements/2026-10-07-lookup-engine-local-m1-summary.csv),
[engine cases](../experiments/premium_filter/measurements/2026-10-07-lookup-engine-local-m1-engine.csv),
[complex controls](../experiments/premium_filter/measurements/2026-10-07-lookup-engine-local-m1-complex.csv),
[provenance and hashes](../experiments/premium_filter/measurements/2026-10-07-lookup-engine-local-m1-provenance.json).
Raw 73,728 block rows, compiled factors/backend, compiler description and
Release/sanitizer logs remain in local `outputs/premium-lookup-engine-m1-20261007`.
The deadline workflow adds independent Windows/macOS `rate-lookup` jobs,
including the numerical controls before timing and 90-day raw-data artifacts.
Its success validates measurements, not a portable CPU limit.

## Windows/macOS paired CI observation

Both new `rate-lookup` jobs completed successfully on exact source
`f95fd19e2a1de8a360ff3cd7e1bfeb62515dbfea`, workflow run `37674205117`.
The routed, 120-case engine and 144-case complex controls pass on both
platforms. The measurement workload and strict deadline counts are the same
as above; timings are hosted-runner observations, never acceptance thresholds.

| Platform | Rate | Buffer | Median paired p50 ratio | Median paired p99 ratio | Reference misses /4096 | Lookup misses /4096 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Windows SSE2 | 48000 | 32 | 0.911954 | 0.918663 | 2 | 0 |
| Windows SSE2 | 48000 | 64 | 0.913039 | 0.949396 | 0 | 3 |
| Windows SSE2 | 48000 | 128 | 0.918405 | 0.925582 | 1 | 0 |
| Windows SSE2 | 96000 | 32 | 0.922006 | 0.914266 | 33 | 2 |
| Windows SSE2 | 96000 | 64 | 0.921152 | 0.917360 | 2 | 9 |
| Windows SSE2 | 96000 | 128 | 0.910374 | 0.909661 | 53 | 22 |
| Windows SSE2 | 192000 | 32 | 0.955040 | 0.949570 | 4096 | 4088 |
| Windows SSE2 | 192000 | 64 | 0.963942 | 0.963995 | 4096 | 4096 |
| Windows SSE2 | 192000 | 128 | 0.949601 | 0.970213 | 4096 | 4096 |
| macOS NEON | 48000 | 32 | 0.851346 | 0.897313 | 8 | 8 |
| macOS NEON | 48000 | 64 | 0.861718 | 0.858584 | 23 | 10 |
| macOS NEON | 48000 | 128 | 0.845503 | 0.877115 | 16 | 19 |
| macOS NEON | 96000 | 32 | 0.874306 | 0.865243 | 181 | 195 |
| macOS NEON | 96000 | 64 | 0.856107 | 0.877316 | 132 | 90 |
| macOS NEON | 96000 | 128 | 0.884338 | 0.989094 | 39 | 124 |
| macOS NEON | 192000 | 32 | 0.891455 | 0.938876 | 433 | 264 |
| macOS NEON | 192000 | 64 | 0.893058 | 1.110693 | 751 | 535 |
| macOS NEON | 192000 | 128 | 0.884652 | 0.862234 | 577 | 194 |

Windows median improvements are about 8–9% at 48/96 kHz and 4–5% at
192 kHz. At 192 kHz **12,280 of 12,288 lookup blocks still miss the budget**.
macOS median improvements are about 11–15%; at 96 kHz total misses increase
from 352 to 409. The macOS 192 kHz/64-sample p99 ratio worsens to 1.110693.
Lower medians do not resolve these tail-latency risks or prove a regression
on a dedicated target machine. They require repeats and component profiling.

Permanent [Windows summaries](../experiments/premium_filter/measurements/2026-10-07-lookup-engine-windows-summary.csv),
[macOS summaries](../experiments/premium_filter/measurements/2026-10-07-lookup-engine-macos-summary.csv)
and [exact-head artifact/source provenance](../experiments/premium_filter/measurements/2026-10-07-lookup-engine-ci-provenance.json)
retain the observations. Raw blocks/compiler data and reports are also saved
locally in `outputs/premium-lookup-engine-ci-20261007` and in 90-day workflow
artifacts. Subsequent lookup jobs additionally preserve the verbose numerical
control log alongside the deadline files.

## Next gate

Review exact-head Windows/macOS/sanitizer results and paired deadline data.
Preserve slower/out-of-budget observations. Repeat on target machines, then
resolve the high-rate cost/quality policy and obtain native host acceptance.
Only then proceed to production filter replacement and latency/state/automation/
preset integration. The shipped sound and GUI have not switched to lookup.
