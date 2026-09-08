# User preset files

PRESETS keeps the existing Factory list read-only and adds a User Presets panel.
Click the name field to edit it, then use:

- **Save As**: save the current sound under a new name. Existing names are rejected,
  never silently replaced. To update a sound, save a new copy first.
- **Load**: open a `.sawstar` file using the native file picker.
- **User Library**: scan and select a preset from the user folder, including after
  an application restart. External loaded files enter this folder only via Save As.
- **Rename**: rename the selected user file to the text in the name field.
- **Delete**: confirm removal from the library. The file is renamed with a
  `.deleted` (or numbered `.deleted-N`) suffix in the same folder. Restore it by
  renaming it back to an unused `.sawstar` filename. The current sound keeps playing.
- **Init**: load the existing Init factory sound.

Factory presets cannot be renamed or deleted. The name at the top shows the loaded
user filename and `*` when the sound differs from its saved snapshot. The filename
selection is editor metadata; DAW project state still embeds all sound parameters,
so playback does not depend on an external preset file. Reopening a project does
not restore the external filename association; load from User Library to associate
it again. The top arrows/dropdown continue browsing Factory presets.

## Storage

- macOS: `~/Library/Application Support/SAWSTAR/Presets`
- Windows: `%APPDATA%/SAWSTAR/Presets`

Files use the existing versioned binary SAWSTAR state codec, including master WIDE.
I/O happens only in editor actions, not in the audio processing callback. Invalid
files are rejected before any parameter change. Names allow up to 80 UTF-8 bytes
and exclude control/path characters and trailing dots/spaces. Operating-system
filename restrictions also apply; errors are shown in the panel.

The `user_preset_files` test covers save/read equality, name validation, refusal to
overwrite, rename, archived deletion and corrupt-file rejection in a temporary
folder. State tests cover parameter round-trips. Native dialogs and visual polish
still require host interaction testing; no new factory sounds were added.

## Remaining visual work

The controls are functional; final dropdown/button differentiation, typography,
spacing and the green/orange/red (70/20/10 display-height) OUTPUT meter palette
belong to the agreed final GUI polish milestone. Pre-release compatibility may be
broken when justified by a concrete design improvement; no codec change is needed
for this feature.

## Build checks

Code: `20da6bf362ad97bd84377ea7ce97a912f5b69760`.

- [macOS checks](https://github.com/RobCZart82/SAWSTAR/actions/runs/34260671316)
- [Windows checks](https://github.com/RobCZart82/SAWSTAR/actions/runs/34260671314)

The system-installed plugin is not replaced by this source change.
