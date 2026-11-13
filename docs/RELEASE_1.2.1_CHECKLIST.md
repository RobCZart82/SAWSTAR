# SAWSTAR 1.2.1 stable build preparation

Status: preparation authorized on 12 November 2026; not published.
The public release remains 1.2.0. The metadata edition `2026.11.12.` is the
prepared build date, not evidence of publication. If publication is deferred,
review that date before the final main build.

## Scope and decisions

- Prepare stable metadata, version-specific notes and bilingual manuals.
- Retain the calibrated Premium engine, 30 factory presets, existing GUI,
  plugin/parameter IDs and preset/state formats.
- Disclose the existing block-based parameter-automation limitation; no
  automation timing or DSP change is included in this preparation.
- Keep K1 unresolved and accurately disclosed. Current passing tests and
  prior host feedback do not establish a numerical fix.
- Preparation does not authorize public release, tag creation or replacement
  of any 1.2.0 public asset.

## Baseline evidence

The source baseline is `a26325475534289ae5a5ff77f794a5660ee8c29c`.
All four required push workflows and their 11 latest-attempt jobs passed:

| Workflow | Run | Result |
| --- | --- | --- |
| Windows Debug/Release and VST3 x64/ARM64 | [43088036845](https://github.com/RobCZart82/SAWSTAR/actions/runs/43088036845) | 4 jobs successful |
| macOS Debug/Release and Universal VST3 | [43088036743](https://github.com/RobCZart82/SAWSTAR/actions/runs/43088036743) | 3 jobs successful, attempt 2 |
| Coverage, ASan/UBSan and TSan | [43088036811](https://github.com/RobCZart82/SAWSTAR/actions/runs/43088036811) | 3 jobs successful |
| Workflow lint | [43088037063](https://github.com/RobCZart82/SAWSTAR/actions/runs/43088037063) | 1 job successful |

The baseline local Windows x64 MSVC Release run passed 39/39 foundation tests
and 181/181 Premium-production tests (overlapping suites). A separate adapter
probe confirmed final-point coalescing and early application of future-offset
normal parameters/bypass. No actual DAW automation render was performed.
The [11 November RC feedback](DEVELOPMENT_LOG_2026_11_11_1.2.1_HOST_ACCEPTANCE.md)
retains its original tested-commit and platform limits.

## Final preparation gates

- [x] User authorized release preparation, with publication kept separate.
- [x] Stable metadata and notes prepared; public-download links still point to 1.2.0.
- [x] K1 and normal-parameter automation limitations disclosed in both languages.
- [x] Regenerated both 15-page PDF manuals; rendered all 30 pages and checked character bounds.
- [x] Metadata synchronization, all 28 release-guard tests and all seven build-display cases passed; the five focused CTest checks passed.
- [x] Verified the corrected head of [preparation PR #150](https://github.com/RobCZart82/SAWSTAR/pull/150).
- [x] Inspected the new Windows x64/ARM64 and macOS Universal package inventories,
      hashes, version labels, document links and known-issue text.
- [x] After integration, verified all required push workflows on the exact main
      commit and the private release draft produced from those packages.
- [ ] Record final host spot checks where available; do not infer unspecified
      OS/DAW/CPU/dropout/native ARM64 acceptance from earlier feedback.
- [ ] Obtain explicit final publication approval; publish only the verified draft.

Update results with exact revisions and evidence. Prior baseline CI is not a
substitute for checks on the preparation PR or the final main commit.

## Local preparation evidence

Both PDFs retain the historical 1.0.3 screenshots and include 1.2.1 edition
metadata, K1 and automation disclosures. Both have exactly 15 pages; contents
list chapter 15 once. All pages were rendered and visually inspected, and no
character extended beyond its page bounds.

- EN PDF SHA-256: `da45fd7fb30cbbca56bafca780600ba3f06764a9bf020b1b5818b71c9142fe4c`.
- HU PDF SHA-256: `8682d2e0caab63101111631a44090706ec32089b11afdf9cedb900fff5fd82d6`.

The five focused CTest checks cover Premium production state, identity/latency,
preview package contracts, release guards and build display metadata. They reuse
the existing offline build; full platform/plugin validation of the new head
remains a separate CI gate. No new DAW host acceptance is claimed.

## First preparation-head CI diagnostic

The first PR head `35ad30f2d49f09ad74067b9463e96f4441191420` exposed an RC-only
documentation assertion in `premium_rc_packaging_contract`: with an empty
candidate it expected the invalid label `1.2.1 -premium-integration` in the
preserved RC supplement. The [Windows Release job](https://github.com/RobCZart82/SAWSTAR/actions/runs/43190849704)
finished 172/173 tests successfully and failed this assertion. This is a real
failed preparation run, not accepted CI evidence.

The correction retains current-RC checks when RC metadata is selected, marks
the preserved RC supplement as historical for stable/dev metadata and adds
negative coverage proving that the RC packager still refuses stable/dev builds
before provenance inspection or validator execution. No packaging or release
safety gate is disabled. Fresh checks on the corrected head are required.


## Final main and private draft verification — 13 November 2026

PR #150 merged on 12 November at
`b2e799802192f9bbdab90f55f2c2c52857d8fbd1`. All exact-main push runs passed:
[Windows](https://github.com/RobCZart82/SAWSTAR/actions/runs/43241970798),
[macOS](https://github.com/RobCZart82/SAWSTAR/actions/runs/43241970125),
[Code quality](https://github.com/RobCZart82/SAWSTAR/actions/runs/43241970438),
[Workflow lint](https://github.com/RobCZart82/SAWSTAR/actions/runs/43241970616)
(11 jobs), and the
[verified draft preparer](https://github.com/RobCZart82/SAWSTAR/actions/runs/43241970817)
(one job). PR evidence is retained above; these fresh push results qualify the
actual final main commit.

The private `v1.2.1` draft (release ID 443139129) targets that exact SHA. All
seven package files match their main CI artifact bytes; SHA256SUMS.txt covers
all seven, making eight assets total. Internal manifests, CRC/file hashes,
relative Markdown links, both PDF bytes and release-note content were checked.
Windows PE x64/ARM64 and macOS Universal x86_64/arm64 architecture checks passed.
The prepared metadata date remains `2026.11.12.`. There is no public 1.2.1
release or tag at this verification; Latest remains v1.2.0. No new host
acceptance is inferred. Publication remains a separate explicit decision.

Further development, starting with the UTF-8 tooling follow-up, must preserve
this verified build and draft. New-branch CI and later fixes are distinct from
acceptance or publication of these frozen 1.2.1 assets.
