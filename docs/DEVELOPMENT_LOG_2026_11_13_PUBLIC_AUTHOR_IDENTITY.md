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
