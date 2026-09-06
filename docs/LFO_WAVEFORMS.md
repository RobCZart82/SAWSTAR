# LFO routing and oscillator waveforms

MAIN has two independent clickable source-shape previews. Click OSC1 or OSC2
to choose Saw, Square, Triangle or Sine. The graph is a schematic one-cycle
shape, not a live scope of the detuned, mixed and filtered audio. Saw remains
the default; the existing unison mix, detune, width and octave controls work
for all four shapes. SUB remains a separate sine oscillator.

Saw/Square use the pinned DaisySP polyBLEP implementations; Sine uses
its sine oscillator. Triangle uses SAWSTAR-owned leaky integration of the
polyBLEP square, following the same approximation,
so its exact shape/level varies with frequency. Shapes have their native levels,
not loudness normalization. Selection crossfades over a 10 ms one-pole response.
Inactive shape banks stop processing after their crossfade weight decays;
returning to one resumes its saved phase. No allocation occurs during playback.

## ADVANCED: one global LFO

- Shape: Sine, Triangle, Ramp or Square. A 2 ms output smoothing stage rounds
  sharp transitions; it is intended for musical low-frequency modulation.
- Rate: 0.05–20 Hz in Free Hz mode.
- Tempo Sync: follows DAW BPM with 1/1, 1/2, 1/4, 1/8, 1/16 or 1/32 cycles.
  1/1 means four quarter-note beats, independent of time signature. Division
  is used only in Tempo Sync; Rate only in Free Hz. No dotted/triplet values yet.
- Amount: 0–100%, default 0 (off). Rate, depth and routing transitions are smoothed.
- One selected destination: Filter Cutoff (up to ±24 semitones), Pitch (up to
  ±1 semitone for OSC1/OSC2/SUB), Amp Level (attenuation tremolo, up to full
  depth), or Pan (linked stereo balance with square-root gains).
- Phase: Free phase or Retrigger first key. Retrigger resets phase when a new
  note arrives with no keys held; legato notes do not reset it. Release tails
  can still be sounding when a new phrase resets the global LFO.

Tempo Sync is BPM synchronization, not alignment to the DAW playhead/bar.
The LFO advances whenever the host processes audio, including silence; an
engine reset resets phase. Missing/invalid BPM falls back to 120.
There is one shared LFO per plugin, not an independent phase per voice.

Cutoff modulation combines with Filter ADSR, key tracking and CC1 before cutoff
clamping. Raise Filter Mix above zero to hear it. Pitch combines with pitch bend
and octave transposition. Amp/Pan act on the voice sum before output protection.
This is the first modulation route, not a general multi-slot modulation matrix;
aftertouch, macros and additional LFOs remain future work.

## State and validation

IDs 33–41 append OSC1/OSC2 waveform, LFO Rate, Amount, Shape, Target, Sync,
Division and Retrigger. Older states default to Saw/Saw and Amount 0. The v1
state now contains 42 records: 504-byte payload / 520 bytes total before the
framework's separate bypass field. Live phase is deliberately not serialized.

Tests cover every source shape and selection transitions, LFO rate/tempo phase,
retrigger behavior, modulation ranges, all four audible routes, OSC independence,
zero-depth identity and prior-state migration at 44.1/48/96 kHz.

The pinned DaisySP triangle Init/constructor do not initialize integrator
history. SAWSTAR therefore owns and explicitly initializes that history, using
the polyBLEP square primitive as input instead of the library triangle mode.
This is covered by Windows Debug/Release waveform-switching tests.
