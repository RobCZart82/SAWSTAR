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
