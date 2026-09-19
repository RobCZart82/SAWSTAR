# 1.0.3 candidate preparation

Development branch: `codex/release-103-preparation`, based on main `91e372a`.
No merge, final tag or public release is part of this work package.
The GUI layout and synthesis engine are unchanged from this main baseline.

## Milestones and acceptance gates

1. **Candidate scope:** 1.0.3-rc1 metadata, notes and changelog. Includes the
   unpublished 1.0.2 reliability work; excludes RC7 sound-character experiments.
2. **Release guard:** candidates exit successfully before accessing GitHub or
   creating any release. Regression fails on the old implementation. Final
   releases still require verified matching builds and release context.
3. **Host acceptance:** use copies of old projects and verify the actual About
   version. Local candidate load and About verification passed. A legacy automated
   project and its re-saved copy rendered bit-identical 24-bit PCM in REAPER
   7.80. A copied five-instance demo rendered 59.68 seconds successfully.
   MAIN/ADVANCED/PRESETS switching and preset loading during playback worked.
   Extended occlusion and additional-host acceptance remain open. Earlier 1.0.1 listening feedback is
   not acceptance of this candidate.
4. **Behavior policy:** preserve stable MIDI arrival order and existing LFO
   first-key retrigger semantics. Added full-engine coverage for LFO order and
   all six active Poly/Mono/Legato mode transitions at 44.1/48/96 kHz. Existing
   controller_lifecycle covers CC120/121 and live envelope edits. FX tails and
   protected-output CLIP semantics are documented; no speculative DSP changes.
5. **Verification:** local complete 54-test suite and ASan/UBSan both passed 54/54;
   real VST3 validator passed 47/47. Seed 103 with 100,000 stress blocks per
   sample rate passed at 44.1/48/96 kHz. Exact candidate Windows x64/ARM64, macOS Universal and Linux
   quality CI must pass. Final installer/host and user listening acceptance
   remain release gates; a successful compile does not replace them.
6. **Documentation and distribution:** update both 13-page PDF manuals,
   README, candidate notes and manual build source. Existing installation guide
   retains bilingual unsigned-package guidance and removal instructions.
   Historical 1.0.1 screenshots are explicitly labelled; the candidate About was visually verified as 1.0.3 Pre Release / local.
   A saved, cropped release screenshot remains a final presentation task.

## Deferred beyond this candidate

A new filter character, 32 voices, automatic pitch smoothing, DC resets and
changes to Mono articulation require separate designs and listening approval.
The closed click/pop investigation is not reopened by this release preparation.

## Publication boundary

Only after candidate evidence and listening acceptance: final metadata, final
platform builds, checksums/installers, merge/tag and verified release draft.
Those actions belong to milestone 7 and require the next instruction.
