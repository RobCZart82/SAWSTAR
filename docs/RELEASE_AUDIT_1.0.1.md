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

Final commit CI and draft artifact verification must pass before public publication. Installers remain unsigned; macOS is not Apple-notarized. Existing 1.0.0 downloads are preserved.
