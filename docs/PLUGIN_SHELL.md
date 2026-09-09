# SAWSTAR First Sound development build

Playable 16-voice instrument with SAWSTAR 7-Saw and DaisySP ADSR primitives.
See FILTER.md for the resonant low-pass and SEVEN_SAW.md for Detune, Mix and Width behavior.
MAIN, ADVANCED and PRESETS provide the modular sound-design, performance,
effects and preset-library pages. All pages share a keyboard spanning MIDI 36–96,
without an octave selector; host MIDI can address notes 0–127.
The SUB offers Sine, Triangle and Square, with a 10 ms waveform crossfade.
User sounds are saved as individual `.sawstar` files, with multi-file import and
exclusive creation to protect concurrent saves. Versioned state reads older
saves; old plugin binaries cannot read newer features. Plugin identity and the
existing parameter order are unchanged.
See [AUDIT_FIXES.md](AUDIT_FIXES.md) for the September 10 corrections.

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
three tabs, adjust controls on each page and load Init. Save/reopen a disposable
project and compare parameter values; test two instances and repeated editor
open/close. Play individual notes and chords from both GUI and MIDI; check note-off, sustain,
voice stealing and silence after release. Confirm finite bounded output.
This checklist is not evidence of a completed REAPER test; see the task report.

The macOS artifact is an unsigned development build, not a notarized installer.
