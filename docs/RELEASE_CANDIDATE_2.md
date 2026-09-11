# 1.0.0 RC2 - presentation and distribution candidate

Binary source: `d515c7cdd9e2228da8fd092d8f72912cf1cd89d6`.
Built 2026-09-11. No public release or v1.0.0 tag is created by this record.

## Delivered changes

The motto moves 5 px toward the wordmark. About centers Developed by / Gyuricza
Róbert and retains the GYR mark. DSP CPU samples its display every 250 ms with
additional smoothing. Scope display frames are held about 90 ms, with gentle
peak-scale decay; the audio capture remains unchanged and hidden views continue
to drain incoming frames. No synthesis parameter or preset schema changed.

Windows packages now have native x64 and ARM64 builds. macOS contains both
arm64 and x86_64 slices, with explicit deployment target 11.0. Windows builds use
static MSVC runtime. Final import audits found only Windows system DLLs and
OpenGL32, with no separate MSVC runtime DLL dependency.

## Validation

- [macOS run](https://github.com/RobCZart82/SAWSTAR/actions/runs/34580535423)
- [Windows run](https://github.com/RobCZart82/SAWSTAR/actions/runs/34580535465)

All three VST3 jobs passed 35 tests. Windows x64 and native ARM64 each passed
47 VST3 validator tests. macOS passed 47 on arm64 and 47 on translated x86_64
under Rosetta; this is not native Intel-machine GUI acceptance.

Windows installers were installed and uninstalled on disposable CI runners.
Checks verified binary identity, both manuals and preservation of a user-library
sentinel file. The macOS PKG was installed on its CI runner; binary identity and
both manuals were verified. Downloaded artifact digests, installer SHA-256 files,
inner ZIP checksums and every package-manifest hash were checked locally.
macOS PackageInfo explicitly reports relocatable=false and the system-wide path.
All seven jobs finished successfully, including the macOS and Windows
foundation Debug and Release jobs.

## Files

| Asset | SHA-256 |
| --- | --- |
| SAWSTAR-1.0.0-rc2-Windows-x64-Setup.exe | b87c05b2e70aee2955efe7665bbe581a60b3583a6cfcf873e1dc46ce2e781061 |
| SAWSTAR-1.0.0-rc2-Windows-ARM64-Setup.exe | 476b5aaf58b2b7e9e3cece7352c8fa05b0bd0d379018219b4ea564d1022e1ca6 |
| SAWSTAR-1.0.0-rc2-macOS-Universal.dmg | 1789562fa7c5013d3764c58ca6fb375516a747a98c8ccec421db8d93a994e642 |
| SAWSTAR-1.0.0-rc2-macOS-Universal.pkg | fbb650a71243d15e51802356db5e048f81caad63ff67bd43fe0c66027070dea1 |

Actions artifact IDs: macOS 10191515800, Windows x64 10191533613,
Windows ARM64 10191559344. The DMG contains the same PKG supplied separately.

## User-reported functional acceptance

On 2026-09-11, following the discussion of unsigned macOS installation, the user
reported: "Kipróbáltam és működik minden jól" (tested, everything works well).
Record this as successful user-reported functional testing in the current RC2
context. Exact package, OS/host versions, architecture, download/quarantine state
and any security prompts or approval steps were not specified. Do not infer
clean-machine Gatekeeper acceptance or minimum-OS coverage from this report.
The installation guide's exact macOS approval steps still require that detail.

## Not yet claimed

- Native Intel Mac or physical Windows ARM64 DAW/graphics acceptance.
- Tested minimum OS versions, RAM requirements or every host/display scaling.
- Developer ID signing/notarization or Windows Authenticode signing.
- Final 1.0.0 PDF edition metadata and updated real GUI/About screenshots.

RC1 user Windows approval is retained in RELEASE_CANDIDATE.md; it does not
implicitly approve this changed GUI and installer. Current candidate manuals
are explicitly the earlier development edition. ZIP release-note text belongs
to that build snapshot; this record is the authoritative RC2 validation summary.
Final packaging, documentation, supported minima and signing policy must be
accepted before the actual release date, final rebuild, tag and publication.
