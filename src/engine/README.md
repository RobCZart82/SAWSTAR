# SAWSTAR engine

Synth owns 16 fixed voices with SevenSaw oscillators, SUB/noise, SAWSTAR ADSR
and filter envelopes. It has no GUI or iPlug2 dependency. The plugin schedules
MIDI at sample offsets with a fixed 1024-event queue; overflow invokes panic and
controller recovery. Equal-offset events preserve arrival order, and future
events retain their relative offsets across blocks.

Repeated same-channel pitches retrigger one voice while retaining press counts.
Allocation prefers idle voices, then the oldest released voice, then the oldest
held voice. Reused poly voices have a 3 ms output-correction ramp. Mono/Legato,
last-note priority and glide have their own key ownership and transition logic.

CC64, CC120, CC123 and CC121 are handled per channel. Bend, mod wheel and
channel pressure feed performance/modulation controls. The voice sum retains
1/16 scaling with combined Volume/Boost smoothing and a linked stereo peak guard.
Chorus, Delay, Reverb and Width follow the summed voice path.

GUI note feedback snapshots physically held MIDI pitches across channels, not
the voice allocator's sounding/release state. Very short notes may not appear
between editor refreshes. Rendering never accesses graphics. Delay/reverb storage
is prepared by Reset/Init; sample processing does not allocate.

See [source routing](../../docs/SOURCE_MIXER.md),
[unison behavior](../../docs/SEVEN_SAW.md) and the
[current development index](../../docs/DEVELOPMENT_DOCUMENTATION_INDEX.md).
