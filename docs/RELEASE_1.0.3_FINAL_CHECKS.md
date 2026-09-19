# 1.0.3 final checks

Baseline: main 2b37bb1. Branch: codex/release-103-final-checks.
The user authorized development through publication, conditional on successful
validation and their listening acceptance of the final candidate.

## This bounded release patch

- Require exact-commit push success for macOS, Windows and Linux Quality before
  draft creation. Newest workflow runs supersede older successes.
- Verify the complete archive member set, duplicates, names, hashes and manuals.
- Correct Voice/Glide host grouping; no parameter ID/GUI geometry changes.
- Resolve the six confirmed GCC misleading-indentation locations without
  changing expressions or execution order.
- Update PERFORMANCE/ARCHITECTURE and retain original synthesis character.

## Candidate acceptance completed

- rc2 source: `0a6c21fd7813698baecbb0fb87ea06279256dfba`; merged main:
  `79a0bfc082c29c2f9d034ac9a037ba6d0e11a534` (same source tree).
- Local Release and ASan/UBSan: 54/54 each; real VST3 validator: 47/47.
- Windows x64/ARM64 and macOS Universal CI, installer checks and Linux Quality
  passed. Matching merged-main workflows also passed.
- Copied legacy project and five-instance REAPER demo rendered bit-identically
  to rc1. Private project/audio/license files are not distributed.
- On 19 September 2026 the owner confirmed rc2 listening plus editor
  occlusion/minimize/reopen acceptance. This is current-candidate feedback.

## Final publication gates

- Final metadata, English/Hungarian manuals and installation documentation.
- Completed: four owner-supplied 1.0.3 screenshots, including About, captured
  from build `8040390`; incorporated into the repository and bilingual manuals.
- Successful exact-commit platform/Quality CI, final installers and hashes.
- Verify the complete draft before publishing. Publication is authorized by
  the owner when these checks are satisfied; no DSP or GUI-layout changes.

## Deduplicated audit backlog (not silent 1.0.3 changes)

1. Instrument preset atomicity and VST3 automation point handling. Current
   block-level control snapshots do not prove atomic preset application.
2. Separate strict preset import from compatible host-state decoding; specify
   discrete rounding before any migration or duplicate-comparison change.
3. Delay re-enable with empty history: reproduce before changing smoothing.
4. Filesystem policy: symlinks, portable race-safe rename, directory durability,
   backup retention with recovery guarantees; no automatic backup deletion now.
5. Wheel double-click: define immediate click versus reset gesture semantics.
6. Benchmark actual wrapper controls/telemetry plus ordered/reverse/random MIDI
   and dense arpeggiator rebuilds before introducing caches/optimizations.
7. Windows cross-version dual-architecture installs and local build-ID refresh.
8. Target risk branches and investigate coverage-tool warnings; line coverage
   is not whole-plugin or GUI acceptance. Confirm unused controls before removal.
9. Separate sound-quality project: filter/Drive oversampling, triangle boundary,
   inactive waveform phases and 32 voices, each with CPU/compatibility/listening.
10. Linux VST3 is a future platform, not a supported-product regression.

The click/pop investigation stays closed. Controller/LFO ordering, protected
CLIP semantics, mode articulation and key tracking retain their documented policy.
