# Pitch bend and modulation wheel

Pitch bend uses all 14 MIDI bits: 0 is full down, 8192 is center, 16383 is full
up. Bend Range defaults to +/-2 semitones and spans 0–24 semitones. A 5 ms
pitch-ratio smoother affects sounding and new voices without resetting oscillator
phase or detune smoothing. Note pitch changes remain immediate.

CC1 is a unipolar modulation wheel. Its first destination is filter cutoff:
wheel/127 * Mod Depth is added to the filter's semitone offset before clamping.
Depth defaults to +24 semitones and spans 0–48. It uses the filter's 16-sample
target updates and 10 ms coefficient smoothing. Raise Filter Mix and lower base
Cutoff to hear it. No vibrato or configurable routing is implemented yet.

Controller values are independent on all 16 MIDI channels. CC121 resets bend,
modulation and sustain on its channel. CC120 stops sound without resetting bend
or mod. Engine reset centers bend and clears mod. RPN bend-range negotiation,
MPE and high-resolution CC1/33 pairing are not implemented.

The fixed bottom strip is shared by MAIN, ADVANCED and PRESETS: compact PITCH
and MOD wheels at left, the flat keyboard alongside, and the status row beneath.
UI wheels send MIDI on channel 1; external channel-1 MIDI updates their display.
Pitch returns to center on mouse release; Mod stays where placed. Double-click
resets either wheel. These are transient MIDI controllers, not preset parameters.
Only Bend Range and Mod Depth are saved (appended IDs 17 and 18); old states
default them to 2 and 24. Current v1 saves hold nineteen records (244 bytes plus
the VST3 wrapper's four-byte bypass). DAWs should record wheel movements as MIDI.

ADVANCED exposes the two depth controls. The shared header preset selector from
the concept remains a later UI milestone. No placeholder CPU percentage is shown.

Tests measure oscillator frequency at center and both bend endpoints, live range
changes, new-note bend, controller reset, channel isolation, filter opening,
extreme controller sweeps and panic silence at 8/44.1/48/96 kHz. State tests
include seventeen-record migration without changing any prior parameter IDs.
