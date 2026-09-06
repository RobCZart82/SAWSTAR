# Milestones

## Foundation (this repository bootstrap)

- [x] Public SAWSTAR repository and MIT original-source license.
- [x] Concept, layer boundaries, identity candidates and parameter contract.
- [x] Reserved source/assets/reference folders with explicit responsibilities.
- [x] Pinned iPlug2 and DaisySP references; LGPL extension excluded.
- [x] Buildable parameter/navigation foundation and optional DSP integration check.
- [x] macOS and Windows build/test workflows.

Workflow execution results belong in the task report; these checkboxes mean
that configuration/code exists, not that a host-load test has been performed.

## v0.1 First Sound — not complete

### 1. Host shell

Create the VST3 target with fixed identity and resources. Load it in REAPER on
both platforms, open/close the editor repeatedly, switch MAIN / ADVANCED /
PRESETS and verify silent, finite output before MIDI arrives. Archive build
logs and unambiguous platform/architecture information.

### 2. First musical path

Implement 16-voice allocation, per-voice polyBLEP saw, Amp ADSR, velocity,
conservative output and gain smoothing. Test chords, repeated notes, 17+
notes, note-on velocity zero, note-offs at block boundaries, all-notes-off,
sample-rate changes and two independent plugin instances.

### 3. Persistence and acceptance

Automate every parameter; save/reopen the REAPER project and compare all
values and the resulting sustained sound. Test corrupted/future state rejection
and Init. Check 44.1/48/96 kHz and block sizes 1/64/512, offline render,
transport restart, voice release and stuck-note recovery. Review CPU and peak
levels; ensure no audio-thread allocations/locks. Verify both macOS and Windows
VST3 artifacts. Only then tag/release `v0.1.0` as First Sound.

## Later

Custom 7-Saw/SuperSaw with measured detune/stereo behaviour; filters and effects;
performance controls; arpeggiator and modulation; annotated factory presets and
learning browser; AU/CLAP; signing and notarization/distribution automation.
