# Repository review — 2026-10-09

Reviewed main: `9cf0ba003935ee01797f5daa92f36f4a11595830`.
Also reviewed the pending #93 research changes at
`cf8a7b1dadc654be24815ac90b3d5d4388175707`; its CI budget follow-up is
`8702c70ff31358dfa44832bf32d7173c22d916fd`.
These are distinct sources: an open research PR is not shipped production code.

## Scope and evidence

The repository-wide pass covers production DSP/voice/MIDI, host parameter/state
and editor boundaries, preset files/import/lifecycle, research algorithms and
reporters, CMake/tests, workflows, packaging/release guards and development plans.
At the reviewed #93 snapshot, the inventory contains 71 `src/` files, 108 test
files, 22 scripts, 14 workflows and 126 Markdown documents. All tracked Python
files parse; the Markdown local-target check found no missing linked files.
This is not an exhaustive proof of every branch or a line-by-line audit of
upstream dependency implementations. DaisySP and iPlug2 remain pinned at
`599511b` and `d54f690`; no dependency update is part of these fixes.

## Confirmed defects and fixes

| Finding | Trigger and evidence | Action |
| --- | --- | --- |
| Classic filter mode tails enter denormal arithmetic | Hold a note after changing LP12 to another mode. The abandoned double-precision weight decays into subnormal values after roughly seven seconds and can stall there. At 48 kHz the reproducer reaches `1.18576e-321`; a nine-second sustained-input test raises `FE_UNDERFLOW` with the old code. | End inactive weights below `1e-24`, transferring their residual to the selected mode. Keep the original fade recurrence and warm filter histories. The new regression rejects the old source and passes with the fix, including an unoptimized build. |
| Full Debug/instrumented jobs exhaust CI budgets | At `cf8a7b1`, two Windows Debug jobs stop at about 20 minutes; Linux coverage stops at about 35 minutes. Their logs reach `engine_controls`, with preceding tests passing and no recorded assertion failure. Earlier macOS Debug attempts also exceeded 25 minutes. | #93 gives Windows/macOS Debug 45 minutes and coverage 60 minutes. Existing commands, per-test limits and numerical/CPU thresholds remain unchanged; Release/VST3/sanitizer budgets are retained. |
| Current progress is split between PR text and older roadmap wording | #92 is already merged; #93's `a351db4` measurement campaign is complete, while its original research notes still describe that campaign as future work. A successful numerical campaign does not close the CPU gate. | Put a current status paragraph before the roadmap history and link the authoritative order from README. Retain historical measurements and distinguish completed collection from acceptance. |

The Classic repair is independent of the experimental premium replacement.
It adds no parameter, state field, latency or input clipping. Ordinary mode
crossfades retain their algorithm; only an inaudible residual below `1e-24` is
ended. Host flush-to-zero behavior must not be the only protection against this
state. The regression checks floating-point underflow rather than a noisy CPU
threshold. It does not claim a portable speedup.

A separate old-source/new-source comparison covers all four target modes at
8/44.1/48/96/192/384 kHz, rapid retargeting followed by a long held mode, changing
cutoff/resonance and 12 dB Drive. All 24 cases / 27,795,600 stereo frames have
zero observed float output difference. This local fixture is additional evidence,
not a native host or universal bit-identity certification.

## Corrections to the supplied filter report

| Claim | Current source/test result |
| --- | --- |
| One channel's NaN contaminates the other | Not reproduced. `EnginePremiumFilter`, `PremiumDrive` and `PremiumLowPass` clear only the affected channel. Existing Drive and four-mode lifecycle tests compare the intact channel against an untouched reference. A global reset would discard valid opposite-channel history. |
| Protect every input by clipping to ±0.98 | Not an appropriate generic repair. Internal oscillator/mixer samples may exceed the final output ceiling; clipping them changes the intended Drive/filter sound. The output guard belongs at the engine output. |
| The premium adapter adds 32 ms | Its latency is **32 samples**: about 0.667 ms at 48 kHz and 0.167 ms at 192 kHz. Production latency/state/automation integration is still open. |
| Adaptive 2×/4× and compact FIR storage are future work | Both already exist as separately tested research paths. The interpolator uses 64 host-history slots with mirroring. The downsampler needs 65/129 retained values at 2×/4×, so a general 64-slot replacement cannot preserve its current FIR contract. |
| BP12 normalization is an unresolved missing implementation | The premium BP12 uses the damping-scaled band output and independent complex transfer-function tests. The supplied approximate error is not a reproduced current failure. Native listening remains a separate gate. |
| LP24 at cutoff is already −3.01 dB in the shipping engine | The premium candidate targets fourth-order Butterworth behavior. The Classic LP24 is characterized separately at gain 0.25, approximately −12.04 dB at cutoff without resonance. Those are different sound paths, not interchangeable measurement labels. |
| The stress harness is limited to 1–4 seconds | It accepts up to one million variable-length blocks per rate. That capability does not prove an hour-long host session was run. A native soak test remains open. |

## Actual CPU and numerical progress

Re-read all four completed #93 jobs in
[run 37918754921](https://github.com/RobCZart82/SAWSTAR/actions/runs/37918754921).
Each contains four separate `DEADLINE_CI_REPORT` payloads identifying
`a351db48d46c432171165aced36e9b4a5be93e80` and its waveform/workload.
All 16 grids completed with the numerical gates. The following are descriptive
SAW results at 192 kHz across 32/64/128-sample buffers:

| Platform / workload | Paired median time ratio, study/reference | Deadline finding |
| --- | --- | --- |
| Windows stationary | 0.972900–0.972980 | Both paths miss all 4096 measured blocks per buffer. |
| Windows modulated | 0.969661–0.970007 | Both paths miss all 4096 measured blocks per buffer. |
| macOS stationary | 0.952806–0.966672 | Mixed miss counts; 32-sample misses increase from 33 to 96. |
| macOS modulated | 0.953926–0.963336 | At 32 samples, p99 ratio is 1.035033 and misses increase from 80 to 170. |

Alternative waveform regressions also remain. No outliers are removed, no
control timing is subtracted, and no threshold is relaxed. A roughly 3–5%
SAW median reduction does not establish acceptable p99, deadline behavior or
native realtime performance. The original raw CSV/compiler artifacts were not
independently downloaded in this review; the table is verified job-log summary
evidence. Later CI-only commits must not be substituted for the measured source.

## Release gates and order

The accepted [roadmap](DEVELOPMENT_ROADMAP.md) fits the major unfinished work.
Its long [development plan](DEVELOPMENT_PLAN_POST_1.0.4.md) remains a history and
evidence log, rather than a list in which every older “next” task is still open.

1. Finish exact-head functional CI and merge the validated bug fixes/research.
2. Close step 3 using controlled, source-identical target-machine CPU repeats,
   including small-buffer p99/misses and all waveform paths; select further cost
   changes from the measured component profile if acceptance still fails.
3. Decide high-rate quality policy and perform the early Windows/M1 REAPER check.
4. Integrate the accepted premium path with existing IDs/GUI, compensating
   latency and verifying old projects, state, preset transactions and automation.
5. Complete native import/editor/unload/multiple-instance/external-volume QA;
   balance factory presets on the final sound path.
6. Validate the production RC and packages/manuals/version, then make a separate
   release-publication decision. No next release version is assigned here.

Block-level parameter application and individual GUI preset gestures are
documented limits in `ARCHITECTURE.md`, not proven sample-accurate or atomic
transactions. Worker import tests verify ownership, retained results and joining,
but not native UI/unload timing on every host. These belong to the remaining
integration/lifecycle gates, not to a claim that the already tested engine is
broken in those scenarios. Dependency updates, Linux VST3 and 32 voices remain
deferred.

## Verification and limits

The reviewed #93 source plus the Classic fix builds with GCC 13.3 / CMake 4.4.4
Release. All 122 registered CTests pass across the full run and targeted retries;
four generated executable permission bits needed restoration in the local
workspace. The filter underflow regression passes in optimized and unoptimized
builds and fails against the original `LowPass.cpp`. Pinned actionlint 1.7.12
passes all workflows with its distribution checksum verified. Release metadata
consistency and tracked Python parsing pass.

The independent main-based fix branch also builds in Release: all 120 registered
CTests pass across the full run and one targeted retry after restoring another
local executable permission bit. Its `filter_character` regression also passes
in a separate CMake Debug build. Windows/macOS VST3 builds, installer and sanitizer
outcomes remain subject to fresh PR CI, not claimed as locally executed on this
Linux workspace. No native
REAPER listening, audio-device deadline, hour-long host soak, coverage percentage,
new release artifact download or release publication was performed here.
