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
- Local verification so far: six package/provenance tests pass; Python syntax
  compilation, the complete corrected-package link scan and `git diff --check`
  pass. Exact-head PR CI and rebuilt artifacts remain required.

K1 remains unresolved. This link packaging issue does not change DSP, presets,
parameter IDs, state, release metadata or any K1 evidence. No release, tag or
publication was made.
