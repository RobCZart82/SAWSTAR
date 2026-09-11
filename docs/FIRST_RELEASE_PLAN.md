# SAWSTAR — first public release plan

Planning baseline: 2026-09-10. This is the consolidated forward plan; older
bootstrap milestones are historical and do not imply that existing synthesis
features must be implemented again. Completed DSP safety checks are recorded in
DSP_SAFETY.md. No new concept renders are required.

The first public release will be **1.0.0**, tagged **v1.0.0**. Current 0.1.0
development builds are not that release. Set the version and actual release date
when preparing the accepted release candidate; do not label a development build
as a published release.

1. **Finish engine lifecycle QA.** Audit remaining smoothing targets across host
   Reset/sample-rate changes and parameter restoration. Exercise automation,
   transport, offline rendering, note overlap, sustain, voice stealing, ARP and
   multiple instances. Extend own-code compiler warnings. Fix demonstrated
   defects while preserving the simple instrument scope.
2. **Sound, CPU and level acceptance.** Check common sample rates/block sizes,
   maximum polyphony, both oscillators and effects/modulation together. Measure
   CPU, clicks, tails, finite output and level balance; perform musical listening
   tests. Existing successful tests remain evidence, but are not a substitute
   for acceptance of the final binary.
3. **Finish functional GUI/preset acceptance.** Test all controls and automation,
   saved HOW IT WORKS previews, Save As/Rename/Delete/Initialize, multi-file
   import, favorites and concurrent instances. Specifically finish long-list
   scrolling/dragging and Windows text/keypad/GUI checks. Add fixed build/release
   date beside the version under the logo on all pages: development example
   `Version: 0.1.0 · Build: 2026.09.10.`, public release uses `Release:`. Embed
   metadata during build/release creation, never derive it from launch date.
   Retain the commit identifier in About.
4. **Final GUI cosmetics.** Follow the approved concept: white wordmark and
   aligned motto, subtle blue controls, colored MAIN headers/outlines, standard
   ADVANCED styling, consistent separators, fonts, dropdowns, dark text entry,
   focus/hover/inactive states and narrow scaled sliders. Meter zones remain
   approximately 70% green / 20% yellow / 10% red. Check all three pages at
   75/100/125% on macOS and Windows. This is a final consistency pass, not a
   reimplementation of already completed styling.
5. **Templates and Factory content.** Rename the visible Init category to
   Templates; keep the neutral Init sound and distinct Initialize action.
   Add Pad-Init, Lead-Init, Pluck-Init and useful starting patches. Upgrade all
   existing Factory presets, including SuperSaw One and Soft Pad One; add a
   curated embedded library with balanced levels, categories, tags and helpful
   saved signal-flow explanations. Exact count is not fixed. Factory sounds ship
   inside the plugin; user sounds remain individual .sawstar files with batch
   import. Migrate retained development presets if needed, with backups; exact
   compatibility with every unreleased experiment is not a release constraint.
6. **Release candidate.** Freeze features and factory content, rerun automated
   tests/validators and final macOS/Windows REAPER acceptance. Check clean
   installation, project recall, automation, editor reopen, offline render and
   multiple instances. Fix blockers and retest affected paths.
7. **Distribution and documentation.** Update the stale README/milestones,
   installation instructions, real GUI screenshots, supported architectures,
   known limitations and changelog. Review bundled licenses/notices/fonts and
   preset redistribution rights. Decide and document signing/notarization and
   packaging requirements before publication; do not claim unsupported targets.
8. **First public GitHub Release — 1.0.0 / v1.0.0.** Attach verified
   macOS and Windows VST3 packages with embedded presets and checksums, publish
   release notes and installation instructions, and verify downloads and the
   plugin's Releases link. Publishing the release is a separate final action
   after release-candidate acceptance; this plan does not publish one now.

AU/CLAP, additional platforms, large modulation expansions and an automatic
updater are not required for the first VST3 release. The instrument remains
focused on approachable leads, pads and plucks.
