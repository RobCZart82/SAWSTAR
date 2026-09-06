# SAWSTAR concept

## Identity

- Product and repository: **SAWSTAR**.
- Tagline: **Simple Synth – Big Sound**.
- GUI brand lockup: large, spaced uppercase **SAWSTAR**, with **Simple Synth**
  and **Big Sound** in two smaller left-aligned lines immediately to its right.
  The two lines together match the logo height; **Big Sound** has subtle emphasis.
- Project owner: **RobCZart82**; an account name, not an invented legal vendor.
- Development version: `0.1.0-dev`; CMake numeric version: `0.1.0`.
- Initial platforms: macOS and Windows; initial format: VST3 instrument.
- AU and CLAP are later goals, not v0.1 requirements.
- Original source and documentation: MIT.

SAWSTAR makes saw-based sounds approachable without hiding the musical signal
path. The long-term character is broad, expressive saw and SuperSaw synthesis,
with clear controls and presets that teach how a sound is built.

## GUI philosophy

**MAIN = sound design / signal flow.** Present oscillator, shaping/filter,
amplifier and output in musical order. For First Sound only the saw, Amp ADSR
and output are functional. Future functions should be absent or explicitly
marked as upcoming, never presented as working controls.

**ADVANCED = performance + arpeggiator + modulation.** This is a coherent home
for playing behaviour and movement. Initially it is a labelled shell. An
arpeggiator or modulation matrix is not required for First Sound.

**PRESETS = library + learning.** Ultimately support discovery, tags, sound
notes and explanations of useful patches. First Sound needs Init and reliable
host state restoration; a full browser, database and factory library follow.

The three-page concept is fixed. The SAWSTAR-branded concept image is in
`docs/reference/SAWSTAR_GUI_Concept.png`. Its provenance is recorded beside it;
it is a design reference rather than a screenshot of an implemented plugin.

## Scope discipline

First Sound proves the complete host-to-audio path. One polyBLEP saw and Amp
ADSR are enough. A custom seven-saw algorithm, filters, effects, arpeggiator,
modulation matrix and preset lessons belong after that path is dependable.
DaisySP supplies selected primitives, not the synth architecture. iPlug2
supplies host integration and UI plumbing, not SAWSTAR's voice allocation.
