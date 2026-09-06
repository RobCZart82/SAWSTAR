# Factory library and shared preset selector

Six original parameter-only sounds ship with SAWSTAR: Init, Wide Saw Lead,
Soft Pad, Saw Pluck, Deep Bass and Bright Keys. No samples or new dependencies.
Each sound includes a category and one practical sound-design explanation in
PRESETS. Click a row to load it; changes replace current parameter values.

The fixed header contains SAWSTAR with the two-line Simple Synth / Big Sound
slogan, then MAIN / ADVANCED / PRESETS, followed by the preset selector.
Its arrows wrap through the library; clicking the name opens the factory list.
The bottom keyboard, wheels and status row remain shared across all views.

The selected name is derived from all nineteen parameter values, including after
DAW project recall. An edited/unrecognized sound displays Custom. From Custom,
next selects Init and previous selects Bright Keys. Returning all values to a
factory sound restores its name. Transient pitch/mod MIDI values do not affect
this comparison. Factory identity is not a separate persisted parameter.

Loading uses host parameter gestures and the existing state codec; no IDs or
state schema change. Existing custom projects remain loadable. Live notes and
MIDI controller positions are retained while the sound parameters change.
Switch presets in a DAW automation read mode with care: host automation can
subsequently override any loaded parameter.

This milestone is an internal factory library. User preset files, save/rename,
search/favorites and VST3 program-list exposure are deferred. Save custom sounds
in DAW projects/host preset facilities for now. Reloading a factory sound replaces
unsaved edits, as with the existing Load Init action.

Tests cover unique keys/names, bounds, Init defaults, recognition/Custom behavior,
arrow wrapping, codec roundtrips, audible bounded output and release completion
for every preset at 44.1/48/96 kHz.

## Validation — 2026-09-06

Code commit: `36d3bb57deff413cae704f21a96ffb8ca9b1cb52`.

- [macOS CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34047265276)
  and [Windows CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34047265261)
  passed all nine test executables in Debug and Release. VST3 validation passed
  47 checks on each platform.
- REAPER 7.79 on macOS loaded the artifact and an existing custom project.
  Header arrows (including wrap), the dropdown, library rows and Custom after
  editing were checked. Saving, closing and reopening the project restored
  Saw Pluck parameters and the derived preset name.
- A stereo Saw Pluck MIDI render at 44.1 kHz / 24-bit PCM peaked at -36.62 dBFS
  with zero clipped samples. A separate listening preview was normalized to
  -6 dBFS; this does not change factory output levels.
- Manual Windows host testing of this preset-library build remains pending.
