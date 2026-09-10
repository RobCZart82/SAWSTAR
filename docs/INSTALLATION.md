# Development installation and release preparation

No public version tag is published yet. Current build pipelines produce a
macOS Apple Silicon VST3 bundle and a Windows x64 VST3 bundle. Other formats
and architectures are not claimed by these packages.

Close the DAW before replacing an installed plugin and retain the previous
bundle. Extract the package and copy the entire `SAWSTAR.vst3` bundle:

- macOS, current user: `~/Library/Audio/Plug-Ins/VST3/`
- Windows: `C:\Program Files\Common Files\VST3\`

Restart the DAW and rescan if necessary. Load SAWSTAR as an instrument and
check its About build ID: an already-running host can retain the old binary.
Keep test projects separate from music projects. Factory presets are embedded;
user `.sawstar` files can be imported together from PRESETS → Import.

User library locations are `~/Library/Application Support/SAWSTAR/Presets` on
macOS and `%APPDATA%\SAWSTAR\Presets` on Windows. Back these up before migration.

Before public publication: finish the acceptance checklist, decide supported
minimum OS versions, resolve signing/notarization and packaging policy, verify
all bundled notices/fonts, attach checksums and release notes, and test a clean
installation. This document does not claim packages are signed or notarized.

Development header dates are fixed at build configuration. A public release
uses CMake `SAWSTAR_RELEASE_DATE` in `YYYY.MM.DD.` form. Version, tag, date,
About information and release notes must agree before publishing.
