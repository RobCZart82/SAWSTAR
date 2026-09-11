# SAWSTAR

**Simple Synth – Big Sound**

An open-source, saw-focused synthesizer for direct sound design and learning.
Built toward **iPlug2 + a custom SAWSTAR voice/synth engine + DaisySP primitives**,
with a custom [7-Saw unison layer](docs/SEVEN_SAW.md).

## Plugin screenshot

![SAWSTAR Master GUI: MAIN, ADVANCED and PRESETS](docs/reference/SAWSTAR_Master_GUI_REAPER.png)

*Actual Master GUI 1 running in REAPER 7.79 on macOS, captured on 2026-09-11. MAIN, ADVANCED and PRESETS are cropped to the plugin only and stacked vertically. MAIN shows a live pre-FX waveform and stereo output meter using temporary edits to SuperSaw One; the other views show the saved preset. These are screenshots of the development build, not concept renders.*

[Master GUI details](docs/MASTER_GUI.md) · [Earlier design concept](docs/reference/SAWSTAR_GUI_Concept.png)

### About

![SAWSTAR About window](docs/reference/SAWSTAR_About_REAPER.png)

*Actual About window from the same development build, with the GYR monogram and both mottos.*

## User presets

The concept-based [preset browser](docs/USER_PRESETS.md) combines factory and user
sounds with categories, search, persistent favorites and saved sound-flow diagrams.
Save, Save As, Copy/Paste, Rename, recoverable Delete, Init and native multi-file
Import are available. One sound remains one `.sawstar` file; there is no bank format.

## Real-time waveform

The MAIN AMP ENV panel includes a green [PRE-FX oscilloscope](docs/REALTIME_SCOPE.md),
with a fixed 32 ms view and no additional sound parameters.

## Master WIDE

The OUTPUT panel now offers [master stereo width](docs/MASTER_WIDTH.md), with
a smoothed switch and amount control. Older presets retain their original sound.

## Modular GUI

The working editor now has a 1280 × 760 modular layout: tinted MAIN headers,
uniform blue ADVANCED panels, and a shared factory/user preset learning page. The white logo,
tabs, preset selector, keyboard, wheels and status strip stay fixed across pages.
See [GUI implementation and validation](docs/GUI_IMPLEMENTATION.md) for controls,
known differences from the artwork and host-check coverage.

The September 10 [audit fixes](docs/AUDIT_FIXES.md) protect concurrent preset saves,
smooth SUB waveform changes and synchronize preset/keyboard display state.
Knobs now share a subtle 270-degree dotted scale.

Repeated MIDI notes now use [paired press/release handling](docs/OVERLAPPING_NOTES.md),
including ARP bypass and all three voice modes.

The [preset reliability and GUI plan](docs/PRESET_GUI_PLAN.md) covers safe favorites,
external saves, a scrolling preset list and consistent dark popup/input styling.

## Status: 0.1.0 development build

SAWSTAR has a working VST3 editor and 16-voice synth engine: two waveform/unison
oscillators, a selectable SUB, White/Pink/Dark Noise with Color, mixer, filter
and envelopes, chorus/delay/reverb, WIDE, performance controls, two LFOs,
four modulation routes and an arpeggiator. Sixteen Factory presets are embedded (five Templates and eleven finished sounds);
user sounds use individual `.sawstar` files with multi-file import.

The shared MAIN / ADVANCED / PRESETS shell includes a 61-key audition keyboard,
pitch/modulation wheels, preset selector, build metadata and GUI scaling.
The DAW can still send all 128 MIDI notes. The concept is a design reference;
see [GUI implementation](docs/GUI_IMPLEMENTATION.md) for actual behavior.

There is no tagged public release yet. Automated macOS/Windows builds and VST3
validation are available; the previous Windows build passed user testing. A focused macOS automation
and offline-render check also passed. Final content listening and acceptance
of the release-candidate binary remain required.

- [First release plan](docs/FIRST_RELEASE_PLAN.md)
- [Latest Templates / host QA](docs/TEMPLATES_HOST_QA.md)
- [Earlier engine/GUI QA](docs/PRE_RELEASE_QA.md)
- [Installation and release preparation](docs/INSTALLATION.md)
- [Windows manual checklist](docs/WINDOWS_ACCEPTANCE.md)

See [the embedded library](docs/FACTORY_LIBRARY.md) for Templates and the
refreshed sounds. Final musical acceptance is still required. AU/CLAP are outside the first VST3 release scope.

## Build the foundation

Requires CMake 3.21+, Git and a C++17 toolchain: Xcode/Command Line Tools on
macOS, or Visual Studio 2022 with Desktop development with C++ on Windows.
Review and accept the Xcode license yourself when required by Apple.

```sh
git clone https://github.com/RobCZart82/SAWSTAR.git
cd SAWSTAR
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The default build is offline after clone and builds `sawstar_foundation` only.
To compile and test the two selected DaisySP primitives as well:

```sh
git submodule update --init third_party/DaisySP
cmake -S . -B build-dsp -DBUILD_TESTING=ON -DSAWSTAR_CHECK_DAISYSP=ON
cmake --build build-dsp --config Release
ctest --test-dir build-dsp -C Release --output-on-failure
```

For future plugin work, initialize iPlug2 with
`git submodule update --init third_party/iPlug2`.
Do not use recursive initialization: DaisySP-LGPL is deliberately excluded.
No SDK download scripts run automatically. See [BUILDING.md](docs/BUILDING.md).

## Design

| Tab | Purpose |
| --- | --- |
| MAIN | Sound design and visible signal flow |
| ADVANCED | Performance, arpeggiator and modulation |
| PRESETS | Library, discovery and learning |

- [Concept](docs/CONCEPT.md)
- [Architecture and real-time boundaries](docs/ARCHITECTURE.md)
- [Parameter and state contract](docs/PARAMETERS.md)
- [GUI reference slot](docs/reference/README.md)
- [Dependency versions and licensing](docs/THIRD_PARTY.md)

`src/plugin` adapts the host; `src/engine` owns synthesis; `src/dsp` wraps
primitives; `src/midi` handles musical events; `src/presets` owns persistence;
`src/gui` presents parameters. Reserved folders have a README documenting their
purpose instead of dummy implementations.

## License

Original SAWSTAR code and documentation: [MIT](LICENSE), copyright SAWSTAR
contributors. Third-party components retain their own licenses. iPlug2 uses a
zlib-style license; the selected DaisySP core uses MIT. The optional LGPL
extension is not part of this build. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
Artwork, fonts and external presets must have recorded redistribution rights
before being added; the concept image is included as a design reference.

The filter now includes an independent per-voice ADSR, bipolar envelope amount
and keyboard tracking. See [filter modulation](docs/FILTER_MODULATION.md).

Pitch bend and CC1 now work per MIDI channel, with shared on-screen wheels
and ADVANCED depth controls. See [performance controls](docs/PERFORMANCE.md).

The [four-source mixer and output calibration](docs/SOURCE_MIXER.md) add an
independent OSC2, sine SUB and White/Dark Noise. New sounds use +18 dB Level
Boost with stereo peak protection; old project levels are preserved.

[Filter Drive and four filter modes](docs/FILTER_CHARACTER.md) now provide
LP12, LP24, HP12 and BP12. Raise Filter Mix to engage them; existing projects
retain undriven LP12 until changed.

[Oscillator waveform previews and LFO routing](docs/LFO_WAVEFORMS.md) add
independent Saw/Square/Triangle/Sine selection plus an ADVANCED LFO with
free/tempo rate, first-key retrigger and Cutoff/Pitch/Amp/Pan destinations.

Stereo chorus is available on ADVANCED (Off by default). See [chorus](docs/CHORUS.md).

Stereo/ping-pong delay is available on ADVANCED (Off by default). See [delay](docs/DELAY.md).

Stereo reverb is available in ADVANCED → REVERB (Off by default). See [reverb](docs/REVERB.md).

Mono/Legato and Glide are available on ADVANCED. See [performance modes](docs/MONO_GLIDE.md).

Noise: White, Dark and Pink sources are available in MAIN's mixer, with an
independent bipolar NOISE COLOR control (0 = original source). Existing presets
retain their White/Dark source and neutral color.

ADVANCED now includes two independent LFOs with a shared LFO 1 / LFO 2 editor,
and four source/destination/amount modulation rows. New routes are disabled by
default. See [Modulation](docs/MODULATION.md) for controls and compatibility.

ADVANCED → ARP now includes five patterns, tempo-synced rate, gate, octaves,
swing and Hold. It is disabled in existing presets. See
[Arpeggiator](docs/ARPEGGIATOR.md) for playing and transport behavior.
