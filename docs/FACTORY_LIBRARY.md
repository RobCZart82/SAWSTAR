# Embedded Factory library

The library contains 16 sounds, compiled into the VST3. No separate bank is
required. Existing factory keys and their first eight positions are retained;
user files and already saved project snapshots are not rewritten.

## Templates — five starting points

Init, Pad-Init, Lead-Init, Pluck-Init and Bass-Init. Templates have no active
chorus/delay/reverb, arpeggiator or LFO depth. Their envelopes, filter and voice
mode provide useful starting shapes. Initialize still loads neutral Init.

## Finished sounds — eleven

| Sound | Intent |
| --- | --- |
| Wide Saw Lead | Broad lead with light chorus and tempo delay |
| Soft Pad | Slow ensemble with restrained reverb |
| Saw Pluck | Filter transient with tempo delay |
| Deep Bass | Centered mono saw, sine sub and gentle drive |
| Bright Keys | Falling envelope with short reverb |
| SuperSaw One | Two ensembles, sub, light chorus, delay and reverb |
| Soft Pad One | Saw/triangle, pink air, slow filter, chorus and reverb |
| Silk Lead | Mono legato triangle/saw with subtle glide and delay |
| Warm Horizon | Steady two-oscillator pad with sub and space |
| Glass Pluck | Triangle and octave sine, short envelope and echoes |
| Simple Steps | Two-octave ascending arp; HOLD defaults off |

Selection loads a complete 93-parameter snapshot. HOW IT WORKS therefore uses
the same saved values as the sound. Factory lessons describe the intended
signal path; user sounds retain individual `.sawstar` save/import behavior.

Automated factory tests now apply the shared full engine rig, including effects,
LFO2, modulation, SUB, WIDE and ARP. They check valid values, unique identity,
state roundtrip, sound output and note release at 44.1/48/96 kHz. Templates are
checked for dry/unmodulated defaults. Final subjective level/timbre approval
remains a listening task; these are an initial curated library, not a claim of
equal perceived loudness for every MIDI phrase.
