# Editor-close crash and preset action safety

## Evidence and cause

Two user-provided macOS REAPER crash reports from 2026-09-09 show EXC_BAD_ACCESS
in StaticStorage<APIBitmap> cleanup inside IGraphicsNanoVG::OnViewDestroyed,
called by CloseWindow when removing the plugin editor.

The pinned iPlug2 NanoVG memory bitmap loader already inserts its APIBitmap into
its per-context cache. The base IGraphics memory LoadBitmap wrapper temporarily
owns the same pointer with unique_ptr. On a Retina screen (target scale > 1), its
ScaleBitmap return path destroys that pointer while the backend cache still holds
it. The later cache cleanup then touches the freed bitmap. The About creator PNG
was the caller of that path even if About was never opened, because OnInit loaded
it with the editor. The source image itself is not corrupt.

At the user's request, About now uses a real text wordmark with the unmodified
Orbitron Bold font instead. The PNG loading path and generated CreatorLogo data
have been removed from the runtime build entirely. The original GYR asset remains
archived in the repository; it is not embedded or loaded. Font loading uses the
framework's font-resource API, not the problematic memory-bitmap scaling wrapper.
This removes the identified crash path rather than suppressing cache cleanup.
No edits were made to the locally modified iPlug2 submodule.

## Safety controls

Preset Actions order: Load, Import, Save As, Copy, Paste, Rename; then a separated
bottom group with Initialize and Delete. Factory Delete is disabled. Both actions
share a full-editor confirmation overlay with matching layout, action labels,
and a default Cancel selection. Escape cancels. Enter initially cancels; Tab can
explicitly move focus to the action button. Cancelling does not invoke the pending
operation. Closing the editor discards the callback.

Initialize from the list Load action or quick menu is guarded as well. The quick
menu places INIT PRESET at the bottom after a separator, and arrow navigation
skips Init. Delete keeps the existing recoverable .deleted backup behavior.

## Scope and validation

Code commit: 3762d9fa0dc82def06def1d8371c44eec82b11bd.

- macOS: https://github.com/RobCZart82/SAWSTAR/actions/runs/34389205900
- Windows: https://github.com/RobCZart82/SAWSTAR/actions/runs/34389205905

Both release plugin jobs passed all 28 foundation tests and all 47 VST3 validator
tests. Both downloaded packages contain the embedded Orbitron font and its OFL
notice; ZIP integrity checks passed.

The compiler and engine/VST3 tests are not an interactive editor-close test.
Repeated open/close on the affected Retina REAPER host, confirmation interaction,
and multiple simultaneously open editors still require manual host verification.
The diagnosis is supported by the matching stack traces and source ownership
path; the user's specific host reproduction has not yet been rerun with this build.

Category submenus, Templates, clickable links, update navigation and UI scale
settings from the notes are separate subsequent work, not claimed in this change.
