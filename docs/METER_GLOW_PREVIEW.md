# Unreleased 1.0.1 meter / display preview

This branch is for REAPER evaluation only. It does not replace the public 1.0.0
release or its downloadable files. Host version: 1.0.1; About: Pre Release.

- Preserve every control rectangle and the meter's outer bounds / fader alignment.
- Keep the twenty-segment 70% green / 20% yellow / 10% red scale.
- Active-only LED glow, 65 ms visual afterglow, fast attack and 24 dB/s fall.
- Separate one-second peak hold, followed by 18 dB/s fall, timed with steady clock.
- Accumulate audio block maxima until the editor consumes them. Drain when hidden;
  reset displayed history when reopening or returning to MAIN.
- Red caps latch actual output sample peaks >= 0 dBFS; click the respective cap
  to clear, or wait five seconds since the last clipped sample. This is not a true-peak detector or a limiter-activity indicator.
  Existing 0.98 output protection normally prevents it from lighting.
- Inset dark/light display bezels; subtle extra scope-line glow.
- GYR mark moved 60 logical pixels right, unchanged size; separator below URL.

No RMS mode, synthesis changes, preset schema changes or panel darkening.
No offscreen blur layers per LED. The tests cover transient retention, independent
stereo peaks, clip latch/reset, silence and frame-rate-independent hold/decay.

Try short plucks, sustained pads, silence, tab changes, closing/reopening the
editor, partial window overlap, resizing, and stopping/resuming audio. The line
must decay naturally without stuck historical peaks or changes to the sound.
