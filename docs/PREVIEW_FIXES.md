# Unreleased 1.0.1 follow-up

- Idle voices snap waveform, unison, sub shape and filter targets before their first processed sample. Active retriggers, steals and legato retain smoothing. Master gain starts at the host's initial target after Reset.
- Reverb target coefficients are cached until size/decay/damping change; Init invalidates the cache. The wrapper avoids duplicate filter-envelope configuration and reads output channel count once per block.
- Empty state data is rejected before pointer arithmetic. Host groups derive from stable parameter keys; IDs and serialized values are unchanged.
- macOS and Windows preset name keys use Unicode lowercase and canonical decomposition, retaining accents. Save/import/rename reject name collisions under the mutation lock. Native OS Unicode tables may differ for newly introduced characters; this is not a promise of identical filesystem rules on every OS. Non-release platforms retain ASCII behavior.
- CLIP resets on click or 5 seconds after the latest clipped sample, independently per channel. Timestamp expiry works even when the editor is closed.
- Keyboard: one logical pixel red felt line inside existing bounds, brighter pressed keys with inset glow. Knob and fader scale marks are brighter. No control geometry changes.
- EN/HU manuals clarify the existing independent Mod Depth cutoff route. Sound/preset settings for modulation are unchanged.

Deferred: RMS meter, waveform phase redesign, clock-measurement redesign, global darkening and broad setter caching without profiling. The official 1.0.0 release is unchanged.
