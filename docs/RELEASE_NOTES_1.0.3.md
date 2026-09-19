# SAWSTAR 1.0.3 — development notes (unreleased)

This is a development candidate, not a published release. Installer and host
acceptance must use the eventual release build before publication.

## Reliability fixes

- Mono/Legato glide starts from a pitch that has actually rendered, rather than
  an earlier unrendered note in the same incoming chord.
- Idle voice reuse starts its amplitude envelope cleanly.
- Release-only Mono MIDI groups resolve their final held-note selection before
  the next audio sample.
- Compensating Volume and Boost changes no longer produce a temporary gain swell.
- Frequency updates preserve the existing detune smoothing trajectory.
- Delay initializes its requested time immediately; subsequent edits still smooth.

## Verification and maintenance

- The plugin and engine tests share the production parameter mapping; an
  independent frozen reference checks output equivalence and state roundtrips.
- The actual fixed-capacity MIDI queue is tested at block boundaries, on overflow,
  with varying block lengths, and at 44.1/48/96 kHz.
- Benchmark reports reject non-finite measurements instead of accepting them.
- Distribution archives select these notes from the version in `release.json`.

The GUI layout, parameter IDs and preset/state format are unchanged. Experimental
RC7 click/pop treatments and changes to the established synthesis character are
not included. No claim is made that all audible Mono transitions are eliminated.

Candidate release preparation now exits successfully without creating a release.
The 1.0.2 development work is included here; there was no public 1.0.2 release.
