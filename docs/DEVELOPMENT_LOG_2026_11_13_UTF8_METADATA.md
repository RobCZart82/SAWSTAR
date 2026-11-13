# Release metadata UTF-8 portability — 13 November 2026

Source baseline: `b2e799802192f9bbdab90f55f2c2c52857d8fbd1`.
Scope: a separate development follow-up; the verified 1.2.1 draft and its
eight assets remain tied to that baseline. No publication is authorized here.

## Reproduced defect (U1)

`scripts/sync-release-metadata.py` read and wrote repository metadata using
Python's process-default text encoding. UTF-8 repository content is not
guaranteed to match that default when UTF-8 mode is disabled.

Two isolated subprocess probes force the default `Path.read_text` and
`Path.write_text` encoding to ASCII while preserving explicitly supplied
encodings. This is a portable model of a non-UTF-8 locale, not a claim that
every Windows installation defaults to ASCII.

- The real UTF-8 metadata fixture fails `--check` with `UnicodeDecodeError`
  when it reaches the accented creator name.
- An ASCII-only fixture with JSON-escaped Unicode metadata reaches repair,
  then fails with `UnicodeEncodeError` when writing a non-ASCII copyright.

Both new tests failed before the fix. The output-encoding fixture separates
write behavior from input decoding and includes Hungarian/Japanese characters
and XML-significant text. It operates only on disposable copies.

## Change and validation

Every metadata/config/plist read and generated-file write in the synchronizer
now explicitly uses UTF-8. Existing validation rules, version mappings and
product identities are unchanged. Existing fixture text reads also state their
encoding explicitly.

After the fix:

- `tests/release_guard.py`: 30/30 tests pass locally on Windows x64/Python 3.13.
- The full 30-test suite also passes with `PYTHONUTF8=0` and the actual Windows
  CP1250 process default; the real checkout metadata check passes in that mode.
- Both new non-UTF-8-default probes pass. `--check` leaves all fixture bytes
  unchanged for both current and stale metadata.
- Repair preserves the exact Unicode copyright in the C++ literal and all
  three XML plists, retains bundle identities, and passes a subsequent check.
- `scripts/sync-release-metadata.py --check` passes on the actual checkout;
  the generated metadata files and `release.json` have no content changes.

New-head remote CI is pending at this record's creation; use the follow-up PR's
Checks for exact-head results. Baseline green CI is not validation of this fix.
No new audio, CPU/dropout or native-host acceptance is claimed.

## Next bounded development steps

1. Validate this tooling follow-up on all existing required CI jobs, without
   changing the preserved 1.2.1 draft or releasing development packages.
2. Continue K1 root-cause work using the preserved negative cases and original
   provenance limits. Passing controls do not close the numerical issue.
3. Plan normal-parameter automation timing as a separate adapter/engine change
   with explicit sample-offset and block-size tests before implementation.
   It is not part of this encoding fix or the frozen 1.2.1 build.
