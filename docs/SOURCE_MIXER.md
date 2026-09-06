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

Filter drive/modes are now implemented; see [FILTER_CHARACTER.md](FILTER_CHARACTER.md).
Still planned: other oscillator waveforms, LFO/routing,
mono/legato/glide, arpeggiator, chorus/delay/reverb and final GUI integration.

## Verified build — 2026-09-06

Code commit `d0adf92efd505ec86e71907fdc8d4d111dd52dc0`:
[macOS run](https://github.com/RobCZart82/SAWSTAR/actions/runs/34050146840) and
[Windows run](https://github.com/RobCZart82/SAWSTAR/actions/runs/34050146814).
Both platforms passed all 11 test executables in Debug/Release and all 47
VST3 validator checks. Manual Windows host testing of this build is pending.

REAPER 7.79 on macOS loaded the previous nineteen-parameter Saw Pluck project
with Level Boost 0, extra sources 0 and its original parameter values.
Setting Boost to 18 restored recognition as the newly calibrated Saw Pluck.
The four mixer faders and White/Dark Noise dropdown were exercised.
A saved project was closed and reopened: OSC1 100%, OSC2 35%, SUB 25%, NOISE 8%,
Dark Noise and Level Boost 18 dB were all restored correctly.

An 8.294-second stereo 44.1 kHz/24-bit PCM MIDI render of this mixed patch peaked
at -13.688 dBFS, with zero clipped samples and no post-render normalization.
This is a functional host check, not a full listening/CPU/automation acceptance
matrix or a finished sound-design preset collection.
