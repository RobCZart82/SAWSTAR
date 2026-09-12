# 1.0.1 release review

Review date: 2026-09-12. Preview source: b53772f (merged stress test).

## Scope and result

Reviewed the changed 1.0.1 wrapper/reset/state handling, meter/Scope handoff, idle voice initialization, reverb cache, preset Unicode and mutation locking, confirmation rendering, and release scripts. No confirmed new runtime defect was found in this pass; no runtime or GUI geometry change is included in release preparation. This is a targeted review, not a proof of defect-free software.

Corrected release tooling: version-specific notes, generic candidate-name rejection, archive manifest commit/version and per-file checksum verification, draft-only publication, removal of the one-off 1.0.0 finalizer. Final public publication remains a separate action after artifact checks.

## Evidence

- Existing preview CI: macOS and Windows Debug/Release plus Universal, x64 and ARM64 VST3 checks passed.
- New local Apple Silicon AddressSanitizer/UndefinedBehaviorSanitizer stress run: seed 0x101, 100000 blocks each at 44100/48000/96000 Hz, 90953580 stereo samples total, peak 0.98; no sanitizer reports.
- Additional sanitizer tests for meter, scope concurrency/reset, state decoding and preset reliability passed locally.
- The author reports Windows and macOS installer tests and four simultaneous SAWSTAR instances in a demo render with no observed failure. This is user-reported evidence, not an agent-performed test on every target OS.
- English and Hungarian 12-page manuals regenerated for 1.0.1; all pages rendered and visually reviewed. Images are supplied real preview captures and retain their capture labels.

Final commit CI and draft artifact verification passed before public publication. Installers remain unsigned; macOS is not Apple-notarized. Existing 1.0.0 downloads are preserved.

## Final artifact check

Final source: `50ba83a0c23ef4d5b856a2f71874975575b3fe0a` (PR #4).
The macOS artifact digest and every manifest-listed package file matched. The
Universal VST3 loaded in an isolated REAPER 7.79 session on Apple M1; MAIN,
ADVANCED, PRESETS and About displayed correctly, and project playback produced
meter activity. About identified `Version 1.0.1 / 50ba83a` with no Pre Release
label. This was a smoke check, not a fresh exhaustive listening test.

The publisher was also exercised with mocked GitHub responses: valid inputs
prepared a draft only; mismatched commit, version and file checksum were each
rejected. This checks control flow and does not substitute for the live workflow.

All seven final CI jobs passed, followed by the verified-draft workflow. Windows
x64/ARM64 artifact digests, per-file manifests and all installer/archive sidecar
checksums were also verified locally. Release `v1.0.1` was published on
2026-09-12 from the final source above, with eight uploaded assets (plus GitHub's
automatically generated source archives). The 1.0.0 release was preserved.
