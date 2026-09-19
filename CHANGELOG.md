# Changelog

## Unreleased — 1.0.3-rc2

Includes the unpublished 1.0.2 development work.

- Candidate release preparation skips cleanly without creating a release.
- Fix combined Volume/Boost smoothing, detune continuity and initial Delay time.
- Share and verify production MIDI scheduling and parameter mapping.
- Reject non-finite benchmark data and package version-specific release notes.

- Resume from released 1.0.1; exclude experimental Mono crossfades and filter-start preparation.
- Fix unrendered same-sample chord pitches becoming glide origins.
- Reset the amplitude envelope only when allocating a genuinely idle voice.
- Resolve same-sample Mono releases once before audio processing.
- Retain Windows test-stack fixes. See docs/CHARACTER_PRESERVING_BASELINE.md.

## 1.0.1 - 2026-09-12

- Refine meter ballistics, CLIP display, keyboard glow, scales and typography.
- Initialize idle voice targets and Reset gain correctly; retain active-voice smoothing.
- Cache unchanged reverb settings; reduce duplicate wrapper work.
- Reject empty state; group host parameters without changing IDs.
- Protect Unicode preset-name collisions; improve confirmation text wrapping.
- Add deterministic stress tests and refresh bilingual manuals/screenshots.

See [1.0.1 release notes](docs/RELEASE_NOTES_1.0.1.md).

## 1.0.0 - 2026-09-11

First public release: VST3 for Windows x64/ARM64 and macOS Universal, with
24 embedded presets, English/Hungarian PDF manuals, installers and manual ZIPs.
