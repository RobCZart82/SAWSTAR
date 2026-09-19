# Architecture — 1.0.3

## Responsibilities

- `src/plugin`: iPlug2 VST3 lifecycle, host parameters/state, output buses and
  sample-offset MIDI. `ApplyEngineControls` is shared with the engine tests.
  Zero audio inputs, stereo output with host-output adaptation; no MIDI output.
- `src/engine`: fixed 16-voice pool, Mono/Legato selection, controllers,
  modulation, voice rendering, effects, output gain and peak protection.
- `src/dsp`: oscillator/envelope adapters and SAWSTAR filter/effect code.
  DaisySP dependencies are pinned; the production engine is tested directly.
- `src/midi`: arpeggiator and fixed-capacity 1024-event `BlockMidiQueue`.
- `src/presets`: factory learning metadata and user-preset/favorites file operations.
- `src/gui`: controls and telemetry. Page changes do not change sound parameters.

## Voice and signal lifecycle

Poly note-on first finds a voice matching note and channel and retriggers it.
Otherwise allocation prefers idle voices, then the oldest released voice, then
the oldest held voice. Repeated note-ons do not automatically allocate separate
voices. Mono/Legato use physically held-key priority followed by last-note
priority, with sustain-latched keys as fallback. See [PERFORMANCE.md](PERFORMANCE.md).

Each voice mixes OSC1 and OSC2 SevenSaw layers, selectable SUB and noise, then
passes the mixture through Drive/DC/filter and the amplitude envelope. The
modulation matrix and filter envelope control voice processing. Global LFO
amplitude/pan processing precedes the PreFX telemetry point. Chorus, delay and
reverb precede output width/gain and the documented stereo peak protection.
Output Volume × Boost is smoothed as a combined gain. See
[SOURCE_MIXER.md](SOURCE_MIXER.md) for calibration and protection semantics.

Oscillator phases and smoothing histories belong to each voice. Idle reuse
resets stale envelope history. Release-only Mono events sharing a sample are
resolved before that sample renders; equal-offset MIDI arrival order is otherwise
preserved. Overflow clears uncertain queued input and invokes all-channel
All Sound Off recovery including arpeggiator/effect cleanup, preserving bend/mod.

## Host and real-time boundaries

Storage is prepared before playback. Audio processing must not allocate heap
storage, access files/network, log, or take file-operation locks. Host adapter
callbacks must serialize access to the audio-owned MIDI queue. The queue keeps
future-block events and processes each MIDI event at its sample offset.

Parameter values are read and applied once per audio block. Current tests prove
MIDI timeline equivalence, not sample-accurate parameter automation or atomic
multi-parameter preset application. Those are separate future wrapper audits.
User-file parsing and mutations stay outside sample processing.

GUI preset loading currently sends individual parameter gestures to the adapter.
Host state and standalone presets share the versioned codec; compatibility
allows older partial states and the optional VST3 bypass trailer. IDs are append
only. The current state contains 93 parameters. Changing decoding strictness or
rounding requires explicit compatibility tests, not a silent format migration.

The scope and meter publish telemetry for the GUI without making audio dependent
on the editor being open. Current wrapper CPU overhead includes control updates
and telemetry; the engine-only benchmark does not measure all that overhead.

## Stable identity and intentional boundaries

Manufacturer: RobCZart82; plugin ID: SwSt; manufacturer ID: RC82; bundle identity:
`io.github.robczart82.sawstar`. Preserve these and parameter IDs for old projects.

Shipping targets are Windows x64/ARM64 and macOS Universal. Linux currently runs
foundation/quality tests, not an advertised Linux VST3 distribution.

RC7 experimental crossfades, automatic pitch smoothing and DC resets are not part
of this baseline. The closed click/pop investigation does not establish a new
engine change. Filter oversampling and 32 voices remain separately scoped work.

White/Dark noise preserve their original streams. Pink uses an independent
seeded generator with staggered octave rows. All remain per-voice mono sources
feeding the existing voice filter/envelope; no additional dependency is introduced.
