# Changelog

## 1.2.1-rc1 — test candidate, not published

The public stable release remains 1.2.0. Exact-candidate host testing and a
separate publication decision are still required; see the [candidate notes](docs/RELEASE_NOTES_1.2.1.md)
and [1.2.1 plan](docs/RELEASE_1.2.1_PLAN.md).

- Prepare the corrected Windows/macOS documentation layout and synchronized
  VST3 metadata for exact-candidate CI validation.
- Correct candidate manuals so they identify the public 1.2.0 release, point to
  the proper candidate chapter, and distinguish the six approved preset
  snapshots from exact-candidate acceptance.
- Preserve the unresolved K1 evidence and do not claim a numerical fix.

## Unreleased — 1.2.1 follow-up

Preparation order and publication gates: [1.2.1 plan](docs/RELEASE_1.2.1_PLAN.md);
local scope and validation: [release-preparation log](docs/DEVELOPMENT_LOG_2026_11_08_RELEASE.md).

- Correct Windows installer documentation paths and verify installed copies
  against the extracted package; normalize the macOS notice link for its flat
  documentation layout.
- Synchronize all product VST3 version plists from central release metadata,
  and make pre-release validation require every exact-head build, quality and
  workflow-lint run to finish successfully.
- Validate every relative Markdown link in the delivered Premium RC package;
  update the packaged Premium RC supplement's candidate links and historical
  acceptance wording; stage and validate the archive before exposing its
  final artifact filenames. Correct the same supplement's stale 1.2.0 About
  label and 24-preset/144-case description to match the 1.2.1 candidate's
  30-preset/252-case bank; make the UTF-8 documentation regression portable
  on Windows.
- Update the current-version manuals and K1 handoff references while preserving
  historical screenshots, evidence and already-published 1.2.0 assets.
- Add an opt-in offline K1 filter/FIR state probe with named bit-pattern snapshots,
  inline SIMD/SVF observation and five preceding samples. Preserve the ordinary
  comparison binary and bounded Rosetta evidence; this is not a K1 fix.

## 1.2.0 — 2026-11-07

- Draw the light About signature with native vector paths alongside the existing GYR mark.

- Release the calibrated Premium lookup filter engine with 32-sample latency,
  updated bilingual manuals, 30 built-in presets and the vector About signature.
- Avoid unused arpeggiator pattern expansion/sorting while bypassed (#137),
  preserving MIDI forwarding and rebuilding held roots when enabled.

- Wrap both chorus interpolation indices when floating-point rounding maps a
  read position to the ring length; prevent cross-channel/out-of-bounds reads
  exposed by the Velvet Orbit tail, preserving ordinary interpolation exactly.

- Expand the factory bank from 24 to 30 with expressive wheel/pressure leads,
  dual-LFO pads, synced bass, velocity keys and two tempo arpeggios; retain
  existing preset identities and values. Add playing hints, controller-aware
  pre-guard headroom checks and updated bilingual RC manuals.

- Refresh both PDF manuals with Premium migration, latency and accurately scoped
  K1 evidence; retain the labelled historical screenshots.
- Record verified RC artifacts and factory evidence; verify complete pinned
  dependency patches and checkout provenance before RC packaging, preserving
  the raw dirty flag and rejecting unexplained changes.

- Prepare 1.1.0 rc1 and an explicit Premium production integration build with
  the SAWSTAR identity, preserved parameters/state and 32-sample latency.
- Reuse accepted Premium factory trims; add production state/recall checks,
  native-package CI and RC-only packaging with explicit open acceptance gates.

- Migrate the remaining macOS 14 CI runners to macOS 15, including Preview,
  qualification and diagnostic workflows; keep the dispatch artifact pair aligned.
- Finish inaudible zero-target LFO depth tails before floating-point underflow,
  including fades from a previous modulation destination; avoid per-depth
  short-circuit branches in the new tail guard.
- Reuse Classic filter Drive gain/normalization only when the smoothed Drive
  value is exactly unchanged; retain per-sample updates during automation.
- Remove the unbounded process-local favorites mutex; retain timed filesystem
  locking and atomic replacement across threads and processes.
- Stop zero-target mixer, mod-wheel and pressure smoothing before float underflow (#118).
- Apply the sample-rate-relative frequency ceiling to the SUB oscillator (#119).
- Reject competing process-local preset UI mutations with a retryable busy response (#120).
- Add isolated Premium Reference/Candidate builds, package qualification and native
  host measurement kits; add deterministic MIDI inputs and GUI-scale project state.
- Add numerical diagnostics and replay/sanitizer controls. The K1 cause remains
  unknown; native CPU acceptance and production promotion remain open.

Local validation and remaining gates: [2026-11-02 audit fixes](docs/AUDIT_FIXES_2026_11_02.md).

## 1.0.4 - 2026-10-02

- Refresh an existing unpublished release draft from verified exact-commit packages; protect public releases and existing tags and mark interrupted refreshes incomplete.
- Link the README to the actual public Latest download.
- Index batch preset imports once under the library mutation lock, reducing repeated directory scans and preset reads while preserving filename collisions, exact duplicate checks and failed-save recovery.
- Synchronize editor note ownership when ARP transport stop clears its roots, so a later GUI release cannot stop a fresh host note of the same pitch.
- Deliver every VST3 MIDI-controller parameter point with its original sample offset, including sustain edges, pitch bend and channel aftertouch.
- Process accepted editor MIDI at sample zero so upstream overflow recovery cannot miss an editor onset deferred into a future block.

- Fix editor/host note ownership during overflow recovery, reset and editor close.
- Preserve final pitch/mod controller targets when MIDI queues overflow.
- Keep the synth MIDI/DSP timeline running during bypass while the host output is muted; retain overflow recovery across zero-frame calls.
- Complete long ADSR segments when float precision would otherwise stall.
- Report a conservative finite VST3 tail for the serial envelope/effect chain.
- Canonicalize enum/integer preset values to avoid false modified markers.
- Reject malformed standalone preset files without narrowing legacy DAW state compatibility.
- Identify development and RC builds clearly and refresh both user manuals.

No experimental click/pop treatment, filter-character change, additional voices or Linux plugin is included.

## 1.0.3 - 2026-09-19

Includes the unpublished 1.0.2 development work.

- Require successful matching platform and Quality checks, verified package contents and both manuals before preparing a release.
- Correct Voice Mode/Glide host grouping and refresh technical documentation.
- Candidate release preparation skips cleanly without creating a release.
- Fix combined Volume/Boost smoothing, detune continuity and initial Delay time.
- Share and verify production MIDI scheduling and parameter mapping.
- Reject non-finite benchmark data and package version-specific release notes.

- Resume from released 1.0.1; exclude experimental Mono crossfades and filter-start preparation.
- Fix unrendered same-sample chord pitches becoming glide origins.
- Reset the amplitude envelope only when allocating a genuinely idle voice.
- Resolve same-sample Mono releases once before audio processing.
- Retain Windows test-stack fixes. See docs/CHARACTER_PRESERVING_BASELINE.md.

## 1.0.1 - 2026-09-12

- Refine meter ballistics, CLIP display, keyboard glow, scales and typography.
- Initialize idle voice targets and Reset gain correctly; retain active-voice smoothing.
- Cache unchanged reverb settings; reduce duplicate wrapper work.
- Reject empty state; group host parameters without changing IDs.
- Protect Unicode preset-name collisions; improve confirmation text wrapping.
- Add deterministic stress tests and refresh bilingual manuals/screenshots.

See [1.0.1 release notes](docs/RELEASE_NOTES_1.0.1.md).

## 1.0.0 - 2026-09-11

First public release: VST3 for Windows x64/ARM64 and macOS Universal, with
24 embedded presets, English/Hungarian PDF manuals, installers and manual ZIPs.
