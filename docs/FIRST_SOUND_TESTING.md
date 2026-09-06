# First Sound validation

Automated engine tests cover 44.1/48/96 kHz: initial silence, nonzero finite bounded
output, note-on velocity zero, sustain hold/release, completed release, 16-voice
limit, all sound off, channel isolation and reset. Both platform workflows run
these in Debug and Release, then build the plugin and run Steinberg's validator.

Manual acceptance: load in REAPER, click keyboard, play a MIDI clip/chord, release
all notes, check pedal and keyboard highlights, change ADSR/output, switch tabs,
Load Init, save/reopen project, and test independent plugin instances.

Earlier silent-shell REAPER test (965120b, macOS ARM64, REAPER 7.79) confirmed
loading, tabs, Load Init and Attack=23.24 ms across project save/reopen. That
result is not evidence of audio acceptance for this new engine build.

User reported successful Windows manual testing on 2026-09-06 (all working).
This is user-reported evidence; no independent Windows GUI session or host
version was captured in this task.

The current development milestone still needs
automation recording/playback, full preset library and final GUI work. Versioned state and legacy migration
are now implemented; see PARAMETERS.md.
