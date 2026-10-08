# Modulated premium engine deadline study

The #81 lookup engine passed functional qualification, while hosted Windows
192 kHz measurements still exceeded the audio budget in all 12,288 candidate
blocks on the final head. A faster median does not resolve those misses.
The next diagnostic measures control changes and LFO modulation during the
same paired SIMD/reciprocal/std::tanh versus lookup workload.

## Workloads and controls

The existing `--deadline` path now records `stationary-v1` in `workload.txt`.
The separate `--deadline-modulated` path records `modulated-v1`. Both retain
sixteen held Poly voices, FX, four modes, 48/96/192 kHz, buffers 32/64/128,
four alternating pairs and 256 timed blocks per path/pair. Allocation, Init
and quarter-second warmup remain outside timing. Raw 73,728 block rows and
288 summaries include p50/p95/p99/maximum and strict over-budget counts.

`DeadlineModulation.h` defines the shared sample-index timeline. Both engines
start from sample zero, advance through warmup, and continue the same timeline
in timing. Events are independent of buffer boundaries:

- Every 128 samples: cutoff 500..6500 Hz, 50% resonance, dry/wet 35% or 100%.
- Every 512 samples: Drive 0/12/24 dB, wheel, pressure and pitch bend.
- Every 2048 samples: a modulation route/source-weight change.
- Both LFOs and three modulation slots are prepared before warmup.

Control setters, MIDI dispatch, timeline checks and synthesis are included in
the timed sample loop. Output validation/diagnostics run outside each timer.
Both paths receive identical events. This measures a Synth API workload,
including its dispatch cost, rather than VST3 automation or host callbacks.
It does not isolate individual components or exercise note lifecycle changes.
The CSV `drive_db=20` field identifies the initial setup; workload metadata
explicitly records the changing 0/12/24 dB targets, rather than presenting
20 dB as the modulated stream's fixed Drive value.

`premium_deadline_timeline` independently observes setter values/counts,
repeat determinism, identical event lists for all three buffers, non-event
samples and the continuation after the 48 kHz warmup offset. It needs no DSP
dependency. `premium_lookup_modulated_deadline_fixture` compares both real
engines for the complete warmup and the longest timed sequence in every
rate/mode scene. Protected outputs must be finite/bounded, voices remain 16,
and post-warmup error stays within `2e-6` peak and `1e-7` relative RMS.
The same bounds apply separately to PreFX, before effects/output protection,
so the protected output does not hide a larger upstream numerical error.
Existing lookup routing, engine timeline and independent 8x checks also run
before the paired CI measurement.

The reporter requires the compiled workload to match
`SAWSTAR_PREMIUM_WORKLOAD` (default `stationary-v1`). It rejects unknown,
missing and mismatched labels. Workload, timed-control status and exact raw
measurement byte hashes are retained in metadata. The existing routing and
summary/raw consistency checks still apply. Older artifacts without the
workload marker must be examined with their original reporter/source revision;
they are not silently reclassified by this reporter.

## Validation and next steps

The Windows local review builds/runs the dependency-free timeline test and
the deadline reporter tests: one C++ Release test and four Python tests pass.
The full engine requires the pinned DaisySP
dependency and remains a fresh Windows/macOS and sanitizer CI gate; this
snapshot has not supplied a local full-engine run or timing result.

The deadline workflow adds distinct Windows/macOS modulated lookup jobs.
Stationary and modulated artifact names include their workload version.
No timing threshold makes a job green or red: success qualifies the fixture
and data format, not realtime performance. Compare p99 and misses as well as
median; retain slower observations and repeat on controlled target machines.

After these measurements, profile components on Windows to attribute cost,
then evaluate high-rate policy/quality and native REAPER deadlines. Production
filter replacement, latency/state/automation and presets follow acceptance.
This change adds offline tooling only; it does not activate a production filter.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DSAWSTAR_CHECK_DAISYSP=ON
cmake --build build --config Release --target sawstar_premium_lookup_deadline sawstar_premium_deadline_timeline_tests
ctest --test-dir build -C Release -R '^(premium_deadline_timeline|premium_lookup_modulated_deadline_fixture|premium_deadline_report)$' --verbose
# Windows executable is build/Release/sawstar_premium_lookup_deadline.exe.
build/sawstar_premium_lookup_deadline --deadline-modulated deadline-results
SAWSTAR_PREMIUM_STUDY=rate-lookup SAWSTAR_PREMIUM_WORKLOAD=modulated-v1 python scripts/report-premium-deadline.py deadline-results
```
