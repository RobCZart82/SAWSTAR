# Polyphonic voice reuse

A playing polyphonic voice previously switched immediately to the next note's
velocity, pitch and envelope state when reused (including same-note retriggers).
The first new output sample could jump abruptly, especially with a large velocity
change or the 17th held note.

Reused voices now join their previous rendered stereo sample through a 3 ms
linear correction ramp. New synthesis starts immediately; there is no MIDI delay
and no extra voice pool. Repeated reuse starts from the already corrected output.
The envelope and filter still run normally. Release waits for any remaining
correction to finish; All Sound Off still silences the voice immediately.

This is a short discontinuity correction, not a complete old/new oscillator
crossfade and not a promise of click-free audio under every modulation setting.
It can briefly add low-frequency energy; the existing output guard remains active.
Idle-voice attacks and ordinary sustained notes are unchanged. Mono/Legato uses
its existing glide and velocity smoothing; this change targets polyphonic reuse.
No parameter, preset format, GUI or third-party dependency changes.

Tests cover same-note velocity retrigger, full 16-voice replacement, dense reuse,
bounded output, complete release and panic during a correction at 44.1/48/96 kHz.
The earlier short-note regression remains enabled to check zero-length MIDI notes.

Manual REAPER listening for this build is pending: play repeated notes with
alternating velocities and dense chords beyond 16 held notes. Compare attacks
and the absence of stuck voices. Filter/oscillator automation still needs its own
broader transient review; this pass does not finalize all parameter transitions.

## Validation

Source: `6e807ea02ba6aa780ad47122da50eb0850f1370e`.

- [macOS CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34159722765)
- [Windows CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34159722738)

Both Release foundation runs passed all 23 tests, including the new transition
regression and the existing zero-length note, modulation and output checks.
Both VST3 validator runs passed all 47 checks.
