# Voice transition preview — 1.0.2-rc1

Unreleased test build. No GUI layout or preset parameter changes. The public
1.0.1 release remains unchanged.

## Confirmed defects fixed

- Mono and Legato reused a running voice without the 3 ms output-continuity
  correction already used for Poly voice reuse. They now preserve the last
  rendered sample on a new note or held-note fallback.
- Unrelated note releases used to cancel any correction in progress.
- Multiple note-offs at the same sample could select and retrigger intermediate
  fallback notes before any audio was rendered. Note-off and sustain-release
  selection now resolves immediately before the next sample. There is no added
  audio buffering or MIDI delay. New note-ons and panic still resolve immediately.

Mono note-on retrigger, last-held-note priority, glide, and paired-note counters
remain in place. Poly processing is unchanged.

## Regression coverage

`voice_transitions.cpp` checks first-sample continuity in Mono/Legato at
44.1/48/96 kHz with four oscillator waveforms, fallback, unrelated releases,
same-sample release ordering with/without sustain, complete release, and panic.
The release-order regression fails against the original 1.0.1 engine.
Existing Poly retrigger/stealing checks remain in the same test.

Local optimized tests also passed: mono, overlapping notes, short notes, idle
start, reset lifecycle, parameter transitions, engine audit, and deterministic
torture (10,000 steps per rate at 44.1/48/96 kHz).

## Reported bass transient: remaining distinction

The supplied project has a Mono chord where note 68 remains held approximately
25.3 ms after note 80 and the other chord notes are released. Last-note priority
therefore legitimately returns to note 68. The low-pass filter passes that lower
pitch much more strongly. No output clipping was found in the supplied render.

An engine-only reproduction places a short bass burst at the same transition;
it is not a sample-identical reproduction of the host render. Its peak in the
5.710–5.736 s window is about 0.04280 before and 0.04278 after these fixes.
Thus these corrections **do not resolve that audible bass burst**. Removing the
lingering note in a diagnostic copy removes the burst; Poly also avoids this
particular mono fallback, while producing the intended full chord.

Suppressing a genuinely held note, adding a fallback grace period, or changing
Mono envelope retrigger semantics would change the instrument's playing
behavior. None is silently included in this patch. The original project and
audio are preserved outside the repository; a host listening comparison remains
necessary before deciding on any further Mono behavior change.
