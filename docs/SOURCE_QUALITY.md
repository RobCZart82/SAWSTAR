# Source transitions and register measurements

## Noise type switching

White, Dark and Pink now crossfade with a 5 ms exponential time constant, shared
with the existing mixer smoothing. This is not a fixed 5 ms fade: it is mostly
settled in about 25 ms and snaps to its destination near numerical silence.
All three noise generators keep running, preserving deterministic histories.
A new selection during a transition starts from the current weights.
Noise Color remains after the blended source. No parameter IDs, preset format,
source gain or GUI controls change. Reset initializes the weights to the selected
type. Steady type selections retain the original source signal.

The regression compares synchronized noise-only renders for all six directed
White/Dark/Pink transitions at 44.1/48/96 kHz. It bounds the first sample's
additional change against an unchanged reference, verifies convergence to the
new source, and checks that selection still audibly changes the signal. It does
not assert that random noise itself has small adjacent-sample differences.

## Register and level measurements

`source_quality` reports standalone centered single-oscillator peak, RMS, mean
and non-fundamental energy for all four waveforms near 110, 440, 4000 and 8000 Hz
at 44.1/48/96 kHz. Frequencies are aligned to DFT bins to avoid window leakage.
Unison mix is zero and envelopes/filter/effects/output gain are excluded.

The sine test limits non-fundamental energy to 0.1%. For saw, square and triangle,
that energy includes wanted harmonics: it is explicitly not an aliasing score.
All shapes must remain finite, bounded and audible in these cases. This is an
initial high-register check, not proof of alias-free operation or complete
spectral coverage under pitch modulation.

The measurements are a basis for later listening at matched loudness. No automatic
RMS compensation is introduced: that could alter existing patches, oscillator
balance and peak headroom. GUI and Factory preset work remain deferred.

Manual listening in REAPER for this build is pending. In a sustained noise-only
patch, switch White/Dark/Pink at a high but comfortable level and compare the
transition with the previous build. Also audition single and unison notes across
the keyboard with filter/FX off before evaluating the complete sound.

## Initial measurements

The macOS Release run passed all 48 source combinations. At 48 kHz, approximate
RMS levels of the standalone oscillator were:

| Waveform | Near 440 Hz | Near 8 kHz |
|---|---:|---:|
| Saw | 0.570 | 0.443 |
| Square | 0.701 | 0.587 |
| Triangle | 0.573 | 0.591 |
| Sine | 0.707 | 0.707 |

Thus equal mixer positions do not imply equal RMS or perceived loudness.
The source balance has not been silently changed. The observed sine residual
was below the 0.1% threshold at every tested pitch/sample rate. Full values:
[source-quality-macos.csv](source-quality-macos.csv).

Tested source commit: `655d89a310bf0399a6cec1f4a884630cf2fe3ba3`.

- [macOS CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34158827039)
- [Windows CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34158827013)

Windows measurements: [source-quality-windows.csv](source-quality-windows.csv).
Both Release VST3 builds passed 22 foundation tests and all 47 validator checks.

The user subsequently reported that the source-quality build worked correctly
in their trial. This is user-reported host feedback, not additional spectral
measurement or exhaustive automation testing.
