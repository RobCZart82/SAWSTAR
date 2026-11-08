# SAWSTAR 1.2.1-rc1 candidate preparation — 2026-11-08

Status: local candidate preparation is implemented and locally verified on branch `prepare/1.2.1-rc1`, based
on main `378bc7ebbab49c04a693a450d19f1226a0ba8a22`. The public release remains
1.2.0. Candidate metadata is explicitly `1.2.1-rc1` with no release date; no
tag, public release or exact-candidate user acceptance is implied.

## Corrections found while preparing the candidate

- Candidate-specific manual text incorrectly named 1.0.4 as the current public
  version even though 1.2.0 is published. The generic candidate notice now says
  the current public release remains unchanged.
- The candidate PDF was regenerated and text-extracted to verify its chapter
  references; the Premium integration RC is chapter 15 in the generated guide.
- Candidate manuals marked preset listening as still open, despite Robert's
  approval of the six exported new-preset snapshots. They now preserve that
  approval while making clear it is not acceptance of this exact 1.2.1 RC.
- The system-requirements page led with 1.0.4-rc1 host evidence even though the
  current 1.2.0 release and its 003b007 macOS/Windows approval are newer. The
  current release and older historical test cycle are now separated.

## Candidate work completed locally

- Set 1.2.1-rc1 metadata with an empty release date and synchronize the
  framework version and all three VST3 identity plists.
- Add candidate release notes and align the installation and bilingual manuals
  with the candidate artifact layout and preserved K1 evidence.
- Keep exact-host testing and any publication approval as later user gates.

- Local Debug build completed successfully; Debug CTest passed **174/174** in
  343.11 seconds, including factory-preset, chorus, premium, packaging, and
  release-guard checks.
- `tests/release_guard.py` passed **28/28**; release metadata synchronization
  check passed; all three VST3 plist files passed `plutil -lint`.
- Changed Python sources passed bytecode compilation, changed Markdown
  relative links resolved, and `git diff --check` passed.
- Both regenerated candidate manuals contain **15 pages**. Text extraction
  confirms the `1.2.1-rc1` identity and chapter 15 references; no stale claim
  that 1.0.4 is the current public release remains. All pages were rendered
  and visually reviewed, and text bounds remained within every page.

These are local results only. Exact-head remote CI, final RC package
verification and user testing of the exact 1.2.1-rc1 Windows/macOS packages
remain pending. K1 remains unresolved; no tag or publication has been made.
