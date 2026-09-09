# Modular GUI implementation

The approved concept is the visual reference; no new concept image is generated.
The editor is 1280 x 760 logical pixels, with iPlug's screen scaling, shared white
branding, page tabs/preset selector, keyboard/wheels and a bottom status strip.
MAIN has tinted, color-coded headers only; ADVANCED uses one standard blue palette.
The DSP parameter IDs, preset data format and synthesis algorithms are unchanged.

## Layout and controls

MAIN: oscillator waveform selectors and octave/detune/unison/width; mixer with
four rectangular faders, horizontal Noise Color and noise menu; schematic filter
curve, cutoff/resonance/drive, filter mix/key tracking and bottom mode selector;
parameter-linked filter/amp envelope diagrams and controls; three FX mix knobs
and On/Off toggles; output fader, level meter, Output Boost and [master WIDE](MASTER_WIDTH.md).

ADVANCED: performance, arp, shared LFO 1/2 editor and all four modulation rows are
visible together. The lower effect-details panel switches between chorus, delay
and reverb. The persistent MAIN/ADVANCED/PRESETS tabs do not affect DSP state.

PRESETS: the approved four-column browser with shared factory/user entries,
category and search filters, persisted favorites, saved-settings learning diagrams
and file actions including native batch import. See [Preset library](USER_PRESETS.md).

Knobs retain iPlug's drag/value-entry behavior. Faders have rectangular handles
and scale marks. On/Off toggles use a restrained blue fill/outline. Curve views
are schematic visualizations, not measured frequency responses or scopes.
Telemetry uses atomics: current block stereo peaks, active voices, sample rate
and smoothed render-time/audio-duration CPU ratio. It is not the OS CPU meter.

## Deliberate differences from the artwork

Master WIDE is now implemented alongside Output Boost. No dummy macro, variable
polyphony, extra detune/spread, editable arp sequence, or fake preset count is
shown. Preset library categories/search/favorites and user file actions are implemented.
This is the working GUI implementation of the current feature set, with further
visual refinement possible after host inspection.

## Validation (2026-09-08)

Code commit `8cb285cff3dc21ad36c60f5bcddce05a14afa049` passed macOS and Windows
Debug/Release engine checks and Release VST3 build, processing/state validation:

- [macOS run](https://github.com/RobCZart82/SAWSTAR/actions/runs/34166408157)
- [Windows run](https://github.com/RobCZart82/SAWSTAR/actions/runs/34166408060)

REAPER 7.79 on macOS was used with a separate workspace bundle to inspect MAIN,
ADVANCED and PRESETS, switch LFO banks, and select SuperSaw One from the library.
Numeric labels overlapping controls were found and corrected; the corrected MAIN
and ADVANCED were inspected again. A final modulation-row height correction was
then built and validated by CI. This is not an exhaustive manual test of every
control or Windows GUI scaling. Original REAPER VST search paths were restored.
The system-installed plugin was not overwritten.

## Visual polish pass (2026-09-09)

Parameter dropdowns now use recessed dark value fields with a right-side blue
chevron and a separate label. Noise type shares this field style. On/Off controls
retain their blue active state and explicit ON/OFF text. Parameter IDs, popup
selection handling, saved state and DSP are unchanged.

The stereo OUTPUT peak display now has a restrained fixed-height gradient:
approximately 70% green, 20% orange and 10% red from bottom to top. The palette
stays anchored to the whole meter rather than stretching with the instantaneous
level. Its existing -60 to 0 dB mapping and block-peak readings remain unchanged;
it is not a calibrated analog VU simulation or a new clipping detector.

Validation for code commit `fb520b597f21a1de862bf2955d397fab02b6ac14`:

- [macOS checks and VST3 build](https://github.com/RobCZart82/SAWSTAR/actions/runs/34305461225)
- [Windows checks and VST3 build](https://github.com/RobCZart82/SAWSTAR/actions/runs/34305461265)

Native REAPER visual inspection of this pass is still pending. A separate test
bundle was scanned, but UI automation did not reliably open the FX window.
The original VST search paths were restored; the installed bundle was not replaced.

## Settings / About (2026-09-09)

The persistent top-right gear opens a menu with About SAWSTAR. Selecting it opens
a centered About window inside the plugin editor, above the current page. It shows
the original creator logo, Gyuricza Róbert, the plugin version and project address.
OK closes the window; Escape/Enter also close it when the editor receives the key.
The full-editor overlay intercepts mouse clicks and wheel events while visible.
DAW playback and incoming MIDI continue normally.

The preset selector is slightly narrower to reserve the gear's own click area.
MAIN, ADVANCED and PRESETS share the same settings control. The logo is embedded
from assets/branding/GYR-Logo1.png at build time, unchanged, with its aspect ratio
preserved and a light backing for the black lettering. It needs no external file
at runtime. Creator branding is excluded from the source-code MIT license; see
[branding notice](../assets/branding/README.md).

About validation uses code commit `d4de33c0a3fdb4b64865f16e2dfd958723cabc8f`:
[macOS run](https://github.com/RobCZart82/SAWSTAR/actions/runs/34307025293),
[Windows run](https://github.com/RobCZart82/SAWSTAR/actions/runs/34307025243).
The Release VST3 jobs run the foundation checks and VST3 processing/state validator.
Interactive About menu/OK behavior still requires a native host GUI check; a build
or binary-resource check alone does not prove that interaction.
All six jobs completed successfully: macOS/Windows Debug, Release and VST3 Release.
Both packaged plugin binaries were checked for the exact original PNG bytes, and
both downloadable ZIP archives passed their integrity checks.

## Current About and action safety

The About creator PNG is replaced by an embedded Orbitron Bold SAWSTAR wordmark.
The original asset is archived only. See [editor-close fix and confirmations](EDITOR_CLOSE_FIX.md)
for the Retina cache ownership issue, safety controls and host-test limits.

## Control finish pass

Inactive page tabs and on/off controls have dark recessed fills and visible
borders. Slider tracks share dim-blue empty tracks, blue value fill and eight
intervals of subtle tick marks. The output meter reads its vertical endpoints
from the volume slider track, avoiding label/handle padding mismatches. Meter
height zones remain approximately 70% green, 20% yellow, 10% red.

ARP HOLD occupies the former ORDER field and ORDER the former HOLD field.
Non-interactive dividers separate oscillator, FX, mixer, performance and output
control groups, including both sides of BOOST. No parameter IDs or DSP change.
