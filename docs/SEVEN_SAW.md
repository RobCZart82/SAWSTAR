# SAWSTAR 7-Saw

An original SAWSTAR unison tuning, mixing and panning layer over seven MIT
DaisySP polyBLEP saw primitives per voice. It does not emulate a specific
hardware synthesizer or reproduce a proprietary detune curve.

- Detune: maximum outer-pair offset in cents, 0–50. Relative offsets are
  0, +/-0.19, +/-0.53 and +/-1 times Detune (symmetric in log frequency).
- Mix: 0–100%. Center weight is 1; each of six side voices has weight Mix/100.
  Per-channel normalization is 1/(1+6*Mix/100), for bounded peak headroom.
  This is not loudness compensation: perceived/RMS level varies with detune.
- Width: 0–100%. Symmetric linear pan pairs; 0 gives exact mono. The center
  stays centered. Mono fold-down retains all seven oscillators.

Init: Detune 20 cents, Mix 0%, Width 75%. Raise Mix (try 70%) to hear 7-Saw.
Mix defaults to zero so older projects retain their single-saw sound. Center
output is regression-tested against the previous DaisySP oscillator. The side
oscillators run even at Mix zero to retain continuous phases during automation.
Detune ratios, Mix and Width have 10 ms one-pole smoothing. MIDI pitch changes
remain immediate. Fixed side starting phases avoid all seven starting together;
phases run while a voice is active and are not reset on every note-on.

16 voices = up to 112 saws. No dynamic allocation, locks, trigonometric calls or
exponentiation in the per-sample oscillator loop. Tuning ratios are recomputed
only when Detune changes. Frequencies are capped below Nyquist (0.45*sample rate).
The existing 1/16 synth headroom and 5 ms output gain smoothing remain.

State compatibility: new IDs 5/6/7 append to the original 0–4. Both old raw
five-double states and five-record v1 states populate new controls with the
Init defaults. Eight-record v1 saves preserve all controls. The original raw
legacy reader now explicitly reads only five doubles, even as new IDs are added.

Tests cover single-saw compatibility, old state migration, exact mono at Width
zero, stereo/mono-fold energy, extreme pitches, 44.1/48/96 kHz, 112-saw output
bounds and panic silence. PolyBLEP reduces aliasing but is not a brick-wall
bandlimited oscillator; no oversampling or full spectral certification is claimed.
