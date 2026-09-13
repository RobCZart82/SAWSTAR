# Changelog

## Unreleased — 1.0.2 test branch

- Reset a truly idle voice's amplitude envelope before reuse. Retire voices over 6 ms when changing Poly/Mono/Legato mode, preserving outgoing sources when the next Mono note starts. Add multi-rate lifecycle and continuity regressions; see [mode transition fixes](docs/MODE_TRANSITIONS.md).

- Retain overlapping Mono/Legato transition branches with fixed storage and finite fade deadlines. Preserve natural waveform edges instead of adding a spurious correction offset; cover dense retargets, channel-local panic and 576 waveform/phase transition cases.

- Do not glide from an intermediate first-chord note that has never rendered audio. Preserve Always Glide between previously rendered, separated notes. See [mono transition research](docs/MONO_TRANSITION_RESEARCH.md) for the separate, unresolved low-note fallback investigation.
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
