# Source mixer and output calibration

The sound path is now **OSC1 + OSC2 + SUB + NOISE → per-voice low-pass →
Amp ADSR/velocity → voice sum → Output/Level Boost → stereo peak guard**.

OSC1 and OSC2 each have an independent seven-saw layer: octave, detune, unison
mix and stereo width. Unison at zero is one saw. SUB is a mono sine, selectable
at -2/-1/0 octaves relative to the played MIDI note, independently of OSC octaves.
All pitched sources follow MIDI pitch bend. NOISE is generated independently
per voice and follows the same filter and amp envelopes, including release.

The four 0–100% mixer faders have 5 ms smoothing. New defaults are OSC1 100%,
OSC2/SUB/NOISE 0%, preserving the original source combination. The selector
below the shortened faders chooses White Noise or Dark Noise. Dark Noise is
white noise through a 1.2 kHz one-pole low-pass, not a pink-noise simulation;
its energy is lower. The NOISE fader controls its level, independently of type.
The main page remains a functional development layout; final concept styling
and oscillator waveform selectors are still pending.

## Output

The original Output parameter remains -60…0 dB with a -12 dB default, so
existing host automation keeps its scale. New append-only parameter 19,
Level Boost, adds 0…24 dB, default 18 dB, after the historical 1/16 voice sum.
This raises an unfiltered full-velocity single saw to approximately -6 dBFS
peak at Output 0 dB, or -18 dBFS at the default Output -12 dB, after settling
and at full amp sustain. Presets, velocity and filters affect actual levels.

A zero-latency, stereo-linked peak guard uses instant attack and 80 ms recovery,
with a 0.98 sample-peak ceiling (about -0.18 dBFS). It reduces both channels by
the same factor. It is not a true-peak limiter or a mastering effect; sustained
overload can audibly compress/distort. Lower source faders/Output in that case.

Older states without Level Boost decode to 0 dB boost and retain their original
volume. Their additional sources default to silent. To raise an old project,
turn Level Boost up yourself, or load a newly calibrated factory preset.
Old factory snapshots can display Custom because their boost differs from the
new factory version. Loading a current factory sound intentionally changes gain.

## Validation scope

Automated tests cover source isolation, octave pitch, noise release and dark
noise attenuation, mixer mute, default source equivalence, coherent 16-voice
peak stress, stereo unison/resonance stress, boost automation, old-state
migration and current-state roundtrips at 44.1/48/96 kHz.

Still planned: other oscillator waveforms, filter drive/modes, LFO/routing,
mono/legato/glide, arpeggiator, chorus/delay/reverb and final GUI integration.
