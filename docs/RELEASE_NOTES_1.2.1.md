# SAWSTAR 1.2.1-rc1 — unpublished test candidate

This is a test-candidate document, not a final release notice. The public
release remains SAWSTAR 1.2.0 until a separately approved 1.2.1 publication.
The 1.2.1-rc1 packages must be identified by their exact source commit and
artifact hashes. This candidate is not a stable release.

## Maintenance changes under evaluation

- Use an online repository reference for a historical checklist that is not
  shipped in the installer documentation.
- Keep Windows installer documents in the same `docs/` layout as the packaged
  payload, including the bilingual manuals and version-specific release notes.
- Check the installed document copies against the package payload in the
  disposable Windows installer test. User presets must remain untouched.
- Normalize the macOS third-party notice links for the flattened installed
  documentation directory.
- Keep version, minimum macOS version and copyright metadata synchronized
  across the Classic, Premium Preview and Premium Candidate VST3 identities.
- Require all relevant exact-commit CI workflows to complete successfully
  before preparing any future release draft.

There are no intended DSP, preset, parameter-ID, state-format or default-engine
changes from 1.2.0 in this candidate. Package and manual changes are still
subject to exact-candidate validation.

## K1 numerical issue

K1 remains unresolved. Three preserved offline negative cases cover Triangle
and Sine configurations. The original failing Sine executable was not retained;
later passing controls and the separate Chorus ring-boundary fix do not identify
or correct K1. The logs do not establish an audible defect. Keep all preserved
negatives and non-reproductions distinct; do not describe K1 as fixed.

## Candidate host testing and publication status

Robert reports that he tested SAWSTAR 1.2.1 on both macOS and Windows,
including listening, loading and saving, and found it working correctly. The
reported host test is user feedback; no OS/app versions, Windows architecture,
detailed CPU/dropout measurements or test matrix were supplied. Do not infer
those results from this statement.

This file and the packages still identify an unpublished `1.2.1-rc1` candidate.
A separate explicit decision is required before changing to stable metadata or
publishing. K1 remains unresolved; successful host testing does not establish
that the offline K1 numerical mismatch is fixed. Keep the known issue disclosed
in any final release notes.
