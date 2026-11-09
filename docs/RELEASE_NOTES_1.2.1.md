# SAWSTAR 1.2.1-rc1 — unpublished test candidate

This is a test-candidate document, not a final release notice. The public
release remains SAWSTAR 1.2.0 until a separately approved 1.2.1 publication.
The 1.2.1-rc1 packages must be identified by their exact source commit and
artifact hashes. Do not use them for important projects before host testing.

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

## Candidate acceptance remains open

The creator has approved listening to the six exported snapshots for the new
factory presets. That does not accept this exact 1.2.1 candidate. The candidate
still requires its exact Windows and macOS packages to be tested in the intended
hosts, and a separate explicit publication decision. CI, offline tests and
snapshot listening are not substitutes for those steps.
