# Opt-in lookup saturation qualification

The earlier component measurements identify Drive/FIR/nonlinearity as a larger
isolated cost than the linear filter. This candidate changes only the tanh
evaluation. `FixedRatePremiumDrive` has a sixth, default-false `Lookup` argument;
no normal research alias or production engine selects it. The existing
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
reporter controls also pass ASan/UBSan. Fresh Windows/macOS and Linux TSan CI
are still required before merge; local results do not close those gates.

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

## Next gate

Review exact-head Windows/macOS qualification and measurements. Then add a
distinct combined rate-scaled engine probe and compare against the unchanged
SIMD/reciprocal/std::tanh engine, with sounding/modulation/reference quality
and small-buffer deadline controls. Preserve slower/out-of-budget results.
Only after target-machine repeats, high-rate cost/quality policy and native
host acceptance can the production filter and latency/state/automation/preset
integration proceed. The shipped sound and GUI have not switched to lookup.
