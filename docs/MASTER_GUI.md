# Master GUI — first cosmetic pass

The approved three-page render is the primary visual reference. This first pass
uses slightly brighter charcoal panels while preserving the existing parameter
set and working layout; rendered labels are not specifications for new engine
features. MAIN retains its section color coding. ADVANCED remains neutral/blue.

## Implemented

- Shared gently shaded panel surfaces, clearer borders and restrained blue accents.
- Recessed dark waveform/response displays with quiet grids; source selectors use
  an explicit dropdown field. Waveforms remain schematics, not measured unison.
- Refined knob rims and 270-degree dotted scales; slim blue filled fader tracks,
  scale marks and subtly shaded rectangular handles.
- OUTPUT fader beside a wider, legible stereo segmented meter, aligned to the
  exact same vertical track span. Twenty cells per channel: 14 green, 4 yellow,
  2 red. Unlit cells stay dim. Existing peak values and dB mapping are unchanged.
- Clear inactive page buttons; color-coded saved-preset signal-flow diagrams.
- Slightly larger spaced wordmark; permanent lower-right motto:
  `TRANCE  •  HOUSE  •  AMBIENT  •  BEYOND`.
- The same motto beneath the first slogan in About, plus a compact vector
  rendition of the user's GYR monogram. No external bitmap dependency is added.

No DSP, parameter IDs, preset format or preset actions change in this pass.
The automatic pre-FX scope retains its existing capture and display behavior.
The image's simplified ADVANCED panel does not remove existing glide, bend,
LFO phase/clock or effect controls. Factory preset development remains separate.

## Acceptance

Candidate `0449b09f2cb2c446cee971901e055af75329431e` passed all 34 foundation
tests and all 47 VST3 validator checks in both VST3 Release builds.
Downloaded macOS/Windows artifact digests and package ZIP integrity were verified.

In REAPER 7.79/macOS, MAIN, ADVANCED, PRESETS and About were inspected at 100%.
The source selector fields, saved-preset diagrams, motto and compact GYR mark
were visible. SuperSaw One loaded from the preset browser. Project MIDI playback
showed illuminated stereo meter cells and a moving pre-FX trace. MAIN was also
inspected at 75%, then returned to 100%. No label/control overlap was observed
in these views. Editor close/reopen worked. The separate test project was closed
without saving its temporary parameter changes; the original empty tab remains.

The macOS bundle is installed with a backup of its preceding version. Windows
manual GUI acceptance and the 125% layout remain to be checked; these are not
implied by CI success.

- [macOS build](https://github.com/RobCZart82/SAWSTAR/actions/runs/34564119821)
- [Windows build](https://github.com/RobCZart82/SAWSTAR/actions/runs/34564119450)
