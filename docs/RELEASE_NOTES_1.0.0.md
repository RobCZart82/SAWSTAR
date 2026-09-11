# SAWSTAR 1.0.0 - release candidate notes

**Candidate, not a published release. No public release date assigned.**

Simple Synth - Big Sound. The Sounds of Trance.

## Included

- Two waveform/unison oscillators, SUB with selectable waveform, White/Pink/Dark
  noise and Noise Color, four-source mixer.
- Filter and amp envelopes, filter character controls, poly/mono/legato,
  pitch bend, sustain, glide, two LFOs and four modulation routes.
- Tempo-following arpeggiator, chorus, delay, reverb and master stereo width.
- MAIN / ADVANCED / PRESETS interface, pre-effects scope and output meter.
- 24 embedded sounds: seven Templates and 17 finished presets.
- Individual .sawstar files, multi-file import, favorites, Save/Save As,
  recoverable deletion and saved signal-flow explanations.
- Separate English and Hungarian PDF guides (development edition).

## Packages and scope

RC2 provides macOS Universal VST3 (Intel and Apple Silicon), Windows x64 and
Windows ARM64 VST3 candidates. Windows Setup EXEs and a macOS PKG inside a DMG
are available alongside build ZIPs. AU, CLAP and standalone formats are not
included. See INSTALLATION.md and SYSTEM_REQUIREMENTS.md. Final minimum OS
support remains to be established; CI OS versions alone are not a support guarantee.
See RELEASE_CANDIDATE_2.md for exact validated files and limitations.

These candidate packages are not represented as Developer ID signed/notarized
or Windows Authenticode signed. macOS may contain an ad-hoc build signature;
that is not Developer ID signing or notarization. A signing/distribution decision
and clean-machine acceptance remain publication gates.

Keep user-library and project backups when moving from development builds.
Factory data is embedded; an existing DAW project recalls its saved parameter
state, not automatically the latest factory preset settings.

## Before publishing

Verify the final binaries, finish listening/clean-install acceptance, finalize
manual edition metadata and screenshots, record supported OS versions and the
signing policy, then configure the actual SAWSTAR_RELEASE_DATE and rebuild.
Publish only matching final archives and their SHA256SUMS under v1.0.0.
