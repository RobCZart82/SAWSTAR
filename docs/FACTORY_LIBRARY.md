# Embedded Factory library

The library contains 24 sounds, compiled into the VST3. No separate bank is
required. Existing factory keys and all previous positions are retained;
user files and already saved project snapshots are not rewritten.

## Templates — seven starting points

Init, Pad-Init, Lead-Init, Pluck-Init, Bass-Init, Keys-Init and Arp-Init. Templates have no active
chorus/delay/reverb, arpeggiator or LFO depth. Their envelopes, filter and voice
mode provide useful starting shapes. Initialize still loads neutral Init.

## Finished sounds — seventeen

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
| Solid Saw Lead | Steady dual saw, modest master width and dotted delay |
| Velvet Pad | Triangle/saw, quiet dark noise, slow filter and reverb |
| Trance Pluck | Dual saw with octave layer, zero sustain and dotted echoes |
| Round Sub | Mono sine sub and quiet triangle, no stereo effects |
| Octave Steps | Square/saw up/down arp with slight swing; HOLD off |
| Air Keys | Triangle, octave sine, subtle pink air and short reverb |

Selection loads a complete 93-parameter snapshot. HOW IT WORKS therefore uses
the same saved values as the sound. Short factory tags appear beneath the category and are searchable with the name
and category (for example `Trance`, `Mono` or `Dry`). Arp-Init is intentionally
a dry rhythmic starting shape with ARP off; enable it to hear the pattern.
Factory lessons describe the intended
signal path; user sounds retain individual `.sawstar` save/import behavior.

Automated factory tests now apply the shared full engine rig, including effects,
LFO2, modulation, SUB, WIDE and ARP. They check valid values, unique identity,
state roundtrip, sound output and note release at 44.1/48/96 kHz. Templates are
checked for dry/unmodulated defaults. Final subjective level/timbre approval
remains a listening task; this is a release-candidate sound library, not a claim of
equal perceived loudness for every MIDI phrase.

Verification and host follow-up: [TEMPLATES_HOST_QA.md](TEMPLATES_HOST_QA.md).

## Level pass (2026-09-11)

Output trims on quieter existing sounds were adjusted against SuperSaw One,
without forcing short decaying sounds to match sustained pads in average energy.
The original keys/order remain stable. Loading an updated factory sound uses the
new trim; existing user files and project snapshots retain their saved values.
No DSP, state schema or parameter IDs were changed.

The following deterministic measurement uses 48 kHz, velocity 110, six seconds
of held notes, tempo 137 BPM. Poly sounds use C3/G3/C4/E4; mono leads use C4;
basses use C2. Values are stereo sample peak and stereo mean-square RMS in dBFS,
not LUFS or a claim of equal perceived loudness. Plucks decay during this window,
so their RMS is naturally lower. Listening acceptance is still required.

| Key | Peak dBFS | Six-second RMS dBFS |
| --- | ---: | ---: |
| init | -8.18 | -20.22 |
| wide_lead | -9.83 | -23.54 |
| soft_pad | -11.19 | -23.90 |
| saw_pluck | -10.52 | -37.23 |
| deep_bass | -14.02 | -23.71 |
| bright_keys | -9.59 | -29.42 |
| supersaw_one | -9.25 | -23.98 |
| soft_pad_one | -12.25 | -24.75 |
| pad_init | -9.11 | -19.43 |
| lead_init | -8.34 | -18.80 |
| pluck_init | -8.72 | -34.42 |
| bass_init | -13.98 | -21.66 |
| silk_lead | -16.07 | -23.31 |
| warm_horizon | -12.40 | -24.92 |
| glass_pluck | -9.33 | -34.90 |
| simple_steps | -14.80 | -28.50 |
| keys_init | -8.95 | -27.86 |
| arp_init | -8.96 | -33.87 |
| solid_saw | -10.04 | -24.34 |
| velvet_pad | -11.39 | -24.20 |
| trance_pluck | -9.72 | -36.18 |
| round_sub | -17.11 | -23.46 |
| octave_steps | -16.49 | -25.80 |
| air_keys | -9.32 | -29.08 |

The `factory_presets` test repeats this phrase check, checks headroom, and also
checks each sound at 44.1/48/96 kHz for valid state, output and voice release.
No WAV deliverables are generated.

## Verification of this library build

Candidate `900fe9800ee07f669a2072f3b7fe9cb8a3b89417` passed the local library
and factory tests. Both macOS and Windows VST3 Release builds passed all 34
foundation tests and all 47 VST3 validator checks. Downloaded package digests
and ZIP integrity were verified; macOS was installed with a prior-version backup.

REAPER 7.79/macOS showed seven Templates and loaded Keys-Init. Searching All
for `Trance` returned SuperSaw One, Solid Saw Lead and Trance Pluck. Solid Saw
Lead loaded with matching header, parameter values, tags and saved diagrams;
the text did not overlap HOW IT WORKS. The separate test project was closed
without saving. This is GUI/load validation, not final musical listening approval.

- [macOS build](https://github.com/RobCZart82/SAWSTAR/actions/runs/34569574994)
- [Windows build](https://github.com/RobCZart82/SAWSTAR/actions/runs/34569574984)
