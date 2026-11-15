# Stable-package link validation audit — 15 November 2026

The audit reproduced a release-gate gap on main cd8785f: inserting a link to a
missing document into INSTALLATION.md and recomputing its manifest hash was
accepted by validate_package. Integrity alone did not establish document-link
completeness. This is not evidence that the published 1.2.2 archive is broken.

Move the existing RC Markdown destination validator into release_validation and
reuse it for stable archive creation, stable release-gate verification and RC
finalization. Check inline links/images and reference-definition destinations;
resolve relative paths (including URL escapes) against the complete package
inventory, including PDFs and directories. External URLs and local fragments
remain allowed. Invalid links are rejected before a new stable archive/checksum
is exposed; the publisher also rejects consistently rehashed invalid archives.
Remote URLs and fragment existence are not fetched or validated.

Update current README/manual index, release notes, roadmap/index and checklist
wording to reflect the already published 1.2.2. Preserve the dated preparation
history and exact final release evidence. No release/tag/package replacement,
DSP/automation/K1 changes, numerical/CPU thresholds or test relaxation.

Local validation and exact-head remote CI results are recorded in the PR.
Implementation, local tests, remote CI and native host acceptance remain
separate; no new native acceptance or K1 resolution is claimed.

Local results: 33/33 release guards and 9/9 RC packaging regressions passed.
The unmodified downloaded 1.2.2 Windows x64 archive passes the new validator;
the same missing-link/rehashed reproduction now rejects. Producer regression
confirms no new ZIP/checksum appears on rejection and validates a successful
archive containing a PDF link. All script syntax, synchronized metadata and
diff checks passed. No new audio/CPU campaign was run for this packaging fix.
