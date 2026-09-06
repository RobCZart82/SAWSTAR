# Resonant stereo low-pass

Signal path per voice: 7-Saw -> stereo 12 dB/oct TPT low-pass -> Amp ADSR.
Left and right channels have independent integrator states. Cutoff, resonance
and wet/dry mix use 10 ms coefficient smoothing. No allocation or locks occur
while rendering, and tangent is calculated only when the cutoff target changes.

- Cutoff: logarithmic 20–20000 Hz, clamped internally to 0.45*sample rate.
- Resonance: 0–100%, maps damping 2–0.1 (Q 0.5–10). Can boost peaks;
  the existing output headroom/clamp remains. No self-oscillation or drive stage.
- Filter Mix: 0–100%, dry/wet blend. Init/old projects use 0% for unchanged sound.

IDs 8/9/10 append to the eight existing IDs. Raw five-double, five-record v1 and
eight-record v1 states remain supported. The initial filter build saved eleven records; current saves contain seventeen.
Filter integrator state is not serialized. Idle voice reuse clears integrators;
live retriggers retain them. Filter envelopes and keyboard tracking are described in FILTER_MODULATION.md.

Tests cover bypass identity, high-frequency attenuation, unity DC response,
channel isolation, extreme cutoff/resonance sweeps, reset/invalid-input recovery,
full 16-voice 7-Saw bounds and panic silence at 8/44.1/48/96 kHz.

## Validation — 2026-09-06

Implementation commit: e0a70262f7369b86e162e50c6a565191b19736c9.
macOS run 34039698926 and Windows run 34039698917: Debug/Release foundation
checks passed; plugin builds passed all six test executables and all 47 VST3
validator tests. Packages are unsigned development builds.

REAPER 7.79 on macOS loaded the previous 7-Saw project with its existing eight
parameters intact and Filter Mix at 0%. A new save contains all eleven records.
A stereo 44.1 kHz/24-bit MIDI render with cutoff 1067.11 Hz, resonance 31.403%
and Filter Mix 100% peaked at -23.65 dBFS with zero clipped samples.
The separate listening preview is normalized to -6 dBFS; plugin gain is unchanged.
Manual Windows host testing of this filter build remains pending.

The filter now also supports LP24, HP12, BP12 and pre-filter Drive.
See [filter character](FILTER_CHARACTER.md); the original LP12 remains default.
