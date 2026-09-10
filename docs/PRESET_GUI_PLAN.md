# Preset reliability and GUI completion plan

The September 10 user notes are consolidated here. The final concept remains the layout reference. Duplicate MIDI-note handling was completed separately; see OVERLAPPING_NOTES.md.

## Implemented in this change

- Favorites updates lock the full read/modify/replace operation across instances and processes. An exclusively created temporary file is flushed before atomic replacement. A failed update preserves the previous file. Locks and disk work are confined to preset/UI operations, never the audio callback.
- External Save As remains the active saved sound when a library copy is skipped or fails. Its saved diagram remains available in the browser for the active session. Existing library sounds are not overwritten.
- Cross-platform reserved names are rejected. UTF-8 names allow up to 80 code points subject to a 240-byte filesystem budget. Extension matching is case-insensitive. Case-only rename is supported, and a normal rename cannot overwrite an existing target.
- Rename/delete report a favorites update failure rather than masking it with a success message. Explicit delete removes its favorite key; externally missing files are not automatically purged, to preserve favorites if the file returns.
- The preset browser uses a vertical draggable scrollbar and mouse-wheel scrolling, replacing Previous/Next pagination. The scrollbar remains visible but inactive when all rows fit.
- Popup menus use opaque dark graphite backgrounds, light text and a muted blue hover. Full control rectangles anchor custom menus; the framework centers below where space allows and constrains menus to the editor.
- Search, rename and parameter-entry fields share a dark background and light text, with blue selection/caret while editing. Native file dialogs retain OS styling.
- Pitch/modulation captions are centered under the wheels. Effect enable buttons are narrower than effect selection buttons.

## Next: version and settings

- Small prerelease version below the existing logo, with build identification; keep host-compatible numeric version fields and distinguish development display text.
- Align the two-line motto with the wordmark and add the restrained, tracked, bold footer “The Sounds of Trance”.
- Gear menu: GUI Scale (75/100/125%), Updates / Downloads linking to GitHub Releases, separator, About. Add a separate Settings page only when it has additional useful global options. Check host resizing and editor reopen behavior before shipping scaling.

## Next: DSP lifecycle and QA

- Test Reset followed by restored Noise Color/Source and other smoothing targets; remove unintended startup transitions without removing normal live smoothing.
- Apply finite-value validation consistently at public DSP boundaries.
- Extend compiler warnings to own engine/plugin targets without changing third-party code; update milestones and packaging documentation.
- Preserve the deliberate All Sound Off recovery after MIDI queue overflow.

## Last: final visual polish

Once layout and behavior are accepted, unify subtle gradients, highlights, borders, typography, hover/active/disabled states and panel depth. Keep the dark restrained design, blue controls, MAIN header color coding and green/yellow/red meter. No new concept renders are required.

## Verification

Run the complete C++ suite and macOS/Windows VST3 CI. `preset_reliability` covers parallel instances, process contention on POSIX, partial-write failure on POSIX, reserved/Unicode names, uppercase extensions, case-only rename and external-save name collisions. GUI acceptance additionally checks dragging and wheel scrolling with more than 12 presets, short/empty filtered lists, popup edges and editing on each supported host. Automated tests do not establish visual acceptance.

### September 10 verification result

- Code commit `ded8215`: all six macOS/Windows Debug, Release and VST3 CI jobs succeeded. Each foundation suite passed 31 tests; each VST3 validator passed 47 tests.
- Local ASan/UBSan: 31 tests passed; the three preset-related tests were rerun successfully after the final persistence changes.
- REAPER 7.79/macOS: updated plugin opens, MAIN/ADVANCED/PRESETS switch successfully, FILTER popup is dark and centered below its source, search entry stays dark with light text, and searching `Pad` returns the two matching presets. Effect enable button sizing was visually checked. The scrollbar is correctly inactive with the existing 11-item library and filtered two-item list. Long-list dragging and Windows visual acceptance still need host testing.
- Both downloaded artifact archives matched the GitHub SHA-256 digests. The macOS VST3 was installed with a backup of the previous bundle.
