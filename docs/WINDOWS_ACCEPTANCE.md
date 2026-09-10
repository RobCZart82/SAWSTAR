# Windows VST3 acceptance checklist

Record Windows version, CPU, REAPER version, audio device, sample rate/block
size, SAWSTAR version/date and About commit ID. Test the matching final build,
not an older plugin cached by the DAW. Keep real projects/presets backed up and
use a new test project. These items are pending until actually observed.

- [ ] Close the DAW, install the x64 VST3 bundle, restart/rescan and confirm About ID.
- [ ] MAIN, ADVANCED and PRESETS at 75/100/125%: no clipping, overlapping labels,
      missing hearts, unreadable text or misplaced popup menus.
- [ ] Open/close the editor repeatedly; chosen scale persists during the instance.
- [ ] Main Return and numeric Enter commit search, rename and knob value entry;
      Escape cancels. Test accented preset names and a Windows profile path
      containing non-ASCII characters where available.
- [ ] Test dropdowns near every edge, LFO1/2, effect tabs, on/off and knob/fader
      editing; inactive controls remain identifiable.
- [ ] Save As opens a named save dialog; cancel does nothing; save/load roundtrip
      succeeds. Existing files are not silently overwritten.
- [ ] Import 30 distinct temporary `.sawstar` files together. Scroll/drag to the
      last row; filter to one and zero results; reset search without stale rows.
- [ ] Favorites survive reopening; two instances update without losing entries.
- [ ] Rename a user sound, including case-only rename; factory modification
      restrictions and Initialize/Delete confirmations work as intended.
- [ ] HOW IT WORKS shows saved settings; editing the current sound does not
      silently rewrite its saved diagram. Save As creates an updated saved view.
- [ ] Notes, repeated/very short notes, sustain, pitch bend, modulation and ARP
      work. Stop/restart does not leave stuck notes.
- [ ] Automate gain, filter, waveform, effect enable/mix and modulation; save,
      close and reopen a two-instance project. Settings and envelopes persist.
- [ ] Check 44.1/48/96 kHz and practical block sizes (64/512), live playback and
      offline render; no invalid output, runaway CPU or unexpected clipping.

Report each failure with the exact action, expected/actual result, build ID and
a screenshot if visual. A successful build or validator is not this manual test.
