# September 10, 2026 audit fixes

The source audit identified five edge cases. This change addresses them without
changing parameter IDs, preset format or plugin identity.

| Issue | Change | Regression coverage |
| --- | --- | --- |
| Concurrent Save As can overwrite an existing writer | Exclusive OS file creation; losing writers fail without deleting the winner's file | Eight concurrent writers, 32 rounds; exactly one succeeds and its complete snapshot survives |
| SUB switches waveform instantly | Phase-continuous 10 ms crossfade, retargetable from the current mixture | All shape pairs, four sample rates, repeated parameter updates and rapid retargets |
| Untouched user preset shows an edit marker | Tolerant finite snapshot comparison, with header refresh | Normalization round trip, one-ULP noise, real small edits and NaN rejection |
| Reopened keyboard misses held notes | Reset displayed-note cache on editor creation; do not advance it while closed | Code-path review; manual host lifecycle acceptance remains pending |
| Host state restore keeps stale user-preset identity | Atomic notification consumed on GUI thread, then clear/reselect saved preview | Code-path review; manual host state/GUI acceptance remains pending |

The local macOS run passed all **29 C++ tests** under AddressSanitizer and
UndefinedBehaviorSanitizer. These checks cover the engine/foundation, not the
complete host editor. Platform builds and VST3 validation run in GitHub Actions.
No WAV export is needed for these checks.

GUI changes in this set: centered quick-preset anchor, OSC knob-row separators,
FILTER MIX / KEY TRACK separator, and a subtle 270-degree dotted knob scale.

## Focused host acceptance

1. Save a user sound, change it and reload it. The name should have no edit marker
   after loading. Move one control, then restore its value; the marker should
   appear and disappear accordingly.
2. With an audible SUB and a held note, switch Sine/Triangle/Square repeatedly.
   Check for a smooth change; the other two oscillators need not be enabled.
3. Hold an external MIDI note while closing/reopening the editor. Its key should
   still light; release should clear it. Also start a note while the editor is closed.
4. Load a user preset, then restore a different DAW-saved state in the same plugin
   instance. The old user filename and unrelated preview must not persist.
5. Check the quick preset menu, both OSC grids, the FILTER separator and knob dots
   on MAIN and ADVANCED at the display scales used for testing.

These steps describe outstanding manual checks; they are not claims of a
completed REAPER test.
