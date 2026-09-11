# Source audit fixes — 2026-09-11

Tested implementation: `6f0985e9ec49d8b9ee1c2dd54393cbb8f28689e8`.

## Confirmed defects and changes

- Concurrent Save and Rename could both succeed, restoring the old filename
  while the renamed file retained outdated settings. Save, Save As, Rename and
  Delete now share the same process mutex and per-directory file lock.
  Overwrite retains its stale-snapshot check, backup and atomic replacement.
  If Rename/Delete wins first, Save reports the missing original instead of
  recreating it. This coordinates SAWSTAR instances; unrelated external editors
  do not participate in the lock protocol.
- All-channel MIDI CC120 stopped voices but left audible chorus/delay/reverb
  history (reproduced peak approximately 0.0471). Once all 16 channels receive
  CC120, the engine clears shared effect history without allocation or parameter
  changes. A new Note On resets the channel mask. Individual-channel CC120
  preserves other channels and shared tails; Note Off and CC123 still permit
  normal release. The plugin's existing all-channel panic sequence benefits
  from this behavior, including its MIDI queue-overflow recovery.
- The scope advertised 32 ms even when its bounded window held only 8192 samples
  at 384 kHz (21.333 ms). Drawing and labeling now use the same window-length
  calculation. Typical rates retain the 32 ms label. Capture size, audio signal
  and preset format are unchanged.

## Verification

All 34 local tests passed under AddressSanitizer and UndefinedBehaviorSanitizer.
After the final small edits, the three affected test executables passed again.
The user-preset lifecycle test also passed under ThreadSanitizer with no report.
The preset test races overwrite against Rename and Delete for 32 rounds each,
accepting either valid operation order while rejecting resurrected filenames or
incorrect destination content. The engine test checks unrelated-channel audio,
exact silence after global panic, and successful new notes afterward. The scope
check covers the capacity-limited 384 kHz timebase.

Both macOS and Windows VST3 Release builds passed all 34 foundation tests and
47 VST3 validator checks. Both downloaded artifact SHA-256 digests and inner ZIP
integrity checks passed. The macOS bundle was installed after preserving its
previous version. No fresh REAPER listening/GUI test or Windows manual GUI test
is claimed for this patch.

- [macOS CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34559100791)
- [Windows CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34559100792)

Automated checks do not replace a listening test, host-specific GUI acceptance
or external-file-editor concurrency testing. No WAV export or public release
is part of this fix.
