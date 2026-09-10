# PRE-FX waveform display

The MAIN AMP ENV panel retains its envelope diagram and controls. Below a quiet
horizontal divider, WAVEFORM shows the actual summed signal after voice amplitude
envelopes, filtering and modulation, before chorus/delay/reverb and master output
width, gain, boost and protection. Stereo is displayed as `(L + R) / 2`; phase
cancellation between channels can therefore reduce the visible waveform.

The display uses a dark square grid, a slightly stronger zero-amplitude center
line and a bright green trace with restrained glow. The horizontal axis is time,
not frequency. No parameter, mouse action or saved preset field is added.

## Reading the scope

The view spans 32 ms at normal audio sample rates (tested through 192 kHz). A rising
zero crossing with 2% hysteresis stabilizes periodic sounds; absent a trigger, the
latest window still draws. SuperSaw, chords and noise naturally remain complex.
Native samples are retained without smoothing or decimation. Per-column extrema
preserve short peaks when many samples occupy one display pixel. The trace is
not a frequency analyzer or a calibrated output-level meter.

AUTO means display-only amplitude scaling, with a fixed low-level floor to avoid
magnifying silence. Output Volume does not change this pre-master view. The VU
meter remains the indication of actual output level. The fixed timebase shows
roughly one 32 Hz cycle or several cycles of higher notes; it does not promise a
stationary single period for every chord or detuned signal.

## Thread and editor lifecycle

- Audio writes a fixed ring and publishes up to 30 frames/second into a bounded
  single-producer/single-consumer queue using lock-free ownership indices.
- No allocation, mutex, GUI pointer or consumer wait enters audio capture. Full
  queues drop display frames. Audio output samples are never changed by scope.
- Capture continues with bounded work while the editor is closed. The queue owns
  all samples for the lifetime of the plugin, not the editor.
- Idle drains the queue into editor-owned data. Only the active MAIN page is
  dirtied for animation. Partial OS-window occlusion is not a stop condition.
- Publication timestamps reject old queued frames. If processing stops, the
  display returns to the center line after 200 ms, on the next available UI tick.
  If the OS suspends UI callbacks completely, the screen can only update once
  callbacks resume; audio never depends on them.
- Audio Reset resets capture history, not shared queue ownership. Resize does not
  affect buffers. Separate plugin instances own separate buffers.

## Verification

Local ASan/UBSan and ThreadSanitizer tests passed for native-rate capture at
44.1/48/96/192 kHz, triggering, nonfinite input sanitization, stalled-consumer
queue saturation, concurrent consumption and reset recovery.

Manual host acceptance: inspect Sine, Saw, Square, SuperSaw, chords and silence;
partly cover the editor with another window; switch tabs repeatedly; close/reopen
and resize; stop host processing; use multiple instances. Confirm fresh output
on return and unchanged sound. These visual cases require host acceptance and
are not established by the automated queue tests.

A local optimized microbenchmark captured/transferred 10 seconds of 48 kHz
samples in approximately 2.9 ms. This excludes GUI drawing and is not a DAW CPU
guarantee; platform/host acceptance still matters.

## Host smoke test — 2026-09-10

REAPER 7.79/macOS opened build `5a0472a`. The scope was visually checked in
silence and with a Saw note and long release. The green saw trace was visible,
and remained fresh after MAIN → ADVANCED → PRESETS → MAIN. Editor close/reopen
worked. The separate test-project tab was closed without saving test changes;
the original open project was preserved. Partial external-window occlusion,
resize, all other waveforms and Windows GUI acceptance remain manual checks.

Both VST3 Release jobs passed 34 tests and 47 validator checks; downloaded
artifact SHA-256 and inner ZIP checks passed. The macOS bundle is installed,
with its previous version preserved.

- [macOS CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34533561714)
- [Windows CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34533561591)

All six macOS/Windows Debug, Release and VST3 jobs completed successfully.
