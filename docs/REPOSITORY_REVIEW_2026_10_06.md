# Repository review — 2026-10-06

Reviewed main: `c53d025a6f9ef215b0d11a47771c0fc98717de1c`.
This review separates source-confirmed defects, CI evidence and outstanding native
acceptance. Green CI does not prove that every host/editor combination is correct.

## Confirmed fixes

| Finding | Trigger and previous behavior | Fix and regression |
|---|---|---|
| Installer types were not individually checked | macOS download contains two `.dmg` installers and no `.pkg`. The previous total-count assertion accepted both files. | Require exactly one installer of each required extension. Publisher subprocess tests reject the incomplete combination in ordinary and optimized Python. |
| Release guards disappeared under Python optimization | `python -O scripts/publish-release.py` omitted seven assertions, including missing release date/notes, expired artifacts and incomplete platform coverage. Existing explicit workflow/package validation still applied. | Replace every publisher assertion with an explicit exception; test real publisher subprocesses with and without `-O`, invalid date/notes, expired artifact, missing platforms, wrong installer type/version/prefix, and valid preparation. External GitHub/package I/O is mocked; existing package and draft tests remain. |
| Empty HOME produced a relative preset directory | `HOME=""` returned `Library/Application Support/SAWSTAR/Presets`, allowing a library relative to the working directory. | Reject empty environment strings as unavailable. The lifecycle test exercises unset/empty/nonempty HOME on macOS/Linux and restores the environment. The Windows APPDATA pointer/empty guard is included defensively; an empty Windows API result is not claimed as reproduced. |

Installer names now require the expected version prefix at the start, rather than
accepting an unrelated name containing it. This is a release-preparation failure,
not a claim that the existing published 1.0.4 installers are broken. Publication
remains a separate action.

## Development plan status at the reviewed revision

The development plan exists in `docs/DEVELOPMENT_PLAN_POST_1.0.4.md`; filter
research and raw measurement limits are in `docs/PREMIUM_FILTER_DEVELOPMENT.md`.

Completed research includes four filter modes, separate engine adapters,
compact/static FIR phases, FIR-only component controls, offline small-buffer
timing distribution and the separate scalar tanh candidate. These are research
components, not completion of the production filter replacement.

Still open: acceptable full-engine CPU cost and high-rate quality policy,
native target-host stress/acceptance, production latency/state/automation
integration, factory preset listening/update, manuals and the next release.
Preset snapshot ordering, synchronous import latency, hard-link-free rename and
ordinary parameter automation timing remain previously documented tasks.
The open #63 PR adds Drive/engine qualification; its head is outside this pinned
main review.

## Verification limits

The reviewed main has ten successful check runs (Windows, macOS and Linux quality).
The local Windows command runner cannot start because the managed networking
sandbox backend is unavailable. No local compiler, Python or audio-host test
result is claimed for this review. The new regression tests are part of the
existing `release_guard` and `user_preset_files` CTest targets, and the PR CI is
the verification source for the fixes. Native UI/audio acceptance is not replaced
by source review or CI.
