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

## macOS: downloaded package blocked

The unsigned package can be blocked with a message that Apple could not verify
it is free of malware. This was observed on macOS 26.6.2 after downloading the
GitHub artifact with Firefox; the extracted PKG retained quarantine metadata.

For the SAWSTAR package downloaded from this repository, first check its SHA256
against the matching published checksum. If you trust this download, dismiss the
warning with Done, open System Settings > Privacy & Security, and use Open Anyway
for this specific SAWSTAR package, if offered. Confirm the named package and
complete macOS authentication yourself. Then continue the installer.
Do not disable Gatekeeper globally. A damaged-file or malware-detected warning
should be investigated rather than treated as an ordinary unsigned-app warning.
If no per-package option is offered, report the exact macOS version and message.

Apple's instructions: https://support.apple.com/en-gb/102445

### Magyar

A letöltött, aláírás nélküli PKG-t a macOS blokkolhatja: az Apple nem tudja
ellenőrizni, hogy mentes-e a rosszindulatú szoftverektől. Ellenőrizd a csomag
SHA256 összegét a hozzá tartozó közzétett ellenőrzőösszeggel. Ha a hivatalos
SAWSTAR-letöltésben megbízol: Kész > Rendszerbeállítások > Adatvédelem és
biztonság > Megnyitás mindenképpen, amennyiben ez a SAWSTAR csomagnál elérhető.
A megerősítést és a jelszó/Touch ID hitelesítést a saját gépeden végezd el.
Ne kapcsold ki a rendszer egészének biztonsági védelmét. Sérült fájl vagy
észlelt kártevő üzeneténél ne ezt az eljárást kövesd; jelezd a pontos hibát.

## macOS manual uninstall / Kézi eltávolítás

Quit all DAWs before removing SAWSTAR. In Finder, use **Go → Go to Folder**
and remove `/Library/Audio/Plug-Ins/VST3/SAWSTAR.vst3` (administrator approval
may be required). If you installed a per-user copy, also check
`~/Library/Audio/Plug-Ins/VST3/SAWSTAR.vst3`. Restart the DAW and rescan plugins.
Optional installed documentation is in
`/Library/Application Support/SAWSTAR/Documentation`.

Your sounds are separate: keep `~/Library/Application Support/SAWSTAR/Presets`
unless you explicitly want to delete your user presets. Back them up first.

Zárj be minden DAW-ot. A Finder **Ugrás → Ugrás mappához** menüjével nyisd meg
a fenti VST3 mappát, és töröld a SAWSTAR.vst3 csomagot. A rendszerszintű példány
eltávolítása rendszergazdai jóváhagyást kérhet. Ellenőrizd a felhasználói VST3
mappát is, majd indítsd újra a DAW-ot és frissítsd a pluginlistát.
A saját preseteket tartalmazó Presets mappát őrizd meg; törlés előtt mentsd el.
