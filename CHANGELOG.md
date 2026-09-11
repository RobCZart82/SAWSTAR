# Changelog

## 1.0.0 - 2026-09-11

### RC2 preparation

- Move header motto 5 px closer to the SAWSTAR wordmark.
- Center Developed by / Gyuricza Róbert in About, retaining the separate GYR mark.
- Label render-time usage DSP CPU; update its displayed value every 250 ms.
- Hold scope frames for about 90 ms and smooth display scaling, without changing
  capture timing or audio. Continue consuming frames when another tab is shown.
- Add explicit Windows x64/ARM64 and macOS Universal build targets.
- Add Windows Setup and macOS PKG/DMG candidate packaging; signing remains separate.
- Separate user requirements from development prerequisites; replace stale README.

### RC1

Aligned plugin version metadata at 1.0.0 Pre Release. Packaged bilingual manuals,
notices and checksums. Six original CI jobs passed; user reported Windows manual
acceptance. See docs/RELEASE_CANDIDATE.md for exact binaries and evidence.

No public v1.0.0 release has been published by these changes.
