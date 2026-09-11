# Installing SAWSTAR candidates

The first public release is planned as 1.0.0. Current artifacts are pre-release
candidates. Back up projects and user presets and close the DAW before updating.
Do not treat a renamed candidate package as the final release.

## Windows

Choose the installer matching your DAW: x64 or native ARM64. Run Setup.exe;
administrator rights are required for the common VST3 directory:
`C:\Program Files\Common Files\VST3\SAWSTAR.vst3`.
The installer adds documentation and an uninstaller under Program Files and
manual links in the Start Menu. It does not modify `%APPDATA%\SAWSTAR\Presets`.
The two architectures use separate installed-document/uninstaller locations.

You can alternatively copy the entire SAWSTAR.vst3 bundle from the ZIP to the
same VST3 directory. A VST3 is not a standalone EXE: Setup.exe is its installer.
The candidates are not Authenticode signed; final distribution policy is pending.

## macOS

The Universal candidate contains Intel and Apple Silicon code. Open the DMG
and run the PKG. It installs system-wide to
`/Library/Audio/Plug-Ins/VST3/SAWSTAR.vst3`, with documentation under
`/Library/Application Support/SAWSTAR/Documentation`.

Earlier manual builds may be in `~/Library/Audio/Plug-Ins/VST3/SAWSTAR.vst3`.
Before testing the system-wide installer, move that older user-local bundle to
a backup outside both VST3 search directories to avoid loading a duplicate.
The installer does not delete the user's old bundle or user presets.

Manual ZIP installation can still use `~/Library/Audio/Plug-Ins/VST3/`.
Current automated packages are unsigned/not notarized, apart from any ad-hoc
linker signature in the binary. They are not represented as Gatekeeper-approved.
Developer ID signing and notarization require the developer's credentials; the
packaging script supports that separate operation. No security settings are
changed by the installer.

## After installation

Restart the DAW and rescan plugins if necessary. Insert SAWSTAR as a VST3
instrument, check its About version/build ID, and try an embedded factory sound.
Retain the previous package until project recall and basic playing are confirmed.
User library: macOS `~/Library/Application Support/SAWSTAR/Presets`; Windows
`%APPDATA%\SAWSTAR\Presets`. Factory presets are built in.

See SYSTEM_REQUIREMENTS.md for target versus tested OS support. Candidate PDFs
are still the development edition and may illustrate an earlier GUI build.
