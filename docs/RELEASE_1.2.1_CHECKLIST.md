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
- [ ] Open a reviewable preparation PR and verify its exact head checks.
- [ ] Inspect the new Windows x64/ARM64 and macOS Universal package inventories,
      hashes, version labels, document links and known-issue text.
- [ ] After integration, verify all required push workflows on the exact main
      commit and prepare the verified private release draft from those packages.
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
