# Installing SAWSTAR 1.0.1 / Telepítés

Download [SAWSTAR 1.0.1](https://github.com/RobCZart82/SAWSTAR/releases/tag/v1.0.1).
Recommended: the matching installer. Optional: a manual-install ZIP.
Back up projects and user presets and close the DAW before updating.

This online guide includes corrections made after release. The documents inside
the original 1.0.0 downloads may contain older candidate wording; the released
binaries and archive checksums have not been changed by these documentation updates.

A [SAWSTAR 1.0.1 letöltése](https://github.com/RobCZart82/SAWSTAR/releases/tag/v1.0.1):
elsősorban a megfelelő telepítőt válaszd; kézi telepítéshez ZIP is elérhető.
Frissítés előtt zárd be a DAW-ot, és mentsd a projektjeidet, saját presetjeidet.
Ez az online útmutató a kiadás utáni pontosításokat is tartalmazza. Az eredeti
csomagok útmutatójában még lehet korábbi kiadásjelölt-szöveg; a kiadott binárisok
és csomagok ellenőrzőösszegei változatlanok.

## Windows

Choose the installer matching your DAW: x64 or native ARM64. Run Setup.exe;
administrator rights are required for the common VST3 directory:
`C:\Program Files\Common Files\VST3\SAWSTAR.vst3`.
The installer adds documentation and an uninstaller under Program Files and
manual links in the Start Menu. It does not modify `%APPDATA%\SAWSTAR\Presets`.
The two architectures use separate installed-document/uninstaller locations.

You can alternatively copy the entire SAWSTAR.vst3 bundle from the ZIP to the
same VST3 directory. A VST3 is not a standalone EXE: Setup.exe is its installer.
The installers are not Authenticode signed; Windows may display a security warning.

## macOS

The Universal package contains Intel and Apple Silicon code. Open the DMG
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

See [System requirements](SYSTEM_REQUIREMENTS.md) for target versus tested OS support.
The 1.0.1 PDFs illustrate the real GUI captured from the preview.

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

## macOS Terminal troubleshooting / Haladó hibaelhárítás

### English

Use the graphical approval steps above first. These are optional troubleshooting
commands for an **already installed** SAWSTAR bundle, not steps to run in order.
Close the DAW. Check the downloaded archive against SHA256SUMS.txt on the official
release page before making changes. Quarantine removal bypasses the quarantine
check for this bundle; it does not prove the download is safe or notarize it.
Do not use it for a malware-detected warning or an unexplained damaged download.

**Remove only the quarantine attribute**, when a trusted installed copy is
blocked by quarantine and the graphical approval option is unavailable:

```bash
sudo xattr -dr com.apple.quarantine "/Library/Audio/Plug-Ins/VST3/SAWSTAR.vst3"
```

For a manual installation in your own user folder, use this alternative instead
(no sudo normally needed):

```bash
xattr -dr com.apple.quarantine "$HOME/Library/Audio/Plug-Ins/VST3/SAWSTAR.vst3"
```

Use only the path where you actually installed the plugin. `-d` deletes the
named attribute; `-r` applies recursively inside that bundle. A missing-attribute
message means quarantine may already be absent. Then restart the DAW and rescan.
This does not unblock a PKG that has not yet been installed.

**Check the existing code signature**, without modifying the plugin:

```bash
codesign --verify --strict --verbose=2 "/Library/Audio/Plug-Ins/VST3/SAWSTAR.vst3"
```

For a user-local copy, replace the path with
`"$HOME/Library/Audio/Plug-Ins/VST3/SAWSTAR.vst3"`.
Success only verifies the existing signature; an ad hoc signature is not an
Apple Developer ID signature or notarization. If verification fails, report the
exact output and macOS version; reinstall the official copy before further repair.

We do not prescribe `xattr -cr`: it removes all extended attributes rather than
only quarantine. Nor is `codesign --force --sign -` a routine installation step:
it replaces the existing signature with a local ad hoc signature and changes the
installed bundle. These are not required follow-up commands.

### Magyar

Elsőként a fenti grafikus jóváhagyást használd. Az alábbiak opcionális
hibaelhárítási lehetőségek a **már telepített** pluginhoz, nem egymás után
kötelezően futtatandó lépések. Zárd be a DAW-ot, és ellenőrizd a letöltés SHA256
összegét a hivatalos release SHA256SUMS.txt fájljával. A karanténjelölés törlése
ennél a csomagnál megkerüli a karanténellenőrzést; nem igazolja a biztonságát,
és nem jelent Apple-notarizációt. Kártevőészlelésnél vagy tisztázatlan sérülésnél
ne alkalmazd ezt megoldásként.

**Csak a karanténjelölés eltávolítása**, ha a megbízható telepített példányt
karantén blokkolja, és a grafikus jóváhagyás nem elérhető:

```bash
sudo xattr -dr com.apple.quarantine "/Library/Audio/Plug-Ins/VST3/SAWSTAR.vst3"
```

Saját felhasználói mappába végzett kézi telepítésnél helyette ezt használd
(általában sudo nélkül):

```bash
xattr -dr com.apple.quarantine "$HOME/Library/Audio/Plug-Ins/VST3/SAWSTAR.vst3"
```

Csak a tényleges telepítési helyre vonatkozó parancsot futtasd. A `-d` a
megnevezett attribútumot törli, az `-r` a csomagon belül rekurzívan alkalmazza.
A hiányzó attribútumról szóló üzenet azt is jelentheti, hogy már nincs karantén.
Ezután indítsd újra a DAW-ot, és frissítsd a pluginlistát. Ez nem a még nem
telepített PKG feloldására szolgál.

**A meglévő aláírás ellenőrzése**, módosítás nélkül:

```bash
codesign --verify --strict --verbose=2 "/Library/Audio/Plug-Ins/VST3/SAWSTAR.vst3"
```

Felhasználói telepítésnél az útvonal legyen
`"$HOME/Library/Audio/Plug-Ins/VST3/SAWSTAR.vst3"`.
A sikeres ellenőrzés nem jelent Developer ID-aláírást vagy notarizációt.
Hibánál őrizd meg a pontos üzenetet és a macOS verzióját; további javítás előtt
telepítsd újra a hivatalos példányt.

Az `xattr -cr` nem ajánlott általános lépés: minden kiterjesztett attribútumot
törölne. A `codesign --force --sign -` helyi ad hoc aláírásra cseréli a meglévőt,
és módosítja a telepített csomagot; ez sem szükséges következő telepítési lépés.
