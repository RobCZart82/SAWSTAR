# Plugin adapter

SAWSTAR.h/.cpp connect iPlug2 VST3 to the synth, sample-offset MIDI queue,
arpeggiator, editor MIDI recovery, meters and scope. The callback renders stereo
audio; bypass mutes the output while processing continues.

Parameters.h/.cpp define the stable parameter IDs and sanitization. State.h/.cpp
encode sound snapshots and host GUI scale, with legacy project migration.
EngineControls.h maps the block parameter snapshot to the engine and arpeggiator.

SynthEngine.h selects Classic by default. Premium Preview, Native Candidate and
explicit Premium Production builds select their separate engine implementations;
this is not an automatic change to the default build.

See [the development index](../../docs/DEVELOPMENT_DOCUMENTATION_INDEX.md) for
current implementation, CI evidence and release gates.
