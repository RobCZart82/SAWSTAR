# SAWSTAR 1.2.1 release-preparation audit — 2026-11-08

Status: implemented and merged in [PR #145](https://github.com/RobCZart82/SAWSTAR/pull/145),
squash commit `378bc7ebbab49c04a693a450d19f1226a0ba8a22` on 8 November 2026.
The latest published release remains 1.2.0; this work did not publish a new
candidate or release.

## Corrections prepared

- Windows installer staging preserves the package's `docs/` hierarchy. Manuals,
  installation/system documents, third-party details and version-specific
  release notes now install there; Start Menu shortcuts use the same location.
  The disposable Windows installer smoke compares installed documents to the
  extracted, hashed package. Actual Windows installer execution remains a CI
  check.
- macOS documentation is flattened under its Application Support directory.
  The packaged notice now rewrites both license and third-party-document
  references to their actual installed destinations. The artwork provenance
  reference points to the repository because that source-only file is not
  included in release packages.
- Release metadata synchronization now checks exactly one occurrence of each
  framework version/copyright macro and each selected field in the Classic,
  Premium Preview and Premium Candidate plists. It preserves product bundle
  identifiers. The two stale Premium plist versions were brought in line with
  the published 1.2.0 metadata.
- The private release-draft gate requires exact-head Windows, macOS, quality
  and workflow-lint push runs. It also waits for every other exact-head push
  workflow to finish successfully, accounts for failures with no jobs, and
  excludes the release preparer itself.
- Current English/Hungarian manual sources now summarize all three preserved
  K1 negative cases and the missing original Sine executable. The published
  1.2.0 release notes retain the exact evidence links; regenerated source PDFs
  do not alter assets already published on GitHub.
- Release 1.2.0 checklist status is marked published and its old open-gate
  tables are explicitly historical. README, changelog, development index,
  roadmap and next-release plan point to the unpublished 1.2.1 preparation
  plan. Third-party pin and shipped root-notice hashes were rechecked without
  expanding the earlier license-scope review.

## Verification completed locally

- `tests/release_guard.py`: **27/27 passed** after the last test changes.
- Full existing configured CTest suite: **182/182 passed**. No C++ sources or
  build targets changed in this preparation.
- `scripts/sync-release-metadata.py --check`: passed; all three VST3 plists
  pass `plutil -lint`.
- Python syntax compilation and `git diff --check`: passed.
- Relative links in the 13 changed/current release and documentation-index
  files resolve locally.
- English and Hungarian PDFs regenerated for public 1.2.0 metadata, 15 pages
  each. All 30 pages were rendered and visually inspected; text extraction
  confirms the current version and K1 disclosure, with every text character
  inside the page's safe margins. The generated PDFs are source-repository
  updates, not replacement release assets.
- iPlug2, DaisySP and nested VST3 SDK pins match the documented revisions;
  the matching root license files and shipped copies have identical SHA256s.

## Remote verification

- PR head `90d8d4de71ad259a5a37256e94fc50a332df28a9`: nine workflows completed;
  25 executed jobs passed and the candidate-only Premium integration job was
  skipped because metadata remained stable 1.2.0. No review was outstanding.
- Merge commit `378bc7ebbab49c04a693a450d19f1226a0ba8a22`: all five main
  workflows and 12 jobs passed. The release-preparation job left the existing
  published `v1.2.0` unchanged. No new tag or public release was created.
- Exact-candidate host testing and publication approval remain separate gates.

## Still requires remote or user evidence

- The exact PR head must pass all applicable CI jobs, especially Windows
  installer install/uninstall/document-path smoke and macOS package layout.
- No 1.2.1 candidate was built, installed or tested in REAPER. RobCZart82 must
  review and accept an exact candidate and separately authorize any later
  publication.
- K1 remains unresolved; the corrections in this PR do not claim to identify
  or fix it. Keep the three negative records and all later non-reproduction
  results distinct. Native CPU/dropout, filter/listening and platform support
  limits remain as documented for 1.2.0 unless new exact-build evidence is
  collected.
