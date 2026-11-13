# SAWSTAR 1.2.2

Creator-identity maintenance release. Preparation and publication were authorized
on 13 November 2026. Publication follows successful exact-commit checks and
verification of the final Windows x64/ARM64 and macOS Universal packages.

## Changes

- Use RobCZart82 in documentation, PDF text and author metadata, legal creator
  credits, installer publisher and synchronized plugin copyright metadata.
- Remove the historical About screenshot containing a typeset private identity.
  The vector creator signature and GYR monogram remain the visual identity.
- Include the UTF-8 metadata synchronization correction merged after the frozen
  1.2.1 build; locale-specific encodings cannot corrupt generated metadata.
- Regenerate English/Hungarian manuals and retain attribution regression checks.

The calibrated Premium engine, 30 factory presets, 32-sample latency, plugin and
parameter IDs, preset/state formats and GUI layout are retained. No DSP,
preset-value, FP-mode or numerical-threshold change is included.

## Known limitations

K1 remains unresolved: retained Triangle/Sine offline negatives are not explained
by later passing controls or the separate Chorus boundary fix. No shared cause,
audible defect or numerical fix is established by this identity correction.

Normal parameters and host bypass use the final value within each audio block;
short intermediate changes can be lost or applied early. Host MIDI-controller
messages retain individual sample offsets. Smaller host blocks reduce timing
granularity; smoothing does not restore discarded automation points.

Windows and macOS packages remain unsigned. Match the plugin architecture to the
DAW and close the DAW before updating. Legacy Classic state is readable but the
Premium filter can change its sound; preserve old plugins and project copies.
See [Installation](INSTALLATION.md) and [System requirements](SYSTEM_REQUIREMENTS.md).

## Validation scope

Prior reported 1.2.1 RC listening/loading/saving success is historical feedback,
not exact-1.2.2 host acceptance, native Windows ARM64 qualification or exhaustive
CPU/dropout coverage. Final CI and package checks are recorded in the
[1.2.2 release checklist](https://github.com/RobCZart82/SAWSTAR/blob/main/docs/RELEASE_1.2.2_CHECKLIST.md).
The archived 1.2.1 package bytes and tag are preserved.
