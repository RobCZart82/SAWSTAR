# Public creator identity — 13 November 2026

At the creator's request, authored documentation identifies the creator only as
RobCZart82. This includes historical narrative attribution, legal holder credits,
PDF body text and author metadata, branding notices and documentation generators.
The vector signature and GYR monogram remain the visual creator identity.
The historical About screenshot containing a typeset private identity is removed
from the current tree and its README reference is replaced with a description.

Installer publisher and synchronized plugin copyright metadata use the same public
identity, so future packages cannot reintroduce the private identity through those
fields. Third-party legal attribution is retained. Existing local filesystem paths
in raw diagnostic records remain evidence paths, not creator credits. Historical
Git commits and external evidence archives are not rewritten.

The published v1.2.1 archives and installers were built before this correction and
still contain the old document/metadata credits. They require corrected package
builds; this source change does not claim to have replaced those public assets.
No DSP, preset, plugin ID, parameter ID, state format or numerical threshold changes.

Local validation passed: both regenerated manuals have 15 pages; extracted text
and PDF author metadata contain only the public creator identity; all 30 pages
were rendered and inspected and text bounds passed. Release guards: 31/31; RC
packaging regressions: 9/9; synchronized metadata and diff checks passed.
Exact-head remote CI remains a separate gate.

The creator selected a new 1.2.2 correction release instead of replacement of
published 1.2.1 assets. Stable 1.2.2 metadata, version-specific release notes and
publication checklist are prepared. This authorization includes publication of
the verified 1.2.2 draft after exact-head/main CI and package integrity gates.

Initial remote head b30ef76: nine workflows succeeded. macOS foundation run
43499312696 failed only because Universal job 132140610387 was not acquired
by a hosted runner despite repeated scheduling attempts. The job has no runner,
no steps and no log; its annotation explicitly reports hosted capacity failure
and macOS arm64 queue constraints. Debug and Release jobs succeeded. This is
infrastructure evidence, not a measured DSP/CPU failure. All runs completed
before uploading the authorized 1.2.2 metadata/manual follow-up. New-head CI
and final package validation are still required.
