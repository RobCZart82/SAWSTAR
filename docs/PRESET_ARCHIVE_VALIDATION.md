# Preserve preset recovery backups

Delete moves a User preset to `.deleted`, then `.deleted-1`, `.deleted-2`, etc.
Each candidate is now attempted with the same no-replace filesystem operation
as Rename. An existing destination causes another candidate to be tried;
permission errors, a missing source, or unsupported exclusive rename fail
immediately. Existing recovery files are never intentionally replaced.

This closes the gap between the former `exists()` check and plain `rename()`:
a writer outside the preset mutation lock could create a destination in that
gap. On POSIX, plain rename could replace it. A dangling symlink also appeared
absent to `exists()` and could be replaced. The source preset and raw bytes are
preserved on failures, subject to the native rename contract documented in
[preset rename validation](PRESET_RENAME_VALIDATION.md).

`preset_reliability` checks occupied backup suffixes, forces an external file
to appear immediately before the real move, checks that only a collision is
retried, and rejects a missing source. Linux/macOS additionally check a dangling
symlink at the first backup name. Existing Save/Delete concurrency and favorite
cleanup tests remain in the test suite.

```sh
ctest --test-dir build -C Release -R '^(preset_reliability|user_preset_files|preset_library_import)$' --output-on-failure
```

Native REAPER acceptance should delete a User preset with pre-existing recovery
files, verify those files remain unchanged and the new recovery opens, then
check the active-preset and favorite indicators. External-volume acceptance
remains separate from ordinary CI.
