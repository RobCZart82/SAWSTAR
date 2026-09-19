# Character-preserving development — merge review

2026-09-19. Continues milestones 1–2 in MILESTONES_1_2.md.

## 3. Parameter transfer and persistence

`EngineControls.h` is the production ProcessBlock mapping with the original call
order and separate floating/integer accessors. SynthTestRig now uses that mapping.
The frozen LegacyControlReference intentionally remains independent, as a test
oracle. Defaults, whole-state min/max, mixed settings and each of 93 individual
parameter endpoints are encoded/decoded, applied and rendered against it at
44.1/48/96 kHz with note-on/off transitions. This does not imply every parameter
has an audible effect in every fixture.

Existing state tests cover legacy data, truncation, invalid values, optional VST3
bypass trailer and unchanged output on failed decode. No format, parameter ID,
stepped-value rounding or migration rule has changed.

The actual macOS VST3 builds and passes the pinned Steinberg validator: 47 passed,
0 failed. Its sample-automation test reports unread intermediate parameter points:
the plugin retains block-level automation; this is not sample-accurate automation.
A fresh interactive DAW project reload/listening session remains release QA;
codec/engine tests and validator are not described as that session.

## 4. Remaining audit hypotheses: explicit decisions

- CC121: raw bend/mod/pressure reset; existing smoothing approaches the reset
  targets. New lifecycle regression verifies raw state, pedal release and new
  notes in Poly/Mono/Legato. Do not replace smoothing with an audible snap.
- CC120: All Sound Off preserves controllers; all-channel panic clears shared FX.
  Output protection retains recovery history. This is not a full engine Reset;
  changing that policy is deferred rather than silently altering restart dynamics.
- Idle preset edits: the established short control ramps remain. No unconditional
  idle snap, because effect tails and post-voice processing may still be active.
- Live A/D/R changes: added finite/bounded-output and eventual-release regression;
  this checks lifecycle safety, not subjective smoothness of every automation.
- Mono SelectMono splice clearing: no demonstrated failing case in this baseline;
  RC7 tail/crossfade analysis does not directly apply. No speculative patch.
- First-key LFO retrigger order: retained current incoming-order semantics.
  A new order-independent policy would change articulation and needs a separate
  specification. It is not claimed fixed by the glide regression.
- Reset concurrency and host automation: framework lifecycle assumptions unchanged;
  no new locks or allocation in the audio callback. MIDI queue is audio-thread owned.
- CLIP: metering observes protected output. Broader overload indication before
  protection would require a separate signal/UX definition; no threshold change.

## 5. Packaging and integration

Archive release notes now follow release.json (currently 1.0.2 development).
Missing version-specific notes fail packaging instead of silently shipping 1.0.0
notes. No released assets are replaced. Linux quality jobs continue to run on PRs;
branch protection is not weakened or bypassed. Coverage is a report, not a claim
of complete host/UI coverage. New arbitrary thresholds are deferred until measured.

## 6–7. Acceptance and main merge

Local full suite, sanitizer, VST3 validator and remote platform workflows are the
merge checks. The PR records the final results and exact source revision. Merge
is not publication: installers, a release tag and GitHub Release are separate.

Sound evidence from the earlier milestones: 27 stationary synthesis fixtures
matched the chosen 1.0.1 baseline bit-for-bit. This is a bounded compatibility
check, not a claim that the intentional transient bug fixes change no sample.
