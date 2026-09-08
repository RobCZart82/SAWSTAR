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

PRESETS: actual existing factory entries and their learning descriptions, plus
a schematic signal-flow diagram. Custom sounds continue to be saved through the
host's preset/project facilities. This does not add a new preset bank or pretend
that search/categories/favorites and standalone file actions already exist.

Knobs retain iPlug's drag/value-entry behavior. Faders have rectangular handles
and scale marks. On/Off toggles use a restrained blue fill/outline. Curve views
are schematic visualizations, not measured frequency responses or scopes.
Telemetry uses atomics: current block stereo peaks, active voices, sample rate
and smoothed render-time/audio-duration CPU ratio. It is not the OS CPU meter.

## Deliberate differences from the artwork

Master WIDE is now implemented alongside Output Boost. No dummy macro, variable
polyphony, extra detune/spread, editable arp sequence, or fake preset count is
shown. Preset actions/search/categories need separate library functionality.
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
