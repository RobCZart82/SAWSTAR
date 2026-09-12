# SAWSTAR 1.0.1

Simple Synth - Big Sound. The Sounds of Trance.

## Changes since 1.0.0

- Refined stereo sample-peak meter: green/yellow/red LED cells, peak hold and subtle afterglow. Independent CLIP indicators clear on click or five seconds after the last clipped sample. This is not RMS/true-peak metering.
- Brighter keyboard highlights, a red felt line, clearer control scales, matched About separators and refined header/footer typography. Control layout and dimensions are preserved.
- Idle voices initialize oscillator/filter targets before their first sample; active retriggers, steals and legato retain smoothing. Reset starts at the intended master gain.
- Cache unchanged reverb settings and avoid redundant wrapper work.
- Reject empty host state safely; organize host parameters into groups without changing IDs.
- Improve Unicode preset-name collision protection on macOS and Windows.
- Confirmation text uses measured width rather than fixed character counts.
- Add a bounded, reproducible engine/ARP stress test and update English/Hungarian manuals.

## Downloads

Windows x64 and ARM64 Setup installers; macOS Universal DMG/PKG (Intel and Apple Silicon). Separate Manual ZIPs contain the VST3 bundle. Packages include both PDF manuals and license notices. SHA256SUMS.txt verifies the downloaded files.

Installers are unsigned and macOS builds are not Apple-notarized. Follow docs/INSTALLATION.md for normal installation and narrowly scoped troubleshooting. Do not install two copies of the same plugin into a host's search paths.

Existing parameter IDs and preset state format are retained. Back up projects and user presets before updating. The 1.0.0 release remains available.

## Validation

Preview b53772f passed macOS/Windows Debug and Release CI, including Universal/x64/ARM64 plugin builds. The author reported successful Windows/macOS installer testing and a four-instance demo render. Final package checks are recorded in docs/RELEASE_AUDIT_1.0.1.md; these do not constitute testing on every supported OS/hardware combination.
