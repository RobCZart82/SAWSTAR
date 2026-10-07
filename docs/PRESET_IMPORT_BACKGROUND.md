# Background preset import

The Import action now queues copied file paths and a destination folder on a
plugin-owned `PresetImportJob`. The worker performs the existing import and
prepares the library snapshot (including directory/favorites reads). It carries
no GUI, plugin, parameter, or editor pointers. The audio callback never accesses
the job. The native file picker still runs on the GUI thread.

`OnIdle` polls the result through the live preset browser. The GUI moves the
prepared library into its view, filters it and reads the selected preview, then
shows the existing report/duplicate confirmation. Confirmed identical copies
are queued as a second background batch. These GUI steps do not wait for the
batch worker; selected-preview reads, dialogs and drawing remain GUI operations.

## Lifetime and busy behavior

- One request/result is retained per plugin instance. A second request is refused
  until the previous result is consumed, preventing an unread report from being
  overwritten.
- Closing the editor leaves the worker and completion alive. Reopening creates
  the browser without an initial filesystem refresh while the job is busy; the
  next idle poll retrieves the result. The imported files remain on disk.
- This instance's preset actions and quick selector are disabled while import is
  pending. Normal synth controls and MIDI processing are independent of the job.
  Save/Save As/Delete confirmation callbacks also check busy state.
- Plugin destruction signals stop and joins the worker before the plugin code
  can be unloaded. A queued, not-yet-started batch may be cancelled; an active
  batch finishes before unload. Slow filesystem I/O may therefore delay plugin
  destruction, but closing only the editor does not join the worker.
- Existing import duplicate/collision checks, exclusive writes and mutation
  locks are unchanged. Other plugin instances still share those locks; their
  synchronous Save/Rename/Delete operations can wait for an active import.
- Completion applies a point-in-time library snapshot. Subsequent external edits
  use the existing library-refresh behavior.

## Automated evidence

`preset_import_job` uses the real import function with a deterministic blocked
worker to verify that request/poll operations remain available, work runs off the
owner thread, a second request is rejected, and results are delivered once.
It checks actual imports and prepared library data, mixed collision/invalid/
missing-file reporting, confirmed duplicate retry, retained unread completion,
error recovery and joined shutdown. Deferred library construction is checked
without filesystem enumeration. The test is included in the Linux TSan subset
as well as the normal Windows/macOS and ASan/UBSan suites.

```sh
ctest --test-dir build -C Release -R '^(preset_import_job|preset_reliability|preset_library_import|user_preset_files)$' --output-on-failure
```

The [earlier 54-case profile](PRESET_IMPORT_PROFILE.md) measures the synchronous
work now dispatched to the worker. It does not measure the new GUI behavior.
No native responsiveness or audio-dropout claim follows from that profile.

## Native REAPER acceptance still required

1. Import 100 new files into a 1000-preset library while playing MIDI. Confirm
   normal knobs/keyboard remain usable and preset actions show import busy.
2. Close the editor during import, reopen both before and after completion, and
   confirm exactly one report, the resulting files, and re-enabled actions.
3. Repeat with duplicate, colliding, invalid and missing files; confirm retrying
   identical copies remains asynchronous and sound settings remain unchanged.
4. Restore host state during import; confirm imported files and the active sound
   remain independent.
5. Remove the instance or close the project during a large import. Verify safe
   worker completion and record any unload wait, including on a slow volume.
6. Run imports in two instances and attempt Save/Rename/Delete in another; record
   lock waiting and verify no overwritten/lost presets.

Run on Windows x64/ARM64 and macOS Apple Silicon/Intel as available, recording
the exact plugin build. These native checks are not replaced by the job model
tests or VST3 compiler/validator checks.
