# SAWSTAR VST3 development shell

This is a silent instrument shell, not the First Sound release. MIDI input is
declared but ignored until the voice engine is connected. MAIN contains the
five host parameters; ADVANCED and PRESETS are real navigation pages with
honest placeholder text. Load Init restores the five defaults through host
parameter gestures. The current framework provides parameter state persistence;
the custom versioned preset format is still planned.

Build requirements: CMake 3.21+, Python 3, Git, C++17, Xcode or Visual Studio 2022.

```sh
python scripts/bootstrap-plugin.py
cmake -S . -B build-plugin -DCMAKE_BUILD_TYPE=Release -DSAWSTAR_BUILD_PLUGIN=ON -DIPLUG_DEPLOY_PLUGINS=OFF
cmake --build build-plugin --config Release --parallel
```

The bundle is build-plugin/out/SAWSTAR.vst3. No automatic host installation
occurs. macOS uses NanoVG/Metal, Windows uses NanoVG/OpenGL 2. An installed
Arial system font is used without redistributing a font file. The visual shell
uses the concept's dark palette and three tabs; it is not a pixel-identical
implementation of the concept image.

## Identity

Product SAWSTAR; manufacturer RobCZart82; plugin ID SwSt; manufacturer ID RC82;
macOS bundle io.github.robczart82.sawstar. Version 0.1.0 is a development version,
not a published First Sound release. Freeze these identifiers before wider use.

## Manual REAPER check

Add the bundle's parent folder to REAPER's VST paths or install it to a standard
user VST3 directory. Rescan, insert SAWSTAR on a new track, open and switch all
three tabs, adjust the five parameters and load Init. Save/reopen a disposable
project and compare parameter values; test two instances and repeated editor
open/close. All output must remain silent and finite, even when MIDI is sent.
This checklist is not evidence of a completed REAPER test; see the task report.

The macOS artifact is an unsigned development build, not a notarized installer.
