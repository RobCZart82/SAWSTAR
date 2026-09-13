# Changelog

## Unreleased — 1.0.2 test branch

- Prepare eligible idle low-pass filter states from a bounded tonal preview, preserving oscillator phase and envelope timing. Keep the previous start path for noise, nonstationary triangle sources, Nyquist clamps and other filter regions.
- Add independent filter-state and preview-continuity regression checks. See [test findings and limits](docs/PREPARED_FILTER_START.md).

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
