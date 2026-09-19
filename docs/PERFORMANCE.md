# Performance controls — 1.0.3

Pitch bend uses all 14 MIDI bits: 0 is full down, 8192 is center, 16383 is full
up. Bend Range defaults to +/-2 semitones and spans 0–24. A 5 ms pitch-ratio
smoother affects sounding and new voices without resetting oscillator phase or
detune smoothing. Note selection and the optional Glide are separate controls.

CC1 always contributes wheel/127 × Wheel Depth to filter cutoff in semitones.
Depth defaults to +24 and spans 0–48. Raise Filter Mix and lower the base cutoff
to hear this built-in routing. The four-route modulation matrix can additionally
route the wheel to other destinations; setting those routes Off does not disable
the built-in cutoff contribution. Set Wheel Depth to zero to disable that part.
Two LFOs also support pitch modulation, cutoff, amplitude and pan destinations.

Controller values are independent on all 16 MIDI channels. CC121 resets bend,
modulation and sustain on its channel, using the existing smooth controller
return. CC120 stops sound without resetting bend or mod. Engine reset centers
bend and clears mod. RPN bend-range negotiation, MPE and high-resolution CC1/33
pairing are not implemented.

## Mono, Legato and live changes

Mono/Legato choose physically held keys before pedal-latched keys, with last-note
priority within that class. Mono retriggers the envelopes for a new key; Legato
retains the envelope on overlap. Glide supports Overlap only and Always modes.
The glide history refers to a pitch that has actually rendered audio.
Release-only MIDI events at the same sample are resolved before rendering.

Retriggered LFOs follow the existing first-key policy. Off→On and On→Off at the
same sample can differ when one order momentarily releases the last held key.
The MIDI queue retains equal-offset arrival order. Free-phase LFOs do not reset
at this boundary. This is covered by `performance_policy`, not silently reordered.

Live voice-mode changes release the old phrase and reset held-key bookkeeping;
new notes use the selected mode. Tests cover lifecycle/release correctness, not
a promise of inaudible mode changes. No experimental RC7 mode crossfade is used.

## GUI and state

The bottom keyboard and wheels are shared by MAIN, ADVANCED and PRESETS. UI
wheels send MIDI on channel 1; incoming channel-1 MIDI updates their display.
Pitch returns to center on release; Mod stays where placed. Double-click resets
the wheel, but the first click can already send the clicked value. This gesture
is not an atomic reset and is a separate future interaction-design item.

Wheel positions are transient MIDI controllers; DAWs should record movements
as MIDI. Bend Range and Wheel Depth are saved parameters, with stable IDs 17/18.
The current v1 state contains 93 records: 1132 bytes (16-byte header + 93 × 12),
plus the optional four-byte VST3 bypass trailer. Older supported states retain
migration defaults. Host parameter automation is currently applied per block;
MIDI sample-offset/block-equivalence tests do not establish sample-accurate
parameter automation.

The header includes the quick preset selector. DSP CPU reports plugin render
time relative to the audio block duration, not whole-computer CPU usage.

## Validation scope

`performance`, `controller_lifecycle`, `glide_history`, `performance_policy`,
`release_order`, `state_roundtrip` and `block_midi_queue` cover the relevant
engine/state contracts. GUI gestures, external hardware and host-specific
behaviour still require actual host acceptance. See
[the release checklist](RELEASE_1.0.3_CHECKLIST.md) for candidate evidence and
remaining checks; historical 0.1-era results are not current acceptance.
