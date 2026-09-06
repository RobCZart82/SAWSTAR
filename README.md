# SAWSTAR

**Simple Synth. Real Sounds.**

An open-source, saw-focused synthesizer for direct sound design and learning.
Built toward **iPlug2 + a custom SAWSTAR voice/synth engine + DaisySP primitives**,
with a custom 7-Saw/SuperSaw planned after the first playable instrument.

## Concept

![SAWSTAR GUI concept: MAIN, ADVANCED and PRESETS](docs/reference/SAWSTAR_GUI_Concept.png)

*GUI concept — work in progress. The image shows the design vision, not an implemented plugin.*

## Status: VST3 development shell / 0.1.0-dev

The repository now includes an iPlug2 VST3 **silent development shell** with
MAIN / ADVANCED / PRESETS navigation, five host parameters and Load Init.
It is not yet a playable synthesizer: MIDI input is declared but the voice
engine is not connected. The concept above represents the longer-term design.

[Build and test the shell](docs/PLUGIN_SHELL.md). GitHub Actions builds macOS
ARM64 and Windows x64 VST3 development archives, with license notices included.
The original offline foundation and optional DaisySP primitive checks remain
available. No First Sound release, signing/notarization or host certification
is claimed. See the task report for actual host test results.

## First milestone: `v0.1 First Sound`

- [ ] VST3 loads and plays in REAPER on macOS and Windows.
- [ ] Three-tab shell: MAIN, ADVANCED, PRESETS.
- [ ] MIDI input, including sample offsets and note-on velocity zero.
- [ ] Fixed-capacity polyphonic SAWSTAR voice manager.
- [ ] One bandlimited saw oscillator per voice, velocity and Amp ADSR.
- [ ] Stereo output with conservative gain and parameter smoothing.
- [ ] Versioned state save/recall and host automation.

The acceptance procedure is in [docs/MILESTONES.md](docs/MILESTONES.md).

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
