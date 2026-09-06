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

## State/automation follow-up (2026-09-06)

Code build b8acc804: macOS CI run 34036993019, Windows 34036992996.
Both ran four test executables successfully; both VST3 validators report
47 tests passed, 0 failed. Debug and Release foundation/engine tests also passed.

Manual macOS REAPER 7.79: migrated an actual legacy project (Attack 168.58 ms,
Decay 100 ms, Sustain 0.200, Release 491.80 ms). Saved a project containing
SAWSTAR v1 magic and ID/value records; reopened the saved state in a copied
project with an Output automation lane. The four envelope parameters remained
unchanged and Output followed the lane's -48 dB starting point.

Output lane was created in REAPER, then populated in a disposable RPP copy with
normalized 0.2 -> 0.8 -> 0.2 points. Offline rendering produced finite audio,
peak -29.1 dBFS and no clipping. Measured early/late note RMS differed by 41.93 dB
(note/envelope differences mean this is not a calibrated gain-linearity test).
The final 100 ms peak was 5.13e-6: quiet release tail, not exact zero. This checks
automation playback and save/recall, not real-time touch/latch recording of all
five controls. Those broader host checks remain pending.
