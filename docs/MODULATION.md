# LFO2 and four modulation routes

ADVANCED has an LFO 1 / LFO 2 selector. It selects the controls being edited;
both LFOs run independently. Each has rate, amount, shape, destination, free/tempo
mode, beat division and first-key retrigger. Zero Amount disables its direct
route. Tempo sync follows BPM rather than host timeline position.

In the current development UI, open MODULATION beside CHORUS / DELAY / REVERB
on ADVANCED. Four rows each contain source, destination and a signed amount.
The final concept will use four compact knobs in its existing MODULATION panel;
the concept image itself has not been regenerated in this change.

Sources: Off, LFO 1, LFO 2, Mod Wheel, Velocity, Aftertouch (MIDI channel pressure).
LFO sources are bipolar; wheel/velocity/pressure range from zero to one. Velocity
is per voice and pressure/wheel follow each voice's MIDI channel. Polyphonic
key pressure and MPE expression are not implemented in this milestone.

Targets at 100% route amount:

| Target | Range per row |
| --- | --- |
| Filter Cutoff | 48 semitones |
| Pitch | 12 semitones |
| Amp Level | Add up to 1 to the amplitude multiplier |
| Pan | Full left/right displacement |
| Noise Color | 100 percentage points |

Negative amounts reverse direction. Routes add and are bounded; four pitch
routes are limited to +/-48 semitones and cutoff to +/-96 before filter limits.
Amp modulation is bounded to a 0..2 multiplier; pan and color to their valid
ranges. Source/destination changes crossfade the old and new route weights over
about 10 ms. Output peak protection remains in place.

LFO matrix sources use the waveform before the LFO panel's direct Amount:
use each matrix row's Amount to set its depth. This permits multiple targets
without also activating the panel's direct destination. Leaving both enabled
adds their effects. The existing Mod Depth wheel-to-cutoff control also remains
active for old presets; set it to zero if using only matrix wheel routing.

## Compatibility and real-time behavior

Existing parameter IDs 0..63 and their normalized ranges are unchanged.
New IDs 64..70 are LFO2; 71..82 are the four source/target/amount triples.
Default LFO2 Amount is zero and every matrix row is Off with zero amount.
The version-1 state parser now permits up to 256 records while retaining size,
finite-value and duplicate-ID validation. Older 64-record noise presets migrate
with the new defaults. New state roundtrips retain both LFOs and all rows.

Fixed storage only, no allocations or locks in processing. Inactive matrix
paths bypass evaluation; settled weights are cached. This is original SAWSTAR
MIT code with no new third-party dependencies.

## Suggested manual REAPER check

- Confirm a previously saved SuperSaw One / Soft Pad One sounds unchanged.
- Switch LFO1/LFO2 editors and confirm independent values remain set.
- Try LFO2 at 0.3 Hz with a small cutoff Amount and compare with Amount zero.
- Set LFO2 direct Amount to zero, then route LFO2 to Pan in the matrix.
- Test a negative velocity-to-cutoff amount and channel aftertouch to pitch.
- Save a named preset and a DAW project, restart, and verify all four rows.
- Automate source, target and amount, and repeat the short-note keyboard sweep.

Manual host/UI checks are pending for this build.

## Automated validation — 2026-09-07

Tested commit: `08dc556488e6f9b77e9794f61156cd5c2a3bfd46`.
All 20 tests passed in macOS/Windows Debug and Release and VST3 Release jobs.
The VST3 validator passed 47 tests with no failures on both platforms.
Tests include all 25 source/target combinations, all four direct LFO2 targets,
negative amounts, disabled-audio equivalence, MIDI channel isolation, bounded
combined modulation, note release, and legacy/new preset state roundtrips.
The Windows modulation test uses heap-owned synth fixtures to avoid overflowing
the test process stack; the audio implementation still allocates nothing while
rendering.

- [macOS run](https://github.com/RobCZart82/SAWSTAR/actions/runs/34150156772)
- [Windows run](https://github.com/RobCZart82/SAWSTAR/actions/runs/34150156815)

Manual REAPER listening, GUI switching and user-preset reload remain pending.
