# macOS installed-document link gate — 17 November 2026

Following the stable ZIP audit fix, macOS PKG creation still lacked link
validation for its different installed documentation layout. The ZIP preserves
docs/, whereas PKG flattens primary documents and puts guides under manuals/.
Source/ZIP validation cannot qualify the rewritten installed destinations.

Validate the completed installed documentation tree after copying notices,
manuals and licenses, before codesign, pkgbuild or productbuild. Reuse the
shared validator without changing its policy. No current broken PKG link is
claimed; this closes a previously unguarded distribution path.

A regression executes the real packager's copy/rewrite/validation steps with
platform tools mocked. Existing documents reach pkgbuild; an injected missing
relative destination rejects before pkgbuild and leaves dist empty. This is
local package-preflight evidence, not execution of a macOS installer or DAW.

Local release guards 34/34 and RC package tests 9/9 passed. Exact-head remote
CI and main integration remain separate gates. No DSP, automation timing, K1,
version metadata, thresholds, released tags or published asset changes.
