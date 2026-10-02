# Changelog

## Unreleased

No further changes recorded.

## 1.0.4 - 2026-10-02

- Refresh an existing unpublished release draft from verified exact-commit packages; protect public releases and existing tags and mark interrupted refreshes incomplete.
- Link the README to the actual public Latest download.
- Index batch preset imports once under the library mutation lock, reducing repeated directory scans and preset reads while preserving filename collisions, exact duplicate checks and failed-save recovery.
- Synchronize editor note ownership when ARP transport stop clears its roots, so a later GUI release cannot stop a fresh host note of the same pitch.
- Deliver every VST3 MIDI-controller parameter point with its original sample offset, including sustain edges, pitch bend and channel aftertouch.
- Process accepted editor MIDI at sample zero so upstream overflow recovery cannot miss an editor onset deferred into a future block.

- Fix editor/host note ownership during overflow recovery, reset and editor close.
- Preserve final pitch/mod controller targets when MIDI queues overflow.
- Keep the synth MIDI/DSP timeline running during bypass while the host output is muted; retain overflow recovery across zero-frame calls.
- Complete long ADSR segments when float precision would otherwise stall.
- Report a conservative finite VST3 tail for the serial envelope/effect chain.
- Canonicalize enum/integer preset values to avoid false modified markers.
- Reject malformed standalone preset files without narrowing legacy DAW state compatibility.
- Identify development and RC builds clearly and refresh both user manuals.

No experimental click/pop treatment, filter-character change, additional voices or Linux plugin is included.

## 1.0.3 - 2026-09-19

Includes the unpublished 1.0.2 development work.

- Require successful matching platform and Quality checks, verified package contents and both manuals before preparing a release.
- Correct Voice Mode/Glide host grouping and refresh technical documentation.
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
