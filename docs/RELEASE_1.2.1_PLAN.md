# SAWSTAR 1.2.1 maintenance release — preparation plan

Status: `1.2.1-rc1` CI and reported macOS/Windows host testing complete; final
publication decision pending, 11 November 2026. The latest public release is
1.2.0 (`v1.2.0`). This
document records repository work and gates; it does not create a release tag or
authorize publication.

The exact `406e76dbe0f763e9fe3d5cfaa1150748f5939d52` main commit passed all four
required workflows and all 11 jobs on the successful attempt. The first
attempt had four workflow-level GitHub internal-server errors and produced no
jobs; the successful second attempt is the accepted CI evidence. The three
RC packages and their hashes are recorded in the
[11 November host-acceptance record](DEVELOPMENT_LOG_2026_11_11_1.2.1_HOST_ACCEPTANCE.md).
Robert reports that he tested SAWSTAR 1.2.1 on macOS and Windows, including
listening, loading and saving, and found it working correctly. No OS version,
host application version, Windows architecture or detailed CPU/dropout matrix
was supplied, so this report is not evidence for those unspecified cases.

The candidate remains unpublished `1.2.1-rc1`. Stable metadata, final release
notes/assets and publication require a separate explicit 1.2.1 release
decision. The K1 numerical test issue remains unresolved and must remain
accurately disclosed; successful host testing does not establish a K1 fix.

## Verified scope

The 1.2.0 release is the calibrated Premium production build with 30 factory
presets. Robert reported that the published plugin worked on his macOS and
Windows systems and approved publication on 7 November. This user confirmation
is evidence for that tested build only; it is not CI evidence, exhaustive CPU
or dropout measurement, Windows ARM64 host qualification, or acceptance of a
future 1.2.1 build.

The 1.2.0 notes and current manual sources now describe three preserved K1
negative cases. K1 remains an unresolved offline numerical test issue: the
records do not establish a shared cause, audible defect, or fix. The original
failing Sine executable was not retained. Later successful controls and the
separate, fixed Chorus ring-boundary defect do not resolve K1. Preserve this
distinction in any 1.2.1 release notes.

## Ordered preparation

1. Keep central release metadata at the latest published version while making
   maintenance changes. Before a candidate build, select 1.2.1-rcN with no
   release date; only a deliberately approved final release change may set
   stable metadata and an actual release date.
2. Keep version, short version, bundle metadata, minimum macOS version and
   copyright synchronized across all shipped VST3 identities.
3. Preserve a single documentation layout from package staging to installation.
   Check that installer links and installed files resolve to the same manuals,
   requirements, third-party information and version-matched release notes.
4. Require the exact source commit's Windows, macOS, code-quality and workflow-
   lint push runs to complete successfully before preparing a release draft.
   Jobless failures and any other failed or running push workflow for that same
   commit must block preparation; the preparation workflow must not wait on
   itself.
5. Regenerate the English and Hungarian manuals from the current public
   metadata, render and inspect all pages, and verify text bounds. Keep the
   version-labelled 1.0.3 screenshots explicitly historical. Do not rewrite
   or silently replace the published 1.2.0 release files or assets.
6. For a 1.2.1 candidate, validate package file inventories and hashes, manuals,
   installers, About metadata, plugin identities, presets, state migration and
   known-issue disclosures on the exact candidate commit. Check every relative
   Markdown link against the delivered package layout, not only installer docs.

## Required evidence before publication

- All relevant exact-head workflows and jobs are successful, with no pending,
  failed, jobless-failed or review-blocked result.
- Windows x64 and ARM64 packages/installers and macOS Universal package are
  built and validated as applicable; installed document files match the
  extracted release payload and links resolve in the delivered folder layout.
- The bilingual manuals and release notes match the candidate version and
  disclose K1 accurately. Package hashes and release assets match CI output.
- Robert's macOS and Windows test of SAWSTAR 1.2.1, including listening,
  loading and saving, is recorded above as user feedback. The exact OS/host
  versions and Windows architecture were not supplied. A separate explicit
  publication decision is still required; 1.2.0 approval does not carry
  forward automatically.

Do not claim K1 fixed unless a specific root-cause correction is independently
reproduced and validated. Do not present a passing retest as proof that the
preserved negatives were false. Do not publish, tag, or replace an existing
release as part of preparation alone.
