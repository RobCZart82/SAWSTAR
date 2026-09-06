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

## VST3 shell

The silent shell is implemented. See [PLUGIN_SHELL.md](PLUGIN_SHELL.md) for
bootstrap, build, identity and REAPER instructions. `scripts/bootstrap-plugin.py`
fetches only the pinned SDK components and recognizes iPlug2's instruction-only
SDK placeholder. It refuses to overwrite an unmanaged SDK directory.

The bootstrap replaces the tracked instruction README inside the upstream
SDK slot, so Git may report the iPlug2 submodule as locally modified after SDK
installation. No iPlug2 library source is patched and the gitlink is unchanged.

## CI scope

`build-macos.yml` runs on macos-14, `build-windows.yml` on windows-2022.
Both build Debug and Release, initialize only DaisySP, enable DSP checks and
run CTest. They run for pushes, pull requests and manual dispatch, use read-only
repository permissions, and never sign, release, install or publish plugins.

Each platform also has a Release VST3 job: pinned SDK bootstrap, real bundle
build, Steinberg validator and packaged development artifact with notices.
Plugin validation additionally requires CMake 3.25+ for SDK 3.8.1. These tests
exercise the factory, processing and framework state; they do not inspect the
GUI or replace the manual REAPER check. Artifact architectures are macOS ARM64
and Windows x64, matching the selected runners.
