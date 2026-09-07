# Architecture

## Dependency direction

```text
Host MIDI / automation / state
             |
       src/plugin (iPlug2 adapter) <---- src/gui (parameter gestures)
             |
       src/engine (SAWSTAR synth + voice manager)
             |
       src/dsp (narrow DaisySP adapters, later custom 7-Saw)

src/midi -> timestamped events -> engine
src/presets -> validated parameter snapshot -> plugin -> engine
```

The foundation currently compiles parameter utilities in `src/plugin` and
page metadata in `src/gui`. The optional DaisySP target is an integration
check, not a playable synth. Other folders are reserved responsibilities.

## Ownership and interfaces for First Sound

- **Plugin adapter:** iPlug2 lifecycle, audio buses, MIDI sample offsets,
  stable host parameter indices and translation to engine units. Zero inputs,
  two outputs; MIDI in; no MIDI out or MPE in v0.1.
- **Engine:** owns a fixed pool of 16 voices, event dispatch, voice summing,
  gain smoothing and lifecycle reset. No dependency on graphics or host APIs.
- **Voice manager:** idle voice first, then oldest released voice, then oldest
  held voice. Repeated note-ons allocate separate voices; note-off releases
  the oldest still-held voice matching channel and note. Add a short steal
  fade/crossfade so reassignment does not produce a discontinuity.
- **Voice:** oscillator phase, amplitude envelope, velocity, gate, note/channel
  and allocation age. Gate release must finish before returning to the pool.
- **DSP:** sample-rate-aware wrappers around DaisySP oscillator and ADSR.
  Use the bandlimited `WAVE_POLYBLEP_SAW`, not the naive saw. Envelope units in
  the engine are seconds; host display/state uses milliseconds.
- **MIDI:** bounded event queue, sample offsets relative to each block;
  velocity-zero note-on is note-off. Process all events at the correct sample.
  CC123 releases gates; CC120 silences immediately. Sustain pedal and pitch
  bend behaviour must be implemented and tested before advertising support.
- **Presets:** versioned state codec and later library/learning metadata.
  Keep files, parsing and memory allocation outside audio processing.
- **GUI:** parameter gestures and display only. The selected page is editor
  state, not a sound parameter; changing a page must not change the sound.

## Real-time rules

No heap allocation, locks, file/network I/O or logging on the audio thread.
Prepare voice storage and event capacity before playback. Define overflow
handling: never drop a note-off silently; request a deterministic all-notes-off
fallback when the bounded MIDI queue overflows. Clamp and reject invalid
parameter/sample-rate inputs outside per-sample work. Smooth output gain;
add further smoothing where discontinuities become audible.

Render independent of block size. On reset/sample-rate changes, clear active
voices and reinitialize DSP. Sum with conservative headroom (initially 1/16
per voice plus master gain), measure peaks, and do not hide clipping with an
undocumented limiter. No shared mutable globals between plugin instances.

## Identity before the first distributed binary

Proposed manufacturer display name: `RobCZart82`; plugin four-character ID:
`SwSt`; manufacturer ID: `RC82`; reverse-domain identity:
`io.github.robczart82.sawstar`.
These identifiers are now compiled into the development shell. Preserve them
when connecting the engine; changing them can break host projects.
Do not copy example plugin identifiers or invent vendor contact addresses.

The source mixer now feeds two independent SevenSaw layers, a sine sub and
per-voice noise into the filter. Output calibration and the documented stereo
peak guard are described in [SOURCE_MIXER.md](SOURCE_MIXER.md).

### Noise extension

White retains the original per-voice xorshift stream. Dark retains its original
1.2 kHz one-pole filter. Pink uses an independent seeded generator with 16
staggered octave-rate random rows and a full-rate component. All are centered
mono voice sources, routed through the existing voice filter and Amp ADSR.
The independent color filter is sample-rate aware, with exact neutral bypass.
No new dependency or external source code is introduced (SAWSTAR MIT code).
