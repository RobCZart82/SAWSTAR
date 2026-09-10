# Compiler cleanup and host acceptance — 2026-09-10

Tested code: `a9319c026f4f3cbaeb783f7e4969285053dadce3`.
This is a development build, not the first public release.

## Changes

- Correct member initialization order, remove unused callback argument names,
  avoid a shadowed drawing variable and make DSP numeric conversions explicit.
- Windows preset paths use the wide-character environment API. Exclusive file
  creation uses the supported secure CRT API while retaining create-only,
  no-overwrite semantics for presets and locks.
- Update the README's current feature status and add installation instructions
  and a Windows manual acceptance checklist.

## Automated results

- All six macOS/Windows Debug, Release and VST3 jobs passed. Each ran 33 tests.
- Both VST3 validators passed 47 checks with zero failures.
- All 33 local C++ tests passed with AddressSanitizer/UndefinedBehaviorSanitizer.
- No compiler `warning:` / MSVC `warning C...` diagnostics appeared in the six
  final job logs. CI tooling still reports Node deprecations and a Windows CMake
  configuration warning; this is not a claim that every tool emits no warnings.
- Both artifact archives passed SHA-256 digest and ZIP integrity verification.
  The macOS bundle was installed after backing up the previous bundle; its
  executable matched the artifact and About displayed `a9319c0`.

Runs: [macOS](https://github.com/RobCZart82/SAWSTAR/actions/runs/34503865190),
[Windows](https://github.com/RobCZart82/SAWSTAR/actions/runs/34503865184).

## macOS REAPER checks

REAPER 7.79, 512-sample device buffer, isolated two-instance MIDI test project.

- Saved the two instances with different Output settings, quit REAPER, installed
  the new binary and reopened the project successfully.
- An Output envelope on instance A drove the displayed -18 / -9 / -27 dB steps
  during playback. The automation lane persisted in the project.
- After re-saving, decoded both 93-parameter plugin state chunks and compared
  them with the saved baseline: all 93 values on B were unchanged; all 92
  non-Output values on A were unchanged. A's Output followed its envelope as
  expected. This checks numeric parameter recall, not every metadata field.
- Closed and reopened the editor; controls and playback remained functional.
- Changed project sample rate from 44.1 to 48 to 96 kHz, observed playback,
  meter/MIDI-key feedback and the matching plugin sample-rate display. Stopping
  returned to zero active voices. Restored 44.1 kHz and saved the test project.

These observations are a focused host smoke test. They do not prove subjective
sound quality or the complete host acceptance matrix. No separate WAV was made.

## Still pending

- [Windows manual GUI/keyboard acceptance](WINDOWS_ACCEPTANCE.md).
- Broader final-binary automation (filter, waveform, effects and modulation),
  additional block sizes and offline-render acceptance in the host.
- Final musical listening and visual approval, Templates/Factory content,
  release-candidate and distribution acceptance from [the release plan](FIRST_RELEASE_PLAN.md).

The earlier [pre-release QA](PRE_RELEASE_QA.md) remains historical evidence for
the earlier build. Its compiler-cleanup follow-up is completed by this change.
