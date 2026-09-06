# Building and dependency setup

## Foundation

CMake 3.21+, C++17 and Git are required. Use the commands in the root README.
CMake builds a static parameter library and CTest contract checks. The default
build has no third-party dependencies or network fetches.

On macOS use an installed Xcode/Command Line Tools toolchain with its license
accepted by the developer. Install CMake from its official distribution or a
package manager. On Windows install Visual Studio 2022's Desktop development
with C++ workload and CMake; use `-G "Visual Studio 17 2022" -A x64` if needed.
Use `--config Release`/`-C Release` with multi-configuration generators.

## Selected DSP integration check

Initialize only the top-level DaisySP submodule and enable
`-DSAWSTAR_CHECK_DAISYSP=ON`. CMake compiles `oscillator.cpp` and `adsr.cpp`
explicitly, with `Source`, `Source/Synthesis`, `Source/Control` and
`Source/Utility` include paths. It does not include the omnibus daisysp.h,
run DaisySP's full build, or enable `USE_DAISYSP_LGPL`.

The test checks finite bounded audio, non-silence while gated, release to
silence and approximate 440 Hz output at 44.1, 48 and 96 kHz. This is neither a
voice-manager test nor a host-loading test.

## Plugin shell: next implementation step

1. Initialize the pinned iPlug2 submodule (non-recursively).
2. Select and pin the required VST3 SDK and graphics/font dependencies;
   review their exact licenses and add their notices.
3. Use the pinned iPlug2 `iplug_add_plugin(... FORMATS VST3 ...)` API in a
   separate `src/plugin/CMakeLists.txt`; do not build all upstream examples.
4. Finalize the identity candidates in ARCHITECTURE.md, provide `config.h`,
   platform resources and a silent stereo instrument adapter.
5. Implement three functioning navigation tabs using iPlug2 IGraphics and
   connect the five parameter definitions. Match the final reference when it
   is supplied. Include resource/font licenses before embedding them.
6. Add explicit `SAWSTAR-vst3` builds and artifact packaging to CI, then verify
   the bundle in REAPER. CI currently builds only the foundation and DSP check.
7. Connect the custom engine, sample-accurate MIDI and state codec.

Do not run upstream download scripts blindly: the exact SDK version and
transitive dependency set must be reproducible. There is intentionally no
pretend plugin build option or uncompiled plugin skeleton in this foundation.

## CI scope

`build-macos.yml` runs on macos-14, `build-windows.yml` on windows-2022.
Both build Debug and Release, initialize only DaisySP, enable DSP checks and
run CTest. They run for pushes, pull requests and manual dispatch, use read-only
repository permissions, and never sign, release, install or publish plugins.
