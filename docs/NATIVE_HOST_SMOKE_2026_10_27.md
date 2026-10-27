# macOS Reference/Candidate — felhasználói alappróba, 2026-10-27

**Eredmény: Robert mindkét telepített tesztplugin működését és megszólalását
hibamentesnek jelezte.** Ez felhasználói betöltési/hangpróba; a teljes natív
CPU-/dropout-elfogadás még nyitott.

## Megfigyelés és eredet

Robert 2026-10-27-i visszajelzése ebben a fejlesztési beszélgetésben:
„Mindkét plugin jól működik, szól ahogy kell, hibát nem találtam”.

A visszajelzés az itt korábban linkelt és telepített macOS Universal
Reference/Candidate tesztpárhoz kapcsolódik:

| Szerep | Név / plugin-ID | Letöltési artifact |
| --- | --- | --- |
| Reference | SAWSTAR Premium Preview / `SwPP` | [12354679231](https://github.com/RobCZart82/SAWSTAR/actions/runs/39622967441/artifacts/12354679231) |
| Candidate | SAWSTAR Premium Candidate / `SwPC` | [12355443166](https://github.com/RobCZart82/SAWSTAR/actions/runs/39622967441/artifacts/12355443166) |

A csomagok forrása `1b6b760aef266e1f7a74745457850063d7938fd0` (#129 head).
A 2026-10-22-i helyi telepítésellenőrzés mindkét bundle-ben ezt a rövid
verzióazonosítót, helyes külön plist-identitást, arm64/x86_64 szeletet és
sikeres macOS codesign-ellenőrzést talált. A 2026-10-27-i csak olvasó kontroll
szerint a két telepített executable hash-a azóta is egyezik:

| Bundle a `/Library/Audio/Plug-Ins/VST3/` alatt | Executable SHA-256 |
| --- | --- |
| `SAWSTARPremiumPreview.vst3` | `7bbddd79f0134809f17fbe054d9a298342fdca22b2548d301614a968a24d73a4` |
| `SAWSTARPremiumCandidate.vst3` | `004fca98166c47f4f511a15a86171f4ca8e0f503635d16cb4950f85c763d732c` |

Az alappróbát **Robert végezte**, nem automatizált REAPER-futtatás.
A korábbi automatizált indítás sikertelen volt (exit 134 / LaunchServices
-10827); az nem hang- vagy CPU-eredmény, és nem írja felül Robert visszajelzését.

## Az eredmény hatóköre

| Vizsgálat | Állapot |
| --- | --- |
| Mindkét hangszer működik és megszólal | Robert sikeresnek jelezte, észlelt hiba nélkül. |
| Telepített fájlok azonossága a korábbi helyi ellenőrzéshez | 2026-10-27-én a két executable SHA-256 egyezett. |
| Tényleges REAPER-folyamatarchitektúra, aktuális verzió, audioeszköz, ráta/puffer | Ehhez a próbához nincs külön rögzített adat. A korábbi telepítésellenőrzés környezetét nem másoljuk át mérési környezetként. |
| 28 eset / 112 mérési ablak, CPU/RT/xrun számszerű értékei | Nincs rögzített eredmény; nem nulla és nem teljesített kapu. |
| Presetváltás, editor reopen, GUI 75/100/125%, projekt-state | Nincs külön, esetenként megerősített eredmény. |
| Windows x64 / natív Intel hostpróba | Ez a visszajelzés nem ezek eredménye. |
| K1, production-integráció, végleges RC/publikálás | Változatlanul külön nyitott feladat. |

A „hibát nem találtam” e próba megfigyelése, nem minden terhelésre vagy
platformra szóló hibamentességi állítás. A globális
`native_host_acceptance` és `production_promotion_allowed` nem válik true-vá.

## Következő lépés (sajátgépes reaper)

Azonos mentett P0 állapot/MIDI/cutoff-automatizálás mellett rögzítjük a tényleges
hostarchitektúrát és audio-konfigurációt, majd a C /48 kHz /128 AUTO négy
filtermódjának AB/BA előszűrését végezzük el. Egy ablak 10 s bemelegítés +
60 s megfigyelés; egyszerre egy hangszer online. Minden eredmény megmarad.
A teljes mátrix, /64 kontroll és kiegészítő próbák ezután következnek.

Irányadó: [natív adatlap](NATIVE_REAPER_CPU_ACCEPTANCE.md),
[kiadási terv](NEXT_RELEASE_DEVELOPMENT_PLAN.md),
[MIDI-bemenetek](NATIVE_HOST_MIDI_INPUTS.md).
