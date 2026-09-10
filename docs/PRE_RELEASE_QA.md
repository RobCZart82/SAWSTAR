# Pre-release engine and GUI work — 2026-09-10

This package advances phases 1–4 of FIRST_RELEASE_PLAN.md. It is not a public
release or a claim that every release-acceptance item has been completed.

## Changes

- Reset startup no longer depends on previous mixer/boost smoothing history.
  The first processed sample uses restored mixer/boost targets; subsequent live
  edits remain smoothed and master gain retains its deliberate startup fade.
- A shared test rig now includes SUB waveform and WIDE in the combined engine
  audit. A new regression compares fresh and reused/reset instances at
  44.1/48/96 kHz with identical restored settings and MIDI.
- Compiler warnings cover the engine and plugin adapter.
- The shared header embeds a build date during configuration. A future public
  build can specify SAWSTAR_RELEASE_DATE=YYYY.MM.DD. to label it Release.
  The date never follows the machine's current date at plugin launch.
- Dropdown, knob and tab/toggle hover cues remain restrained and consistent.
- Favorite hearts are vector paths, avoiding missing glyph boxes in platform
  fonts. Existing MAIN color coding, blue controls, narrow scaled faders,
  separators and 70/20/10 green/yellow/red meter zones remain.

## Verification and limits

Code: `719886e4c1d6cc5a0194b771cb4954afdf14c9c9`. The final macOS build was installed with a backup; About confirmed its ID. The vector hearts were checked in both states and the previous favorites selection restored. All 30 temporary preset files were moved out of the user library after testing.

- All six final macOS/Windows Debug, Release and VST3 CI jobs succeeded.
- Both final VST3 packages passed 33 tests and 47 validator checks per platform; archive digests and ZIP integrity were verified.
- All 33 local C++ tests passed with AddressSanitizer/UndefinedBehaviorSanitizer.
  The reset regression failed before the startup fix and passed afterward.
- Optimized local CPU results are in pre-release-cpu-macos.csv. At 48 kHz,
  16 voices used about 11% (dry) / 14% (effects) of real-time processing duration
  in this one-second standalone benchmark. This is not REAPER CPU usage or a
  guarantee on other machines. Short runs have scheduling variance; lower
  one-voice effects timing does not establish that effects improve performance.
- The measured scenes remained finite and below full scale. Existing combined
  audit, source quality, transitions, MIDI, ARP, effect and persistence tests
  remain the automated sound-path coverage. No separate WAV was generated.
- macOS REAPER: three pages inspected at 75/100/125%; header date visible,
  no observed clipped controls. LFO1/2 and effect detail switching worked.
- A temporary 30-preset fixture produced a 41-item browser. Dragging the scrollbar
  reached the last entries; short and empty searches disabled the scrollbar.
  Searching Pad with keypad Enter returned two entries. Saved preset diagrams
  appeared when selecting Soft Pad One. Temporary files are not Factory content.
- MIDI playback produced meter/key feedback; stopping returned to zero voices.

Still required before claiming phases 1–4 fully accepted: Windows manual GUI
and keyboard testing, comprehensive final-host automation/project-recall matrix,
and musical listening acceptance of the finished sound/visual balance. Automated
tests and screenshots do not establish those results. Templates/Factory content
has not been changed.

The expanded warning flags also expose existing unused callback arguments, constructor member-order and numeric-conversion/deprecation warnings. These are not reported as a warning-free build; follow-up cleanup remains.
