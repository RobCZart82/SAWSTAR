# Dependency and license review

Reviewed 2026-09-06. Scope: pinned upstream core sources and the two selected
DSP translation units, not an as-yet-unbuilt plugin distribution.

| Component | Exact commit | License | Foundation use |
| --------- | ------------ | ------- | -------------- |
| iPlug2 | `d54f69050f517e43b941d88c2a170f0a840b9ee4` | zlib-style; individual dependencies retain licenses | Pinned submodule; not compiled yet |
| DaisySP | `599511b740f8f3a9b8db72a0642aa45b8a23c3a3` | MIT core | Optional oscillator + ADSR integration check |
| DaisySP-LGPL | Not initialized or linked | LGPL extension | Excluded |
| VST3 SDK | Not pinned/included yet | Current upstream root is MIT | Deferred to plugin integration |

## Compatibility decision

Use MIT for original SAWSTAR code/documentation. The reviewed iPlug2 zlib-style
and selected DaisySP MIT sources can be combined with it while retaining their
own attribution/notices. This is not a relicensing of third-party code.
iPlug2 requires preservation of its source notice, truthful origin and marking
modified upstream sources. DaisySP requires its copyright and permission notice
in copies or substantial portions. Keep upstream licenses in submodule checkouts
and copies under `third_party/licenses` for source/binary notice assembly.

DaisySP's root license also contains Plaits and Soundpipe MIT attributions.
The inspected oscillator.h, adsr.h and Utility/dsp.h carry MIT-style notices;
oscillator.cpp and adsr.cpp fall under the core license. The CMake check compiles
only those two translation units. DaisySP's separate LGPL submodule is not
fetched or linked, and `USE_DAISYSP_LGPL` must not be defined.

The iPlug2 root lists additional WDL, NanoVG, NanoSVG, MetalNanoVG, RTMidi,
RtAudio and optional Skia licenses. Their actual linked files, platform SDKs,
fonts and images must be inventoried when choosing the plugin backend. No
blanket approval of every optional iPlug2 dependency is implied.

The current Steinberg VST3 SDK root license is MIT, but the plugin build must
pin and review the actual SDK revision and submodules it consumes. Older SDK
versions may have different terms. No SDK, trademark/logo permission or
signing credential is supplied by this repository.

## Primary sources

- [iPlug2 pinned license](https://github.com/iPlug2/iPlug2/blob/d54f69050f517e43b941d88c2a170f0a840b9ee4/LICENSE.txt)
- [DaisySP pinned license](https://github.com/electro-smith/DaisySP/blob/599511b740f8f3a9b8db72a0642aa45b8a23c3a3/LICENSE)
- [DaisySP optional submodule declaration](https://github.com/electro-smith/DaisySP/blob/599511b740f8f3a9b8db72a0642aa45b8a23c3a3/.gitmodules)
- [VST3 SDK current license](https://github.com/steinbergmedia/vst3sdk/blob/master/LICENSE.txt)
- [iPlug2 pinned CMake documentation](https://github.com/iPlug2/iPlug2/blob/d54f69050f517e43b941d88c2a170f0a840b9ee4/Documentation/cmake.md)

## Dependency changes

Change submodule gitlinks and this table together in a reviewed commit. Examine
license and source diffs, run both platform workflows, and update bundled
notices. Initialize named submodules only; avoid `--recursive` for DaisySP.
