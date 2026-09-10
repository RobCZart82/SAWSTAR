# Preset library

The approved final concept is the primary reference; the user's earlier notes are
subordinate where they conflict. PRESETS uses four columns: Categories, Presets,
Preset Info / How It Works and Preset Actions. No additional concept image or
factory sounds are generated for this milestone.

## Browsing

Factory and user files share one list. All shows both; User shows saved/imported
files. Factory categories map to the existing sound metadata. Empty categories
remain empty rather than showing invented content. User sounds currently have the
User category; editable category/tag metadata is future work. The count is real.
Search matches name/category with case-insensitive ASCII matching; accented text
can be searched with its original spelling. Enter commits the search, empty text
clears it. Mouse wheel or the draggable scrollbar navigate longer lists.

Left-click a row to select and load its saved sound (Init asks for confirmation). The info panel shows the
saved waveform, mixer levels, filter, amplitude envelope and enabled/wet effects.
The diagrams are schematic, not measured responses or live scopes. Factory sounds
have authored descriptions; user sounds have a factual saved-settings summary.
Hearts toggle favorites, persisted in favorites.txt in the user preset folder.

The persistent top selector and arrows include both factory and managed user
files. Top selection loads immediately. The loaded user filename has an asterisk
when the current sound differs from its saved snapshot. Changing categories or search filters does not change the playing sound.

## Actions

- Save: overwrite the active User preset after a Cancel-default confirmation naming
  the preset. Enabled only when its sound has changed. Factory/Templates cannot
  be overwritten; use Save As. The target is the active sound, not a different
  preview selected by filtering. Successful Save refreshes the saved diagram.
- Save As: native save dialog for the current sound; existing filenames are not
  overwritten. Default destination is the managed user folder. When saving to an
  external folder, also import a library copy; a library name collision is reported
  and the external saved copy remains intact.
- Initialize: confirm before loading the embedded Init sound.
- Copy/Paste: copy the current sound's parameters inside this editor and apply
  them without creating a file. Save As persists the pasted sound. This is not a
  system clipboard preset format; closing the editor clears this temporary copy.
- Rename: enter a new name for the selected user file and press Enter. Factory
  files are read-only. Favorite identity follows a successful rename.
- Delete: confirm removal of the selected user file. A .deleted / .deleted-N
  backup is retained beside it. The playing sound continues. Restore by renaming
  the backup to an unused .sawstar filename.
- Import: select one or many .sawstar files in the native file dialog. Validate
  each file and copy it into the managed folder without overwriting existing
  names. Report imported/skipped/failed counts and diagnostic filenames; a bad file
  does not stop other imports. Cancel changes nothing. Imported sounds appear
  under User without being automatically played.

There is no bank format: one sound is one .sawstar file. Windows allows a combined
selection path buffer of 65,536 characters; exceeding the platform dialog limit
reports an error and requests a smaller selection.

## Storage and safety

- macOS: ~/Library/Application Support/SAWSTAR/Presets
- Windows: %APPDATA%/SAWSTAR/Presets

The existing versioned binary sound-state format is unchanged. User filenames
allow up to 80 UTF-8 bytes with path/control characters excluded; OS restrictions
also apply. File reads validate the state before applying any parameters. File I/O
is done on editor actions, not the audio thread. The native macOS importer uses
NSOpenPanel with multiple selection; Windows uses the multi-select common dialog.
No third-party dependencies were added.

DAW project state embeds all sound parameters. It does not depend on a preset file
and does not preserve the external filename association; reload from the library
to associate it. Favorites survive editor/app restart; clipboard and browser
filter/preview selection are temporary editor state.

## Validation

Code: 9285b24a12a213b453d92f38d074971f2a5f78e5.

- [macOS checks](https://github.com/RobCZart82/SAWSTAR/actions/runs/34359575985)
- [Windows checks](https://github.com/RobCZart82/SAWSTAR/actions/runs/34359576007)

preset_library_import tests 30-file import, content preservation, duplicate and
conflicting names, corrupt/missing files, case-insensitive search, categories,
favorites persistence across sequential editor instances and rename/archive behavior in a temporary directory.
Existing user file lifecycle, engine and state tests also run. Native multi-select
interaction and final visual layout still need manual REAPER host inspection.

Both Release VST3 jobs passed 28 foundation/engine/file tests and 47 VST3
validation checks. Both downloadable ZIP archives passed integrity checks.
A REAPER UI inspection was attempted, but the automation tool returned no contents
for the settings window, so native selection and layout are not claimed as tested.
All six final jobs succeeded: macOS and Windows Debug, Release and VST3 Release.

Initialize and Delete are now separated at the bottom of the actions panel,
using a shared Cancel-default confirmation. Factory Delete is disabled.
The quick-menu Init is isolated at the bottom; preset arrows skip initialization.
See [editor safety changes](EDITOR_CLOSE_FIX.md).

## Live explanations and identical imports

HOW IT WORKS displays only the selected preset's saved parameters. Save refreshes the active saved sound; Save As
selects and displays the new saved sound. No live/saved toggle remains. Diagrams
are generated from parameters, not stored images or audio measurements. The SUB
waveform name is included.

Import skips filename collisions without overwriting. Matching decoded parameter
snapshots under different user filenames are skipped and reported with the
existing name. A separate Cancel-default confirmation allows those copies.
Factory/User text and colored dots indicate library ownership: factory sounds
can still be edited and saved as user sounds. The shared header uses Orbitron.

## Confirmed Save — 2026-09-10

Save first writes and flushes a temporary file beside the target, retains the old
file at `.sawstar-backups/<unique-id>/<preset-name>.sawstar`, then replaces the
target. Recovery copies are not shown in the library. Copy a recovery file to an
unused `.sawstar` filename to restore it. Backup or replacement errors are shown
and do not mark the sound as saved. This is not a general disk-backup service.

Save operations use an interprocess lock and reject a target whose decoded
settings differ from the snapshot loaded by this instance. Reload or Save As
resolves this conflict. Arbitrary external file edits are not coordinated by the
lock. Save As remains an exclusive new-file operation. No state format, audio
processing or factory preset values change.

The confirmation saves the snapshot captured when Save was pressed. If host
automation changes the sound meanwhile, the changed indicator remains.

Verification: four targeted ASan/UBSan tests passed locally, including Unicode
paths, recovery copies, competing writers, stale saves, missing files and a
backup-creation failure. Full platform build results are recorded below.

Manual host acceptance remains: load a User preset, edit a control, cancel Save
and verify unchanged disk contents, then confirm Save and reopen the preset.
Check the changed marker and saved diagrams, disabled Save for unmodified User
and Factory sounds, and Save As for a new copy. Reopen two instances of one User
preset and verify that the second stale Save reports a conflict. Restart REAPER
to unload the previous binary before testing the installed build.

Build `0245866db93f67c863e8526ee3332d8b70038a43`: both Release VST3 jobs
passed 33 tests and 47 validator checks. Both artifact SHA-256 digests and inner
ZIP integrity checks passed. The macOS bundle is installed with its predecessor
preserved. Manual Save-dialog host acceptance is not claimed.

- [macOS CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34528078863)
- [Windows CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34528078935)

All six macOS/Windows Debug, Release and VST3 jobs completed successfully.
