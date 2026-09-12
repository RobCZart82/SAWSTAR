# Engine stress test (1.0.1 preview)

The standalone `sawstar_torture` executable exercises the engine and arpeggiator in memory. It never opens an audio device, writes presets/WAVs, changes installation files, or accesses the network. It consumes CPU while running; stop it with Ctrl+C. It is not compiled into the plugin.

Configure with `-DBUILD_TESTING=ON -DSAWSTAR_CHECK_DAISYSP=ON`, then build `sawstar_torture`. Run `ctest --test-dir build -C Release -R torture --output-on-failure`. CI runs 1200 blocks per rate with a 180-second timeout. Invalid CLI arguments are tested separately.

For a longer local run:

```sh
./build/sawstar_torture --steps 100000 --seed 0x9876
```

Multi-configuration builds place the executable under `build/Release/`. Windows uses `.exe`. Defaults are 10000 blocks at each of 44100, 48000 and 96000 Hz. Steps are limited to 1..1000000; sample rate accepts integer Hz from 8000..384000. Large runs can still be expensive. `--sample-rate 48000` selects one rate; `--verbose` prints the last 32 events.

## Coverage and limits

Random MIDI events include repeated note-on/off, sustain, channel pressure, mod wheel, pitch bend, controller resets and panic; parameters cover oscillators, filters, envelopes, LFOs, modulation, effects and ARP. Processing groups vary from 1 to 2048 samples. Checks reject non-finite audio, final output beyond the safety bound, voice counts outside 0..16 and audio/voices remaining after global CC120.

This calls SynthTestRig, not the VST3 host callback. Block grouping does not validate host buffer handling, GUI threading, preset files, installers, audio quality or CPU deadlines. Panic verification does not prove every balanced note sequence releases correctly; keep the dedicated overlap/short-note tests. A finite set of random seeds is not exhaustive.

Failures report the original seed, sample rate, block and step plus recent events. Re-run with that seed and sample rate and at least `step + 1` steps. The output hash is useful for repeatability within the same build/platform, not as a cross-platform golden checksum. Replaying the same seed on the same build yields the same event sequence.

## Review and validation (2026-09-12)

Reviewed against preview engine f301962. No external I/O or plugin runtime changes. Fixed permissive numeric parsing (negative values, trailing garbage, excessive step counts), added explicit standard header, and removed derived-seed reporting that prevented direct replay. Retained CTest timeout. GUI layout is untouched.

Local Apple Silicon: 10000-step matrix seed 0x1234 passed, 1200-step AddressSanitizer/UndefinedBehaviorSanitizer matrix passed, 8 invalid-input checks passed, and two 48000 Hz replay runs matched exactly. Additional long-run results are recorded in PR #2.
