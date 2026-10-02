# SAWSTAR 1.0.4

Published: 2026-10-02. Source commit: `1a6a5f2fc13d9b85a4f0ad4afcb191e4f84ddf38`.
The packaged UI/build metadata date is 2026-10-01; it identifies the prepared build, not the publication day.

This maintenance release collects reliability fixes made after v1.0.3.

## MIDI and host lifecycle

- Editor overflow recovery accounts for its own notes and absorbs late releases
  without releasing a same-pitch note owned only by the host.
- Reset discards editor messages pending before its boundary. Duplicate or
  unmatched editor releases are ignored.
- Closing the editor releases its mouse-held keyboard note and finishes an
  active PITCH-wheel gesture. The MOD wheel remains latching.
- Final pitch/mod controller targets survive queue overflow; GUI wheel state
  uses a bounded latest-value mailbox. Host sample-zero MIDI wins a tie.
- Zero-frame processing retains a pending editor-overflow notification until
  accepted MIDI has reached the tracker in a real audio block.
- Bypass continues MIDI and engine processing while muting the host output,
  avoiding stale queued events being replayed after unbypass.

- ARP transport stop also clears editor note ownership, preventing a late GUI
  release from stopping a fresh host note of the same pitch.
- Every VST3 MIDI-controller parameter point keeps its original sample offset,
  including sustain transitions, pitch bend and channel aftertouch. Ordinary
  parameter automation retains its existing last-value policy.
- Accepted editor MIDI is processed at sample zero, including overflow recovery.

## Envelopes, tail and presets

- Long ADSR segments reach their stage boundary even when float recurrence
  stops changing at high sample rates.
- The VST3 reports a conservative finite tail of 380 seconds, covering serial
  envelope/chorus/delay/reverb history. This is a host rendering bound, not a
  forced timeout on held notes. Some hosts may append long silence when using
  the bound; their render-tail/trimming settings still matter.
- Enum/integer preset values use the existing host rounding rule, preventing
  untouched external presets from appearing modified on load.
- Standalone `.sawstar` files require a non-empty versioned payload containing
  a known parameter and exactly the declared length. Arbitrary headerless data,
  empty/unknown-only payloads and host trailers are rejected as files. Old
  versioned partial presets remain supported; legacy DAW project state and
  its optional VST3 bypass trailer retain their existing compatibility.

- Batch import indexes existing names and exact settings once under the shared
  mutation lock, instead of repeatedly scanning and decoding the library.
  Failed saves do not poison the index; existing names remain protected.
  Import remains synchronous, with no persistent cache or background worker.

## Build verification

The Windows, macOS and Quality workflows succeeded for the exact package source
commit. All three platform ZIPs passed manifest, file-list, manual and license
validation; all seven release payloads match SHA256SUMS.txt.

## Compatibility and packages

Parameter IDs, plugin identity and state wire version are unchanged. The GUI
layout and synthesis character are preserved. This update does not introduce
experimental Mono click/pop treatments, a new filter, 32 voices, Linux VST3,
sample-accurate parameter automation or atomic multi-parameter preset loading.

Packages are Windows x64/ARM64 installers and manual VST3 ZIPs,
and macOS Universal DMG/PKG and manual VST3 ZIP, with English/Hungarian manuals.

Close the DAW before updating, back up projects/user presets, and check the
actual About version and commit after reopening. Packages remain unsigned and
macOS is not notarized. See [Installation](https://github.com/RobCZart82/SAWSTAR/blob/v1.0.4/docs/INSTALLATION.md) and
[System requirements](https://github.com/RobCZart82/SAWSTAR/blob/v1.0.4/docs/SYSTEM_REQUIREMENTS.md). Previous public releases remain
available; none are overwritten by this release.

