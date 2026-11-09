# 1.2.1-rc1: package verification and remaining acceptance — 9 November 2026

## Exact source and CI evidence

Examined candidate: `afa2a65a365e2ff4679228ed8411bd1db5c05ad9` (main after #146).
Release metadata is 1.2.1, candidate rc1, with an empty release date.
This is not a public release or a publication approval.

All five exact-source push workflows completed successfully, with **12/12 jobs**:

| Workflow | Run |
| --- | --- |
| Windows foundation and VST3 x64/ARM64 | [42512247251](https://github.com/RobCZart82/SAWSTAR/actions/runs/42512247251) |
| macOS foundation and Universal VST3 | [42512247227](https://github.com/RobCZart82/SAWSTAR/actions/runs/42512247227) |
| Linux sanitizers, ThreadSanitizer and coverage | [42512247226](https://github.com/RobCZart82/SAWSTAR/actions/runs/42512247226) |
| Workflow validation | [42512247202](https://github.com/RobCZart82/SAWSTAR/actions/runs/42512247202) |
| Verified draft preparation | [42512247254](https://github.com/RobCZart82/SAWSTAR/actions/runs/42512247254) |

The Windows VST3 jobs report successful factory/processing/state validation,
headroom verification, architecture/runtime audit and disposable-runner
installation/uninstallation checks. The Universal job reports successful
validation, headroom checks on both slices and disposable-runner installation.
These are CI step results, not tests performed on Robert's machines.
The draft workflow's successful job is named `publish`; that name alone is not
evidence of a public release.

## Downloaded candidate evidence

All three Actions artifact ZIPs were downloaded and inspected on Linux.
Their contained plugin ZIP and installer SHA-256 sidecars matched the bytes.
All **86** manifest entries matched their hashes and source commit; file
inventories contained no unlisted files beyond PACKAGE-MANIFEST.json.

| Artifact | ID | Manifest files | Binary architecture |
| --- | --- | --- | --- |
| [Windows x64](https://github.com/RobCZart82/SAWSTAR/actions/runs/42512247251/artifacts/13629768959) | 13629768959 | 28 | PE AMD64 / 0x8664 |
| [Windows ARM64](https://github.com/RobCZart82/SAWSTAR/actions/runs/42512247251/artifacts/13630447102) | 13630447102 | 28 | PE ARM64 / 0xaa64 |
| [macOS Universal](https://github.com/RobCZart82/SAWSTAR/actions/runs/42512247227/artifacts/13630745527) | 13630745527 | 30 | Mach-O x86_64 + arm64 |

These links identify the **pre-correction** candidate examined here. Do not
describe their contents as including the documentation fix below. Actions
artifacts also have retention limits.

| Delivered file | SHA-256 |
| --- | --- |
| Windows x64 plugin ZIP | ce037a79353ef69bd6567c59818101b4800aa75073ce07be6834d98d98cefc98 |
| Windows x64 Setup.exe | 7aa15a2899ddbd82a3d4cc6eacc0af047f4e03130b7d6d6c23cf7b4def211d87 |
| Windows ARM64 plugin ZIP | 4ff3a540a02f7bf8bda678304a1683f92da0b9a498850139a5ceb84e845c9295 |
| Windows ARM64 Setup.exe | 07ef5c2771e0df1f36fb1b87e880091b5e990d745d5d44524e41df013bad47fd |
| macOS Universal plugin ZIP | e7eeb3cbf679b3e7a0411defe2b21740b308814bd358053552cf6c305ba874cf |
| macOS Universal PKG | abff26625ff9b081b2db68a0adc5d875f3c0876a8ba32f900bd480703601ea5f |
| macOS Universal DMG | efe29a5586bc2e4b3e3c94f7cddd9101eebfbd17f4ac8e57801028a6536398c3 |

The two packaged PDF manuals match the exact source Git blobs on all three
platforms, not merely each other. The macOS plist has version 1.2.1, the SAWSTAR
bundle identity and minimum OS 11.0. This checks metadata, not minimum-OS
compatibility. The CSV files contain measurement rows, not one row per preset;
their successful headroom CI steps do not imply native host acceptance.

## Confirmed defect found during package review

**P3: broken historical documentation link in every package.**

`docs/SYSTEM_REQUIREMENTS.md` linked to `RELEASE_1.0.4_CHECKLIST.md` as a local
file, but that historical checklist is intentionally not in the delivered
document inventory. Relative-link inspection of all packaged Markdown files
found this missing target in Windows x64, ARM64 and macOS ZIPs.

The source correction points to the checklist in the repository, like the
other historical references on that page. It requires online access. It does
not add an obsolete checklist to the installer or modify DSP/plugin source.
Simulated corrected package text resolves all inline relative Markdown links
in the three reviewed payloads.

Existing ZIPs/installers remain immutable evidence of the original defect.
The correction must pass its own PR CI and be rebuilt; new package hashes and
the exact corrected source commit must then be recorded before final acceptance.
No sound/parameter/preset/state-format changes are part of this correction.

## Ordered remaining work

1. Complete exact-head CI for the documentation correction, then check the
   resulting main builds. Re-download corrected packages and repeat inventory,
   hash and document-link checks. Use those new hashes for host acceptance.
2. Install the corrected Windows x64 candidate on the intended Windows machine
   and verify the installed manual/requirements/release-note links
   **(sajátgépes reaper)**.
3. Test that same candidate in native Windows REAPER: scan/load, About version
   and source, factory sounds/ARP, playback and stop, project save/reopen,
   preset save/load, editor close/reopen, and multiple instances
   **(sajátgépes reaper)**.
4. Repeat installation and those lifecycle/sound checks with the corrected
   macOS Universal candidate in native Apple Silicon REAPER
   **(sajátgépes reaper)**. Native Intel remains a distinct target; ARM64 Windows
   likewise requires an ARM64 host/machine for a native user qualification.
5. Record actual sample rate, block size, instance count and any CPU/dropout
   observation **(sajátgépes reaper)**. Do not infer zero xruns or numeric CPU
   results from a general positive listening report.
6. After exact-candidate user feedback, make a separate publication decision.
   Stable metadata, final packages and release assets must correspond to that
   approved source. Historical 1.2.0 approval does not accept this candidate.

Host evidence to fill in for each intended machine: package SHA-256, source
commit, OS/CPU, REAPER version/architecture, sample rate/block size, installation
and document checks, sound/ARP, state/preset/editor lifecycle, multiple instances,
observed glitches/CPU, tester and date. No fields have been filled by inference.

## K1 and development scope

K1 remains unresolved. The three preserved negatives and non-reproductions
remain distinct; successful current CI does not establish a root cause or fix.
The active-ARP deadline study and UI/RT review items from the November 5 audit
remain future development work. They are not silently introduced into an
otherwise packaging-only maintenance candidate.
