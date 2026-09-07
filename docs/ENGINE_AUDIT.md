# Engine audit without a DAW

Scope: direct engine tests and VST3 validator on macOS and Windows, without
REAPER, audio-device playback or separately exported WAV files. Existing GUI
and factory presets are unchanged. Test scenes are not new factory presets.

## Combined coverage

The new `engine_audit` test uses a full 90-parameter snapshot and applies its
engine settings following the plugin wrapper's current parameter mapping.
It exercises three synthetic configurations at 44.1, 48 and 96 kHz:

- Low-register dual-unison lead: 16 notes, sub, Pink Noise, driven filter.
- Mid-register pad: 16 notes, 2-second Amp Release, 12 seconds after note release.
- Higher-register arpeggiated pluck: random order, three octaves, swing and Hold.

Every scene uses both LFOs, four modulation routes, chorus, delay and reverb.
Wheel, channel pressure and pitch-bend events arrive while notes sound.
Stereo peak, RMS, mono-fold RMS, signal mean and elapsed test render time are
reported to CI logs. Stereo/mono signals must be audible, finite and bounded by
the existing 0.98 sample-peak guard. Arp Clear and envelope release must leave
zero active voices; effect tails are allowed and are not falsely classified as
stuck notes. This does not assert complete silence while effect tails remain.

A second fresh engine loads the encoded/decoded snapshot and receives identical
MIDI. Its settings are reapplied every 127 samples while the original is configured
once. The two renders must be sample-identical. This tests parameter snapshot
recall and repeated block parameter delivery; it does not serialize running DSP
phase, delay buffers or an entire DAW project, nor simulate host automation timing.

## Existing checks retained

The suite also checks oscillator source levels and the sine's high-register
residual, noise spectra/color/transitions, output protection, filter behavior,
mono/legato/glide, short notes, voice replacement, parameter transitions, both
LFOs and the matrix, arp order/timing/Hold, all three effects, parameter contracts
and state migration. Release CI runs the existing comparative CPU workload.
Its elapsed-time measurements depend on the CI machine; they are not REAPER CPU
percentages. The combined audit render time includes two engines and assertions.

## Limits

Passing these cases is not proof of no bugs, no aliasing, equal perceived
loudness, or glitch-free audio-device scheduling. The stereo-to-mono test checks
that sound remains, not a perceptual stereo-image judgment. A listening review
and host-specific project/automation checks remain separate. No blanket engine
freeze or automatic source-gain normalization follows from this audit.

## Results

Tested source: `3945d5ba8c96728f2c80dc9a1ad769aaddeb6334`.
Both Release runs passed all 25 tests and both VST3 validators passed all 47
checks. All nine combined scenes passed on each platform. No engine defect was
found by these checks, so this change adds coverage rather than changing DSP.

Full-snapshot recall and repeated parameter delivery were sample-identical within
each platform. Cross-platform bit identity is not asserted. The pad reaches 0.98
sample peak, exercising protection; the limit is never exceeded. Every scene
remains audible in mono and reaches zero active voices after clearing/release.
Differences between scene RMS values reflect their different envelopes, notes
and arpeggiation and are not automatically classified as gain defects.

- [macOS measurements](engine-audit-macos.csv)
- [Windows measurements](engine-audit-windows.csv)
- [macOS CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34161377187)
- [Windows CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34161377252)

The existing single-engine 48 kHz / 16-voice full-FX workload consumed a median
18.795% of rendered duration on the macOS runner and 33.763% on the Windows
runner (three measurements each). These include measurement overhead and cannot
be interpreted as the user's device CPU meter or worst-case callback latency.
The workload retains the original baseline comparison, not a comparison against
the immediately preceding build; this audit makes no new optimization claim.
