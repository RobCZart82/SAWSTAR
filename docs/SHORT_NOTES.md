# Short-note release fix

A note-on and note-off can arrive before the next rendered sample, for example
when quickly dragging across the onscreen keyboard. Previously the poly voice
called ADSR Retrigger but its gate never reached Process(true), so Process(false)
could miss the falling edge and leave the voice sustaining.

Each voice now remembers a pending gate edge. When a note is already released
before its first sample, the amp and filter envelopes observe the true gate
before processing the release. Ordinary held notes follow the unchanged path.
This extends the existing mono safeguard to polyphony and voice reuse.

The SAWSTAR keyboard also sends note-off on mouse exit before the underlying
keyboard clears its touched-key state. No third-party source is modified.

Regression: the old engine failed short_note_release at test commit
7a9282491c7614ecb93d6b8f2ca58e1edf7129fc, reporting
“zero-sample note sequence must release every voice.”
The test covers a 61-note sweep exceeding polyphony, zero/one-sample notes,
normal and velocity-zero note-off, all-notes-off and pedal release, plus held
notes, in Poly/Mono/Legato at 44.1/48/96 kHz.

Manual REAPER verification remains pending: sweep quickly, drag out of the
keyboard, release outside it, and confirm silence after the configured envelope
and effect tails. Repeat with a host MIDI clip containing very short notes.
No preset format, parameter ID, or intended held-note sound changes.

## Validation

Fixed source commit: `60de18e9cb818835aa0b5eeca000bb995677a65b`.
All 19 tests passed in macOS and Windows Debug/Release and VST3 Release jobs.
The VST3 validator passed 47 tests on each platform, with no failures.

- [macOS](https://github.com/RobCZart82/SAWSTAR/actions/runs/34145920890)
- [Windows](https://github.com/RobCZart82/SAWSTAR/actions/runs/34145920886)
