# Arpeggiator

ADVANCED → ARP contains the complete initial arp controls:

- On/Off (default Off).
- Up, Down, Up/Down (without repeating end notes), Random, Played order.
- Rate: 1/4, 1/8, 1/16, 1/32, eighth/sixteenth triplets and dotted values.
- Gate 5–100%, Octaves 1–4, Swing 0–75%, Hold On/Off.

First held key starts a phrase immediately. Rate follows host BPM at sample
resolution, independently of audio block size. Tempo sync is key-triggered, not
absolute host beat-position alignment. The live keyboard also works with stopped
transport. A running-to-stopped transport transition clears the arp phrase;
a new key starts a new phrase. Random uses a repeatable local PRNG.

Swing lengthens the first and shortens the second step while preserving the
pair duration. Gate is a fraction of that individual swung step. Rate/octave/mode
changes take effect without resetting the host; a rate change sets the next
step's duration. Invalid/out-of-range octave notes above MIDI 127 are omitted.
Input channels and per-key velocities are preserved. Played mode follows key
press order within each octave. Generated notes drive SAWSTAR internally.

Hold keeps the chord after releasing keys. Once all physical keys are up,
pressing a new key starts a replacement chord (additional held keys join it).
Turning Hold off removes latched notes, except those held by sustain pedal.
The pedal is handled by the arp so it cannot accidentally sustain every generated
step in the synth. Controller reset/all-notes-off/all-sound-off clear the
appropriate channel's arp state. Turning arp off releases its step and restores
still-physically-held keys for normal playing; latched keys are discarded.

Preset load, factory preset selection, buffer overflow recovery, transport stop
and audio reset clear generated/latched notes. Envelope/effect release tails may
continue normally. No new parameters are enabled in older presets.

New stable IDs 83..89: arp.enabled, arp.mode, arp.rate, arp.gate, arp.octaves,
arp.swing, arp.hold. Defaults: Off, Up, 1/16, 80%, 1 octave, 0%, Off.
The existing parameter identities and ranges are unchanged.

Implementation uses fixed key/note storage with no allocation, locks or external
code in the audio path. Sorting occurs only when the held chord/config changes.
This is original SAWSTAR MIT code. The GUI concept artwork is unchanged; a
programmable per-step sequencer is not part of these arp controls.

## Manual host check

In REAPER, select a short pluck preset, enable ARP and hold a C/E/G chord.
Compare all five modes, octaves, straight/triplet/dotted rates and swing. Adjust
Gate, then test Hold with replacement chords. Confirm silence after release
and envelope tails, switching ARP off, stopping playback and loading presets.
Save/reopen the project and a named preset, checking all seven arp settings.
This host check is pending for the new arp build.

## Automated validation

Source commit `7eee5bdc9a853bccf602dfbacf1f3e5599e56be4` passed macOS and
Windows Debug/Release foundation checks and both VST3 builds. Each VST3 validator
reported 47 passed, 0 failed. The arpeggiator tests cover all five modes, gate,
swing, hold, sustain, stop/reset, block-size independence and synth voice modes.

- [macOS run](https://github.com/RobCZart82/SAWSTAR/actions/runs/34155445278)
- [Windows run](https://github.com/RobCZart82/SAWSTAR/actions/runs/34155445269)
