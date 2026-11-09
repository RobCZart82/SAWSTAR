# 1.2.1 RC packaged Markdown link follow-up — 2026-11-09

Status: a real package-content defect was found and a source/test fix is in
progress. No corrected RC has passed CI yet; the current candidate remains
unaccepted and unpublished.

## Evidence

- Main source: `75fe83f6a0aa3b8fd8d128dbe25184fc513f4ad1` after PR #147. Its five
  push workflows and all 12 jobs passed; no PR was open at the start of this
  follow-up.
- Premium RC workflow dispatch: run
  [42643021881](https://github.com/RobCZart82/SAWSTAR/actions/runs/42643021881),
  exact source `75fe83f6a0aa3b8fd8d128dbe25184fc513f4ad1`. Metadata and both
  platform jobs passed: Windows x64 in 6m30s and macOS Universal in 8m54s.
- Downloaded Windows artifact
  [13686413068](https://github.com/RobCZart82/SAWSTAR/actions/runs/42643021881/artifacts/13686413068)
  has SHA-256 `a43028f72fdc198fcafc502dbd6a327cfbd1d35c343148299b5dec580769c57e`;
  its ZIP has 31 entries, a valid CRC and 30/30 matching payload hashes.
  Downloaded macOS artifact
  [13685499926](https://github.com/RobCZart82/SAWSTAR/actions/runs/42643021881/artifacts/13685499926)
  has SHA-256 `b32db27daac99751c5f0438fbb3801fb42be0de22908eb0bd573e0ede667b5bd`;
  its ZIP has 33 entries, a valid CRC and 32/32 matching payload hashes.
- Both RC manifests record the exact main commit, verified managed-dependency
  provenance, `validator_passed=true`, and correctly leave native, listening
  and publication acceptance false.
- A separate audit of all Markdown links in both delivered ZIPs found the same
  two broken relative links in `docs/PREMIUM_PRODUCTION_RC.md`: the 1.2.0
  release notes and the factory-evidence document were not shipped. Therefore
  neither otherwise hash-valid artifact is an acceptable test candidate. This
  finding is additional to the `SYSTEM_REQUIREMENTS.md` link corrected by #147.

## Fix in progress

- Point the two historical references to their canonical repository pages and
  make the packaged RC supplement clearly distinguish 1.2.1 candidate
  acceptance from the 1.2.0 approval.
- Validate every relative Markdown link against the files in the completed RC
  ZIP before hashing and accepting the package.
- Add a passing and a broken-link regression fixture to
  `tests/premium_rc_package.py`.
- Automated review found that the original validation failure path could leave
  the already-created ZIP at its final filename for the workflow's
  always-run artifact upload. The package is now built and finalized in a
  temporary sibling directory; only a fully validated checksum and ZIP are
  moved to final names, with the ZIP exposed last. A regression test asserts
  that rejected candidates leave neither final file nor staging directory.
- A second audit of the successful PR RC artifacts found stale claims later in
  the supplement: the About label still named 1.2.0 rc1 and the factory section
  still described the historical 24-preset/144-case grid. The 313e0e7
  Windows/macOS packages had internally matching manifests, ZIP/payload
  hashes, PDFs and 252-row factory CSVs, but are not suitable for user testing
  because their own supplement contradicted the 1.2.1 candidate and the CSVs.
  Their ZIP hashes are Windows
  `169863b95b724b57a5cc7e7a5f2f8a7a41bcdd1bf142a23ef1601f4d3acb2105` and
  macOS
  `4e8b2c1d78b627a99e84d3b6c3061788e3e5656dfd3efb1ca8efb6ff67db09a3`.
- Correct the supplement to the current 1.2.1 rc1 label and 30-preset grid:
  180 base cases plus 72 controller-use cases, 252 total per architecture.
  Add a regression test binding the supplement to current release metadata and
  those current counts.
- Local verification so far: eight package/provenance/documentation tests pass;
  Python syntax compilation, the complete corrected-package link scan, release
  metadata consistency and `git diff --check` pass. Exact-head PR CI and rebuilt
  artifacts remain required.

## Windows test encoding correction

The first run of head `972a1c8` found a platform-specific issue in the new
documentation regression itself: Windows Python used cp1252 as the default
`Path.read_text()` encoding and could not decode the UTF-8 Hungarian supplement.
The Windows RC job stopped in the package test before configuring or building
the plugin; no Windows RC package was created. The macOS RC integration job on
the same head succeeded. The test now explicitly decodes UTF-8, matching the
repository's Markdown encoding. The local package suite passes 8/8 with the
explicit UTF-8 read. Commit `e16274f` contains this portability fix and the
preserved CPU evidence. It was held locally until every `972a1c8` job finished,
so none was canceled.

Head `972a1c8` closed with 8 of 14 jobs passing. Five Windows jobs failed on
the same decoding exception: the Windows foundation Debug, Release, x64 VST3
and ARM64 VST3 jobs, plus the Windows Premium RC integration job. The remaining
failure was the macOS Release CPU comparison described below. macOS Debug,
Universal VST3, macOS RC, all three Linux quality jobs, and workflow validation
passed. These were one test portability defect and one preserved CPU-measurement
negative, not a general build failure.

## Preserved macOS CPU measurement on head 972a1c8

The macOS Release check failed only the `full_fx`, one-voice CPU comparison.
The preserved manifest reports baseline median 1.8377%, current median
2.9358%, and an allowed maximum of 2.8377% (the larger of the absolute and
relative limits). All 18 raw CSV records and their manifest hashes are saved
under [`ci-148-972a1c8-macos-cpu`](measurements/ci-148-972a1c8-macos-cpu).

For context, the successful main run 42619218535 used the same runner image,
Python version, workload SHA, baseline binary SHA and current binary SHA. Its
`full_fx`, one-voice medians were baseline 1.2871% and current 1.4886%, within
the existing gate; its 18 raw records and manifest are preserved under
[`ci-148-75fe83f-main-macos-cpu`](measurements/ci-148-75fe83f-main-macos-cpu).
The PR run's failure is therefore preserved as a real measured negative, but
the PR contains no DSP source changes and the two runs used identical binaries.
The difference is consistent with run-to-run measurement/runner variation;
that is an inference, not a proven root cause. We are not repeating the CPU
measurement to seek a more favorable value, and this packaging/documentation
fix does not claim to resolve it.

K1 remains unresolved. This link packaging issue does not change DSP, presets,
parameter IDs, state, release metadata or any K1 evidence. No release, tag or
publication was made.
