# Noise sources and color

MAIN → MIXER contains White Noise, Dark Noise and Pink Noise in the source menu.
NOISE controls level; NOISE COLOR is a separate horizontal bipolar control:

- 0%: unchanged source (default).
- Negative: progressively softer, low-pass character.
- Positive: progressively removes low frequencies, emphasizing the upper range.

Color does not normalize loudness: removing parts of a spectrum can reduce its
level. Adjust NOISE to balance it with the oscillators. The existing voice filter,
Amp ADSR, modulation and effects remain downstream of the noise source.
Dark is the original filtered white noise; Pink is an independent approximate
1/f source. They are intentionally distinct timbres. The sources are per voice.

## Compatibility

Existing White/Dark states and their normalized host automation retain ID 26's
original two-value range. New source selection uses ID 62; the default Follow
legacy value honors ID 26. The UI displays the effective White/Dark/Pink choice.
Once an explicit source is selected, use Noise Source for new automation.
Old presets load neutral color; all new values are included in saved state.
No new third-party dependency was added.

## Manual REAPER check

1. Load an older Soft Pad One or user preset and confirm its old noise character.
2. Mute OSC1, OSC2 and SUB, raise NOISE, and hold a note.
3. Compare White / Dark / Pink and sweep NOISE COLOR through -100 / 0 / +100.
4. Check that COLOR has no audible effect with NOISE muted.
5. Save a named Pink preset with nonzero color, restart the host, and reload it.
6. Check a chord and release, then automate color during held notes.

This manual host check is pending for this build.

## Automated validation — 2026-09-07

Tested source commit: `24cf919d90d2deff3b026ae40419c116b0e0cd2e`.
Both macOS and Windows passed all 18 tests in Debug and Release, plus
18 foundation/DSP tests in the VST3 Release jobs. The VST3 validator passed
47 tests with zero failures on each platform.
Noise checks cover approximate pink spectral slope, color direction at
44.1/48/96 kHz, finite bounded output, release, and oscillator isolation.
State tests cover the new settings and legacy 62-parameter state migration.

- [macOS run](https://github.com/RobCZart82/SAWSTAR/actions/runs/34144516746)
- [Windows run](https://github.com/RobCZart82/SAWSTAR/actions/runs/34144516792)

Development archives include the applicable license notices. Manual REAPER
listening, GUI interaction and named-preset reload remain to be checked.
