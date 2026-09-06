# Dependency and license review

Reviewed 2026-09-06. Scope: pinned upstream core sources and the two selected
DSP translation units, including the development shell dependency selection documented below.

| Component | Exact commit | License | Foundation use |
| --------- | ------------ | ------- | -------------- |
| iPlug2 | `d54f69050f517e43b941d88c2a170f0a840b9ee4` | zlib-style; individual dependencies retain licenses | Pinned submodule; compiled by the VST3 shell |
| DaisySP | `599511b740f8f3a9b8db72a0642aa45b8a23c3a3` | MIT core | Optional oscillator + ADSR integration check |
| DaisySP-LGPL | Not initialized or linked | LGPL extension | Excluded |
| VST3 SDK | `3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96` | MIT | Development shell; details below |

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
fonts and images must be inventoried when changing the plugin backend. The shell selection is recorded below. No
blanket approval of every optional iPlug2 dependency is implied.

The Steinberg VST3 SDK revision used by the shell is pinned below; its root
and selected submodule licenses have been reviewed. Older SDK
versions may have different terms. No trademark/logo permission or signing credential is supplied by this repository.

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

## VST3 development shell dependency selection

The shell pins VST3 SDK 3.8.1 at `3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96`.
Its base (`fcf9da0bd27a16f7f03773a3a39822f28f5c8477`), pluginterfaces
(`4f547e8e102b47de4a8b8aaf343c73b700786372`) and public.sdk
(`586dc5e6c8012c3e4b01c79389375cbe96bdb1da`) licenses are MIT and copied
under third_party/licenses. The SDK commit pins the cmake tooling as well.
Bootstrap initializes only these four children, never VSTGUI or SDK examples'
external dependencies.

Graphics: iPlug2's vendored NanoVG + FontStash (zlib), NanoSVG (zlib),
MetalNanoVG (MIT, macOS), glad 0.1.28 (MIT) and Khronos headers (MIT, Windows),
STB helpers (public domain or MIT option). WDL retains its zlib-style notice.
Copies of the applicable notices accompany each development artifact.
All these source revisions are fixed by the existing iPlug2 commit.
No font file is bundled: the UI loads installed Arial via the platform font API.
The SAWSTAR original code remains MIT; optional LGPL DaisySP modules remain excluded.
