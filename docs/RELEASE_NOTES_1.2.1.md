# SAWSTAR 1.2.1

Published on 13 November 2026 from frozen build `b2e799802192f9bbdab90f55f2c2c52857d8fbd1`.
The archived 1.2.1 packages predate the public creator-identity correction.

## Maintenance changes

- Correct Windows installer documentation paths, retain the packaged `docs/`
  layout and verify installed document copies against the package payload.
- Normalize macOS third-party notice links for its installed documentation layout.
- Use an online reference for the historical checklist not shipped in installers.
- Synchronize version, minimum macOS version and copyright metadata across all
  three VST3 identities without changing their IDs.
- Require the relevant exact-commit CI workflows to succeed before preparing
  a verified release draft; preserve existing published releases and tags.
- Refresh both manuals for 1.2.1, correct the manual index and duplicate
  chapter-15 contents entry, and document the automation limitation below.

The calibrated Premium engine, 30 factory presets, 32-sample host latency,
parameter IDs, preset/state formats and GUI layout are retained from 1.2.0.
This maintenance preparation makes no DSP or preset-value changes.

## Known limitations

### Block-based parameter automation

Normal plugin parameters and host bypass use the final value in each audio
block. A short on/off change inside one block can be lost, and a value with
a later sample offset can affect the block from its beginning. DSP smoothing
does not restore discarded points. The separate host MIDI-controller path
retains individual sample offsets. Smaller host blocks reduce the timing
granularity; this release does not introduce sample-accurate normal-parameter
automation. This is a documented existing limitation, not a new DSP fix.

### K1 numerical issue

K1 remains unresolved. Three preserved offline negative cases cover Triangle
and Sine configurations. The original failing Sine executable was not retained;
later passing controls and the separate Chorus ring-boundary fix do not identify
or correct K1. The records do not establish a shared cause or audible defect.
Successful current tests do not invalidate the earlier negatives or prove a fix.
Historical evidence is linked from the
[1.2.0 release notes](https://github.com/RobCZart82/SAWSTAR/blob/main/docs/RELEASE_NOTES_1.2.0.md).

## Compatibility and installation

Windows x64 and ARM64 installers/manual archives and macOS Universal
PKG/DMG/manual archives are prepared by CI. Packages remain unsigned.
Match the plugin architecture to the DAW, close the DAW before updating and
preserve important projects, the previous plugin and user presets.
See [Installation](INSTALLATION.md) and [System requirements](SYSTEM_REQUIREMENTS.md).
Legacy Classic state remains readable, but the Premium filter changes its sound;
format compatibility does not provide identical Classic audio recall.

## Publication record

The frozen build passed all five exact-main workflows and 12 jobs. All seven
package files and SHA256SUMS.txt were verified before publication on
13 November 2026. The later UTF-8 tooling correction was a separate follow-up.
Prior macOS/Windows RC listening, loading and saving feedback does not establish
exact-final-build or native Windows ARM64 host acceptance. K1 remains unresolved.
