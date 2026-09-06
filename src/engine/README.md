# SAWSTAR engine

Synth owns 16 fixed voices, SAWSTAR SevenSaw oscillators using DaisySP polyBLEP primitives and ADSR envelopes.
It has no GUI or iPlug2 dependency. The plugin adapter schedules MIDI at sample
offsets using a fixed 1024-event buffer; overflow clears voices to prevent stuck
notes. Equal-offset events preserve arrival order. Pending future events retain
their offsets across blocks. Parameter values are sampled once per block; gain
uses a 5 ms one-pole smoothing filter.

Repeated notes on one channel retrigger a voice. Allocation prefers idle voices,
then the oldest released voice, then the oldest held voice. Stealing uses a soft
envelope retrigger but changes frequency immediately; a dedicated steal crossfade
is deferred. CC64 sustain, CC120 all sound off, CC123 all notes off and CC121
sustain reset are handled independently per channel. Pitch bend and modulation
work per MIDI channel. The voice sum retains 1/16 scaling, with a calibrated
Level Boost and stereo peak guard. See docs/SOURCE_MIXER.md for source routing
and old-project gain compatibility.
See docs/SEVEN_SAW.md for unison behavior.

GUI note feedback is a 30 FPS snapshot of held voices, not an event history;
notes shorter than an editor refresh may not flash. Audio triggering still uses
sample offsets. Atomics convey held-note state to the UI; render callbacks never
access graphics or allocate memory.
