# SAWSTAR 1.0.3

Release edition: 19 September 2026. This maintenance update includes the
unpublished 1.0.2 reliability work and preserves the established interface and
synthesis character.

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

- Final draft preparation requires matching Windows, macOS and Linux Quality
  success. Archives reject unlisted, missing, duplicate and unsafe members.
- Voice Mode and Glide use the Performance host parameter group.
- Technical architecture/performance notes now describe the current engine.
- Misleading-indentation warnings in project code are removed without changing
  processing order or sound behaviour.

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

## Packages and updating

Windows x64 and native ARM64 installers and manual VST3 ZIPs are supplied
separately. macOS Universal supports Intel and Apple Silicon, with a DMG/PKG
installer and a manual VST3 ZIP. Both English and Hungarian PDF manuals are
included. Linux VST3 distribution is deferred.

Close the DAW before updating and back up projects and user presets. Verify
the About version after restarting. Installers remain unsigned; macOS is not
Apple-notarized. Follow [Installation](https://github.com/RobCZart82/SAWSTAR/blob/main/docs/INSTALLATION.md) for package approval,
architecture matching and removal. Build targets and tested-system limits are
listed in [System requirements](https://github.com/RobCZart82/SAWSTAR/blob/main/docs/SYSTEM_REQUIREMENTS.md).

The release assets include SHA256SUMS.txt. Use the matching checksum for the
file you downloaded. Factory sounds are embedded; no extra bank is required.
