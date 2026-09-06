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
eight-record v1 states remain supported. New saves contain eleven records.
Filter integrator state is not serialized. Idle voice reuse clears integrators;
live retriggers retain them. Filter envelopes and keyboard tracking are deferred.

Tests cover bypass identity, high-frequency attenuation, unity DC response,
channel isolation, extreme cutoff/resonance sweeps, reset/invalid-input recovery,
full 16-voice 7-Saw bounds and panic silence at 8/44.1/48/96 kHz.
