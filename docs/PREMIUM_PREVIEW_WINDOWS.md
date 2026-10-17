# Premium Reference/Candidate — Windows x64 REAPER tesztcsomag

Állapot: **2026-10-18, implementált build-/csomagolási út; a friss CI és a
célgépes telepítési/REAPER-próba külön ellenőrzendő**. Ez nem nyilvános release.
A production SAWSTAR Classic marad. A hangerőkorrekció lezárt, a 0,44 dB
RMS-különbség marad; a CPU-/natív elfogadás továbbra nyitott.

## Melyik két hangszer?

| Szerep | REAPER FX-név | Teljes bundle mappa | Egyedi ID |
| --- | --- | --- | --- |
| Reference | SAWSTAR Premium Preview | `SAWSTARPremiumPreview.vst3` | `SwPP` |
| Candidate | SAWSTAR Premium Candidate | `SAWSTARPremiumCandidate.vst3` | `SwPC` |

Mindkettő ugyanazt a kalibrált Premium filtert, paramétereket, factory trimeket
és 32 mintás latencyt használja. A Candidate-ben az OSC1/OSC2 Saw-dispatch
optimalizációja a különbség. A meglévő Classic `SAWSTAR.vst3` / `SwSt` marad.
Ez a csomag **Windows x64**; nem Windows ARM64- vagy 32 bites tesztbuild.

## Letöltés és telepítési smoke

1. A `Windows Premium Preview pair` workflow **azonos forráscommitú, két
   sikeres jobjából** töltsd le az artifactokat:
   `SAWSTAR-Premium-Preview-Windows-x64` és
   `SAWSTAR-Premium-Candidate-Windows-x64`.
   GitHub-bejelentkezés szükséges lehet. Hibás/részben kész futás nem elfogadott pár.
2. Bontsd ki az artifactot, majd ellenőrizd a benne lévő ZIP-et a `.sha256`
   fájllal. PowerShell: `Get-FileHash .\SAWSTAR-Premium-Candidate-Windows-x64.zip -Algorithm SHA256`.
   A két `PACKAGE-MANIFEST.json` `source_commit` mezője egyezzen; a `pair_role`,
   `plugin_unique_id`, `vst3_class_id`, `binary_sha256` külön azonosítsa a szerepet.
3. Zárd be a REAPER-t. A két ZIP-ből a **teljes külső `.vst3` mappát** másold
   a REAPER által vizsgált VST3-helyre, például `C:\Program Files\Common Files\VST3\`.
   Ehhez rendszergazdai fájlmásolás szükséges lehet. A belső
   `Contents\x86_64-win\*.vst3` fájlt ne emeld ki a bundle-ből.
4. REAPER x64 alatt szükség esetén Preferences → Plug-ins → VST → Re-scan.
   A két fenti név külön jelenjen meg a Classic mellett. About-ban ellenőrizd
   a forrásazonosítót és a `-premium-preview` / `-premium-candidate` utótagot.
5. Töltsd be ugyanazt a `.sawstar` hangszínt, MIDI-t és cutoff-automatizálást.
   Ellenőrizd a hangot, vezérlőket, 75/100/125% GUI-projektmentést/újranyitást,
   két független példányt és a külön presetmappákat:
   `%APPDATA%\SAWSTAR Premium Preview\Presets` és
   `%APPDATA%\SAWSTAR Premium Candidate\Presets`.
6. A fő CPU A/B-próbában egyszerre csak egy hangszer legyen online; a másik
   legyen offline. A két hangszer egyszerre szólaltatása külön tartalékteszt.
   A részletes [REAPER-terv](NEXT_RELEASE_DEVELOPMENT_PLAN.md) és a
   [kitöltendő natív adatlap](NATIVE_REAPER_CPU_ACCEPTANCE.md) szerint rögzítsd az eredményt.

Az eltávolításhoz REAPER bezárása után csak a két külön tesztbundle-t kell
eltávolítani. Nincs installer, automatikus telepítés vagy Classic-felülírás.
A korábban lezárt célgépes offline kampányt nem kell újraindítani.

## Mit bizonyít a CI?

A két job ugyanazt a pontos forráscommitot, pinned iPlug2/DaisySP/SDK-t és
Release/x64 buildpolicy-t használja. Lefut a saját motor identity/audio/latency
smoke-ja, a teljes 480 esetes kalibrált motorpár, a PE-architektúra/statikus
runtime audit és a tényleges VST3 Steinberg-validatortesztsora.

A validator után a pinned SDK `moduleinfotool` **a tényleges bináris factory-jából**
olvassa ki a nevet és CID-t. A csomagoló az ismert Reference/Candidate CID-re,
névre és kategóriára ellenőriz; a mappa átnevezése nem elegendő.
A validator előtti/utáni binárishash egyezése és az eredetjegyzék a pontos
forráshoz és moduleinfo-hashhoz köti az ellenőrzést. A nyers SDK-moduleinfo és
az eredetjegyzék a ZIP `evidence/` mappájában marad; a workflow külön megőrzi
az identity-, numerikus-, runtime- és validatornaplókat hiba esetén is.
A pinned SDK JSON5-ának strukturális záróvesszőit csak olvasáskor kezeli a parser;
a megőrzött eredeti fájl byte-jait és hashát nem módosítja.

A csomagok `native_host_acceptance=false` és
`production_promotion_allowed=false` értékkel készülnek.
A felhős Windows CI nem Robert célgépének REAPER-/ASIO-/dropout-eredménye.
A jelen fejlesztési környezet Linux: helyi Windows VST3-futtatást nem állítunk.

## Első Windows csomagolási hiba — 2026-10-18

A [39076294186 futás](https://github.com/RobCZart82/SAWSTAR/actions/runs/39076294186)
`2eca85fae727b9d35769bfb2855bbde7644cf49e` headjén mindkét saját smoke,
480 esetes motorpár, runtime audit és 47/47 validator sikeres volt.
Mindkét csomagolás a szigorú factory-azonossági kapunál megállt: az SDK
külső `Name` mezője üres volt, miközben a `Classes[0].Name`, CID,
kategória és vendor a helyes külön Reference/Candidate identitás maradt.

A pinned public.sdk `586dc5e6c8012c3e4b01c79389375cbe96bdb1da`
`source/vst/hosting/module_win32.cpp` `Module::create` függvénye csak `/`
szeparátornál állítja be a modul nevét. A hívó `str(bundle)` Windows alatt
backslash útvonalat adott. Javítás: kizárólag a `moduleinfotool -path`
argumentuma `bundle.as_posix()`; a bináris, forrás, SDK és metadatahashok,
a név-/CID-/vendor-/kategória-kapuk nem változnak. Nem fogadunk el üres nevet,
nem írjuk át a korábbi metadatafájlt vagy annak hashát.

Megőrzött negatív evidence artifactok:

| Szerep | Artifact | Artifact ZIP SHA-256 | Eredeti moduleinfo SHA-256 |
| --- | --- | --- | --- |
| Candidate | [12117127563](https://github.com/RobCZart82/SAWSTAR/actions/runs/39076294186/artifacts/12117127563) | `ab4f1a17fcdc3930fcef38238976f10c763be8347266df09501d717e7a30fe0d` | `387916ca2f1aab5b1d569e7a45343cb1ba6d0525a50d46c9d0d93a41d1d41560` |
| Reference | [12116993594](https://github.com/RobCZart82/SAWSTAR/actions/runs/39076294186/artifacts/12116993594) | `0e4494696000bbc9fb2824029703db67b4510b875b8b844c29d78b1b78184cc3` | `4b187bcff6fa0ac3e577c217c5e8066139f22f1905d46aade671167c37ecc313` |

A javított head friss CI-je és tényleges sikeres Windows csomagolása még kapu;
a régi validator-siker önmagában nem elegendő. Nincs célgépes CPU-újramérés.
