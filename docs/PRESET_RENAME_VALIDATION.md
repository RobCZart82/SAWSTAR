# Preset rename without hard-link support

The Windows and macOS preset rename path uses a native, same-volume operation
that refuses to replace an existing destination. Windows uses `MoveFileExW`
without `MOVEFILE_REPLACE_EXISTING` or `MOVEFILE_COPY_ALLOWED`; macOS uses
`renamex_np(..., RENAME_EXCL)`. Renaming no longer requires hard-link support
on these release platforms. The preset bytes are moved unchanged.

Name validation, case-only renames and cooperating-instance mutation locks
remain in `RenameUserPreset`. The filesystem operation also protects against
a destination created after the directory/name check. On macOS, a filesystem
that does not support exclusive rename reports an error; there is no unsafe
plain-rename fallback. Non-release platforms retain the hard-link path.

## Automated checks

`preset_reliability` checks raw-byte preservation, Unicode paths, an existing
destination, a missing source, and 32 races between two independent sources
renaming to one destination. Exactly one source must win; the losing source
and its bytes must survive. Existing preset lifecycle and case-only rename
checks still run.

```sh
ctest --test-dir build -C Release -R '^(preset_reliability|user_preset_files)$' --output-on-failure
```

Use the configured build folder and configuration for the local build.

## External-volume acceptance

The real-volume check is separate from ordinary CI. Point
`SAWSTAR_PRESET_TEST_ROOT` at a writable directory on the test volume, then run
`preset_reliability`. The test creates and removes its own uniquely named
subdirectory. Record OS, filesystem, build commit and result.

Windows PowerShell example:

```powershell
$env:SAWSTAR_PRESET_TEST_ROOT = 'E:\SAWSTAR-test'
ctest --test-dir build -C Release -R '^preset_reliability$' --output-on-failure
Remove-Item Env:SAWSTAR_PRESET_TEST_ROOT
```

macOS example:

```sh
SAWSTAR_PRESET_TEST_ROOT=/Volumes/TestDisk/SAWSTAR-test \
  ctest --test-dir build -C Release -R '^preset_reliability$' --output-on-failure
```

Test exFAT on Windows and macOS, plus the normal NTFS/APFS library. A native
GUI acceptance check should import an external preset, rename it, reopen it,
and confirm its favorite entry follows the new name. Automated tests alone
do not establish acceptance on every external filesystem.
