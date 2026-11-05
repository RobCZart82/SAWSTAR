# MIDI and arpeggiator

BlockMidiQueue.h holds up to 1024 timestamped events, preserves equal-offset
arrival order and carries future events across blocks. Overflow invokes recovery
and retains selected final controller updates.

Arpeggiator.h implements Up, Down, Up/Down, Random and Played patterns, gate,
octaves, swing, hold and sustain-pedal handling. It follows host BPM, not timeline
phase. Transport stop clears ARP key ownership. Bypass forwards MIDI and retains
physical key counts; enabling rebuilds the pattern from those keys. No pattern
construction or sorting is needed while bypassed.

EditorMidiTracker.h owns editor-note overflow recovery separately from host
notes. EditorWheelMailbox.h preserves the latest GUI bend/CC1 return outside the
finite editor FIFO. These classes are audio-thread owned except the explicitly
atomic UI-to-audio mailbox publication.
