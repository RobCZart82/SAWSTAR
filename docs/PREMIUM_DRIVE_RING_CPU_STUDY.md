# Capacity-only Drive CPU study

The [storage reduction](PREMIUM_DRIVE_RING_STORAGE.md) in #75 is merged as
`1e99508dc0564dbd84190b7fe7c41beff6e9fb68`, with every PR check successful.
It proves a 4,096-byte saving per stereo 2x Drive. This study asks whether that
smaller object also reduces isolated Drive wall time.

## Comparison and controls

The reference is frozen from `c5f5d6351b7dbed9e30af329c99934dcda5cc7f6`,
immediately before the ring reduction. Its namespace and include qualification
are adjusted for simultaneous compilation. Its phase dispatch, arithmetic,
coefficients, smoothing and original 256-slot decimator ring are retained.
The shared `FirLanes4` and `ResearchTanh` primitives are unchanged between that
commit and the study base. Both measured nonlinear paths use `std::tanh`.

Each implementation is compared with its own predecessor: scalar versus scalar,
SIMD versus SIMD, at fixed 2x and 4x factors. The unchanged 4x capacity is a
control. Earlier runtime-phase or scalar/SIMD comparisons would mix independent
changes into the storage question and are not used as the timing reference.

- 48, 96 and 192 kHz; sixteen stereo Drive objects, 20 dB, 16,384 input frames.
- 1,024 warmup frames per object; allocation, Init and warmup outside the timer.
- Eight pairs per case, alternating reference/candidate execution order: 96
  CSV rows in total. A finite, exactly matching paired checksum is required.
- 513,048 stereo frames bit-identical in the local capacity fixture, including
  wraps, gain changes, clear/snap, invalid channels, copied live state, impulses
  and reinitialization at six sample rates. Impulse peak remains at sample 32.
- The reporter verifies the complete grid, positive finite timings, matching
  storage sizes and expected reductions. Eighteen malformed-data controls fail.

`premium_drive_ring_identity` and `premium_drive_ring_report` run with normal
Windows/macOS and sanitizer testing. The standalone target needs no DaisySP.
The separate `Premium ring storage study` workflow collects Windows/macOS
Release timing, actual checked-out commit, source/CSV hashes, compiler metadata,
backend and eight-pair summaries. Artifacts are retained for 90 days. Timing
ratios are descriptive; a slower candidate still produces a valid report.

## First local result: 2026-10-07

MSVC 19.44.35229.0, Windows x64 Release, SSE2 backend. The local run used
uncommitted working files: provenance records their exact byte hashes, rather
than assigning them the base commit SHA. Every row passed validation; both new
CTest checks passed. Values below are medians of candidate/reference wall time;
below one means shorter time. Pair ranges are available in the provenance.

| Implementation | Rate | 2x smaller ring | 4x unchanged control |
| --- | ---: | ---: | ---: |
| Scalar | 48,000 | 0.996131 | 0.997399 |
| Scalar | 96,000 | 1.009910 | 0.997021 |
| Scalar | 192,000 | 0.982133 | 0.991493 |
| SSE2 | 48,000 | 1.014619 | 0.994192 |
| SSE2 | 96,000 | 0.982056 | 1.000940 |
| SSE2 | 192,000 | 0.997404 | 1.003730 |

No consistent speedup follows from this run. Scalar 96 kHz and SSE2 48 kHz
were slower; individual pair ranges overlap unity and the unchanged control
also varies. Keep the proven memory saving, without a CPU-speed claim.

[All 96 local pairs](../experiments/premium_filter/measurements/2026-10-07-ring-local-windows.csv)
and [local provenance](../experiments/premium_filter/measurements/2026-10-07-ring-local-windows-provenance.json)
are retained in the repository. Compiler description hashes identify the local
build metadata; full compiler descriptions accompany the CI artifacts.

## Limits and next gate

This is an isolated Drive microbenchmark with identical input for each object,
without affinity or realtime scheduling. It does not measure complete-engine
block deadlines, host callbacks, audio dropouts or native REAPER acceptance.
Compiler code placement and runner load can affect even the unchanged control.
Windows/macOS repetitions and full-engine/target-machine work are still needed;
the high-rate policy and production filter integration remain open.

```sh
cmake -S . -B build-ring -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DSAWSTAR_CHECK_DAISYSP=OFF
cmake --build build-ring --config Release --target sawstar_premium_ring_study
ctest --test-dir build-ring -C Release -R '^premium_drive_ring_(identity|report)$' --verbose
```

The CI workflow shows platform-specific executable paths and capture commands.
