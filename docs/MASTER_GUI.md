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
  `The  Sounds  of  Trance` (double spaces between words).
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

## Motto correction and actual three-page screenshot

Build `ab3d0ecbf596934da9908e9fef586c16c962d5c8` restores the approved
`The  Sounds  of  Trance` wording, shared by the footer on all pages and About.
The larger footer typography is retained. The first public release is planned as
1.0.0; this remains a 0.1.0 development build, not a published release.

On 2026-09-11, this macOS build was installed and inspected in REAPER 7.79.
The motto was verified in MAIN, ADVANCED, PRESETS and About. The README image
contains authentic captures of the three main pages, cropped to the plugin
surface and stacked in that order. No GUI content was painted or generated.
MAIN uses temporary attack/release edits to SuperSaw One to capture the live
scope and stereo meter; ADVANCED and PRESETS show the saved SuperSaw One sound.
The test project was closed without saving changes.

Both macOS and Windows VST3 Release builds passed all 34 foundation tests and
all 47 VST3 validator checks. The installed macOS artifact's SHA-256 and ZIP
integrity were verified before installation.

- [macOS correction build](https://github.com/RobCZart82/SAWSTAR/actions/runs/34566162945)
- [Windows correction build](https://github.com/RobCZart82/SAWSTAR/actions/runs/34566162923)

## Backlighting pass

Build `3121f84da4f3189b02d0a4c18e16e0ab143e11b9` adds restrained layered blue
light to knob rims/value arcs, slider handles/tracks, selected page/LFO/effect
tabs and enabled toggles. Lit meter cells have a faint halo in their own color;
the 14/4/2 cell zones and meter mapping are preserved. Labels and scale marks
remain sharp. Active button lighting is inset into the existing control bounds.
The implementation uses a small fixed number of vector draw layers, with no
blur buffers, extra animation timers or changes to audio processing.

All three pages were visually checked at 75%, 100% and 125% in REAPER 7.79 on
macOS. LFO/effect tab selection, About open/close, and live scope/meter drawing
worked; no overlap was observed in the inspected views. Scale was returned to
100% and the separate test project closed without saving temporary edits.
Windows manual visual acceptance remains pending.

Both VST3 Release builds passed 34 foundation tests and 47 validator checks.
The macOS package was verified and installed with a backup of the prior bundle.
README screenshots now show this build: three plugin-only pages and a separate
cropped About window. The images contain only authentic screenshot pixels.

- [macOS backlighting build](https://github.com/RobCZart82/SAWSTAR/actions/runs/34568237577)
- [Windows backlighting build](https://github.com/RobCZart82/SAWSTAR/actions/runs/34568237568)
