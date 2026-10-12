# Contributing

Report bugs with SAWSTAR version/build ID, OS and architecture, DAW version,
steps to reproduce, expected and actual behavior. Include a minimal preset or
project only if it contains no private/licensed material you cannot share.

Keep the instrument focused on clear sound design. Discuss large feature or
preset-format changes before implementing them. Preserve parameter identities
unless a migration is explicitly planned. Audio processing must not wait for
the GUI or allocate/lock in its realtime path. Add targeted regression coverage
for behavioral fixes and follow docs/BUILDING.md for checks.

Do not commit credentials, local user presets or generated build trees. Keep
external license notices with bundled dependencies; do not change submodule
versions silently. Use original or explicitly licensed artwork and fonts.

## Keep development status current

When a milestone, acceptance decision, release gate or publication changes,
update the current overview in docs/DEVELOPMENT_PLAN_POST_1.0.4.md and
its matching status/order in docs/DEVELOPMENT_ROADMAP.md in the same PR.
Record the review date, source commit and available CI/measurement evidence.
After merge, reconcile merged/pending wording at the next status update;
never label pending checks or native acceptance as completed.
Keep historical sections dated and clearly marked as history. Preserve their
original measurement sources and negative results. Completed tooling or
synthetic tests do not establish target-machine CPU or native-host acceptance.
