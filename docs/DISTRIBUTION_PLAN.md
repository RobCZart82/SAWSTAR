# RC2 to 1.0.0 distribution plan

## Implemented for candidate validation

- Header spacing and centered developer credit.
- DSP CPU: audio callback measurement unchanged; editor sample at 250 ms,
  additional 75/25 EMA, one decimal. Scope: capture remains 30 Hz; displayed
  frames held about 90 ms, immediate stale/reveal handling, gentle peak release.
- Static MSVC runtime retained, explicit Windows x64/ARM64 matrix and correct
  arm64-win VST3 bundle path; validator uses the same target architecture.
- Universal macOS build with explicit 11.0 deployment target.
- Inno Setup candidate EXE; system-wide PKG inside DMG. User presets untouched.
- Public-facing README, requirements, installation, changelog and contributor guide.

## Publication gates

Successful compilation alone does not establish a supported OS minimum. Validate
both architectures and run clean-install/uninstall and actual-host tests.
Confirm final imports before advertising no separate VC++ Redist requirement.
Update screenshots and PDF edition metadata after the accepted GUI build.
Record exact tested OS/host configurations, finalize developer signing policy,
then rebuild with the actual release date and publish matching hashes/tag/assets.

No valid code-signing identity was found in the local keychain on 2026-09-11.
Signed/notarized output requires Developer ID Application, Developer ID Installer
and a Keychain notarytool profile. The script requires all three together;
without them it explicitly builds an unsigned candidate. No Apple credentials
are created, exported or committed by the workflow.

Reference: GitHub runner availability and Inno Setup architecture rules:
https://docs.github.com/en/actions/reference/runners/github-hosted-runners
https://jrsoftware.org/ishelp/topic_archidentifiers.htm
