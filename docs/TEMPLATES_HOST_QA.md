# Templates and host follow-up — 2026-09-10

## Host follow-up on a9319c0

The user reported successful Windows testing with no observed errors. Hardware
details and individual checklist results were not supplied; this is a user
acceptance report for the preceding build, not a new automated certification.

macOS REAPER 7.79: an isolated two-instance project exercised 12 automation
envelopes: Output, Filter Mix/Cutoff, OSC1 Wave, Chorus enable/mix, Delay
enable/mix, LFO2 depth and modulation route 1 source/target/amount. The saved
project retained all 12 distinct VST3 parameter IDs after REAPER re-saved it.
Saved parameter chunks also showed the automated cutoff and effect mix values
on A, while B retained its own settings. The user's unsaved project was kept
in its original tab; only the test tab was closed afterward.

Full-speed offline **Dry run (no output)** completed at 44.1, 48 and 96 kHz.
At 48/96 kHz, processing at the project/hardware rate was disabled so effects
used the requested render rate. All three runs reported zero clipped samples,
peak -25.7 dBFS and integrated -34.1 LUFS for this deliberately quiet test scene.
No WAV was created. These figures do not describe factory preset loudness.

This covers the selected host automation/offline path. It does not establish
every parameter combination, every buffer size or subjective sound quality.

## Factory / Templates changes

See [FACTORY_LIBRARY.md](FACTORY_LIBRARY.md): five Templates and eleven finished
sounds, including refreshed SuperSaw One/Soft Pad One and four new sounds.
The engine and parameter IDs were not changed. Already saved user sounds and
projects keep their stored values; an old factory snapshot may be identified as
Custom when it no longer matches the updated factory values exactly.

All 33 local tests passed with ASan/UBSan. The expanded factory test now uses
the complete synth/ARP rig. Its first Windows run exposed excessive stack use
in the test executable; allocating the rigs on the heap fixes that test setup
and matches the other full-engine tests. The revised factory test also passed
locally with sanitizers.

The measured library levels are in [factory-levels.csv](factory-levels.csv).
Scene: 48 kHz, notes 48/55/60/64/67 at velocity 110, held for eight seconds,
137 BPM, complete saved signal path. Mono patches naturally retain one voice;
ARP patches sequence the held chord. Peak and stereo sample RMS are measured,
not perceived-loudness matching. All values were finite and below full scale.

Final listening approval and final-release GUI/platform acceptance remain open.
No public release/tag is created by this work.

## Build artifacts

Code build: `9b01575f2eb950ae63c3eb9751884c2b582ff8a1`. Both macOS and
Windows VST3 jobs passed 33 tests and 47 validator checks. Both downloaded
archives matched their GitHub SHA-256 digest and passed ZIP integrity checks.
The new macOS executable was installed and byte-verified against the artifact,
with the preceding bundle backed up. REAPER was left open to preserve the
user's unsaved project; it must restart before loading the new binary. The
above host dry runs used a9319c0; they are not claimed as GUI acceptance of the
new library build.

CI: [macOS](https://github.com/RobCZart82/SAWSTAR/actions/runs/34523475422),
[Windows](https://github.com/RobCZart82/SAWSTAR/actions/runs/34523475404).
