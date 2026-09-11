# Building SAWSTAR

Use CMake 3.25+, Python 3, Git and a C++17 toolchain. The default build runs
foundation tests without downloading plugin dependencies:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

For the instrument, run `python3 scripts/bootstrap-plugin.py` first. This sets
up the pinned iPlug2, DaisySP and VST3 SDK dependencies. The iPlug2 submodule
may appear modified because SDK placeholder files are replaced during setup.

## macOS Universal

Use Xcode with its SDK and developer-accepted license. Build both slices:

```sh
cmake -S . -B build-mac -DSAWSTAR_BUILD_PLUGIN=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build-mac --config Release
```

`release.json` defines the default minimum macOS target (11.0). Both slices
must be present in distribution bundles. Intel execution under Rosetta does
not replace testing on an Intel Mac. Packaging supports unsigned PKG/DMG
files; signing and notarization require separately supplied Apple credentials.

## Windows x64 and ARM64

Install Visual Studio 2022 C++ desktop tools, CMake and the toolchain for the
selected architecture. Use separate build directories:

```powershell
cmake -S . -B build-x64 -G "Visual Studio 17 2022" -A x64 -DSAWSTAR_BUILD_PLUGIN=ON
cmake --build build-x64 --config Release
cmake -S . -B build-arm64 -G "Visual Studio 17 2022" -A ARM64 -DSAWSTAR_BUILD_PLUGIN=ON
cmake --build build-arm64 --config Release
```

The plugin uses the static MSVC runtime and OpenGL renderer. End users do not
need Visual Studio, CMake or the SDK. Inno Setup 6 builds the Windows installers.

## Release metadata and CI

Edit `release.json`, then run `python3 scripts/sync-release-metadata.py`.
The generated literals in config.h and Info.plist are required by the plugin
framework. `--check` detects stale generated metadata. Set `candidate` to an
empty string for final package filenames; this does not publish a release.
CMake and packaging scripts read the central metadata directly.

CI builds Debug/Release checks and Release VST3 bundles, validates plugins,
packages installers and exercises installation/uninstallation on disposable
runners. Windows x64 and ARM64 have separate jobs; macOS builds Universal.
These checks do not replace manual GUI or dual-architecture coexistence QA.
