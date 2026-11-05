# DSP

The implemented Classic signal path includes SevenSaw oscillators (DaisySP
polyBLEP primitives plus SAWSTAR unison/waveform handling), the SUB oscillator,
noise sources, SAWSTAR ADSR, filter envelope and multimode filter, two LFOs,
a modulation matrix and stereo effects.

Adsr.h retains the established float response where possible and uses double
state to keep long envelopes advancing. LowPass.h/.cpp implement the Classic
LP12, LP24, HP12 and BP12 path; Premium engine experiments live outside this
directory. Safety.h centralizes finite-value and sample-rate guards.

See [effects](Effects/README.md), [engine](../engine/README.md) and the
[development index](../../docs/DEVELOPMENT_DOCUMENTATION_INDEX.md). DaisySP-backed
tests require the pinned submodule; foundation-only checks do not cover the
complete audio engine.
