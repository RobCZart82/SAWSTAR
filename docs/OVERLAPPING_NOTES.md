# Overlapping MIDI notes

SAWSTAR pairs repeated Note On messages with Note Off messages per MIDI channel and pitch. Repeating the same key retriggers one voice; the first release does not stop a key that still has an outstanding press. Velocity-zero Note On is a release. Extra releases are ignored.

This applies to Poly, Mono and Legato, including the arpeggiator input. When ARP is disabled during overlapping input, it restores one voice per held key and waits for the matching final release. Sustain and ARP Hold remain separate from physical key counts. Channel All Notes Off / All Sound Off clear counts; Reset Controllers preserves physical presses. Reset and voice-mode changes start a new phrase and clear input bookkeeping. Counts saturate rather than wrapping.

## Validation

`tests/overlapping_notes.cpp` checks paired releases, sustain, channel isolation, panic, reset, voice-mode changes, voice stealing, ARP operation and ARP-to-bypass transitions in all three voice modes. `tests/mono.cpp` also checks paired repeated notes at 44.1, 48 and 96 kHz.

All 30 standalone test programs passed locally with AddressSanitizer and UndefinedBehaviorSanitizer on 2026-09-10. Plugin builds and host validation are performed separately; these engine tests do not substitute for a DAW test.
