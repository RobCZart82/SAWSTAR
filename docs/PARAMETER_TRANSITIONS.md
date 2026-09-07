# Live parameter transitions

Amp Sustain now has a 5 ms exponential smoothing time constant while voices are
active. With a very short Decay, a sudden sustain target previously propagated
too quickly through the envelope. The extra smoothing reduces the initial level
step during live edits. This is mostly settled in about 25 ms, not a fixed 5 ms
fade. Idle parameter setup takes the target immediately, preserving the first
note's programmed envelope. Reset adopts the retained target; release processing
still uses the existing ADSR release stage.

There are no new controls, parameter IDs, preset records, dependencies or GUI
changes. Static sustain settings and idle patch preparation are unchanged;
active sustain automation deliberately has a slightly slower response.

The new regression compares synchronized renders with/without a parameter edit
at 44.1/48/96 kHz for Amp Sustain, output level, mixer level, waveform and filter
engagement. It checks the first sample's additional difference against the
unchanged reference, an audible eventual change, and finite/bounded output.
It also rapidly retargets envelope/level settings and checks complete release.
These are representative cases, not exhaustive click-free guarantees for every
parameter, preset, host automation stream or sample-rate setting. The existing
noise and voice-transition regressions remain enabled.

Manual REAPER check is pending: sustain a note with Decay at 1 ms, then move Amp
Sustain quickly between zero and full. Compare with the previous build, also
checking normal note attacks and release. Existing mixer/filter/waveform tests
do not change those controls' established smoothing behavior.

## Validation

Source: `0e0cd3f52e61e83a2aad34170532e49c6ddee43c`.
Both Release foundation runs passed all 24 tests.

- [macOS CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34160337988)
- [Windows CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34160337916)
Both VST3 validator runs passed all 47 checks.
