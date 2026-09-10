# Overlapping MIDI notes

SAWSTAR pairs repeated Note On messages with Note Off messages per MIDI channel and pitch. Repeating the same key retriggers one voice; the first release does not stop a key that still has an outstanding press. Velocity-zero Note On is a release. Extra releases are ignored.

This applies to Poly, Mono and Legato, including the arpeggiator input. When ARP is disabled during overlapping input, it restores one voice per held key and waits for the matching final release. Sustain and ARP Hold remain separate from physical key counts. Channel All Notes Off / All Sound Off clear counts; Reset Controllers preserves physical presses. Reset and voice-mode changes start a new phrase and clear input bookkeeping. Counts saturate rather than wrapping.

## Validation

`tests/overlapping_notes.cpp` checks paired releases, sustain, channel isolation, panic, reset, voice-mode changes, voice stealing, ARP operation and ARP-to-bypass transitions in all three voice modes. `tests/mono.cpp` also checks paired repeated notes at 44.1, 48 and 96 kHz.

All 30 standalone test programs passed locally with AddressSanitizer and UndefinedBehaviorSanitizer on 2026-09-10. Plugin builds and host validation are performed separately; these engine tests do not substitute for a DAW test.

## Build and host acceptance

Code commit: `eff0ac7c61be82df3f50f35aaaf0cc2de3f441ae`.

- All six macOS/Windows CI jobs passed (Debug, Release and VST3 Release). Each VST3 build passed the 30 engine/foundation tests and 47 VST3 validator tests.
- The macOS artifact was checksum-verified and installed in the user VST3 directory. Binary SHA-256: `bbd5af8d19a0b02d104a6bd8d142d39d29e79c25dfb54dcd2cda50d26b7250f7`.
- REAPER 7.79 on macOS, 44.1 kHz / 512 samples: separate Poly, Mono and Legato projects sent same-channel C4 Note On at 0 and 2 seconds, Note Off at 6 and 22 seconds. With effects disabled and a short release, the host meters remained active between releases and fell silent after the final release, before the test's cleanup controller. Poly was observed at 19.051 / 31.892 seconds, Mono at 18.877 / 27.909 seconds, Legato at 9.914 / 23.648 seconds (repeat passes where needed).
- Host acceptance used visual output-meter observation, not a recorded WAV or subjective listening comparison. ARP transitions, sustain, panic and stealing were covered by automated tests; they were not repeated manually in REAPER in this run.

CI: [macOS](https://github.com/RobCZart82/SAWSTAR/actions/runs/34480461446), [Windows](https://github.com/RobCZart82/SAWSTAR/actions/runs/34480461441).
