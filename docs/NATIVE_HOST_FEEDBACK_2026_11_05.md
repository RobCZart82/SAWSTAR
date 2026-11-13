# Windows/macOS működési visszajelzés — 2026-11-05

**RobCZart82 a legutóbb linkelt plugin működését Windows és macOS alatt egyaránt
hibamentesnek jelezte.** Ezzel mindkét platformhoz van pozitív felhasználói
működési visszajelzés; a részletes mérési és kiadási kapuk külön követendők.

## Visszajelzés és buildkapcsolat

RobCZart82 közlése: „Tökéletesen működik a plugin windows-on és macos-en is”.

A beszélgetésben közvetlenül előtte linkelt, 2026-11-04-i sikeres main build
forrása: `0521387689e0387d16a78d2667f88317c3539139`.

| Platform | Linkelt artifact | Futás |
| --- | --- | --- |
| macOS Universal (arm64 + x86_64 szelet) | [SAWSTAR-macos-universal-candidate / 13308426768](https://github.com/RobCZart82/SAWSTAR/actions/runs/41801813953/artifacts/13308426768) | [41801813953](https://github.com/RobCZart82/SAWSTAR/actions/runs/41801813953) |
| Windows x64 | [SAWSTAR-windows-x64-candidate / 13308252584](https://github.com/RobCZart82/SAWSTAR/actions/runs/41801813847/artifacts/13308252584) | [41801813847](https://github.com/RobCZart82/SAWSTAR/actions/runs/41801813847) |

Ez a visszajelzés beszélgetésbeli buildkapcsolata. A telepített executable
hashait ezen alkalommal nem olvastuk vissza, és RobCZart82 nem adott külön
About-/manifest-azonosítást. Nem állítunk ennél erősebb helyi binárisazonosságot.
A táblázat artifactjai az általános SAWSTAR main build csomagjai; a
külön Premium Preview/Candidate és Premium production RC buildjeinek
elfogadása nem másolható át automatikusan ezekből.

## Rögzíthető eredmény

- Windows: RobCZart82 sikeres működést jelzett.
- macOS: RobCZart82 sikeres működést jelzett.
- A közölt használat során nem jelzett hibát.

A pontos gép/OS/hostverzió és folyamatarchitektúra, audioeszköz/driver,
ráta/puffer, presetlista, projekt, MIDI, időtartam és számszerű CPU/RT/xrun
adatok nem kerültek külön megadásra. Nem töltünk ki helyettük feltételezett
értéket vagy nulla hibaszámlálót. A Universal fájl két szelete nem bizonyít
két külön natív architektúrán elvégzett hostpróbát.

Ez a közlés nem esetenkénti igazolás a teljes C/V/G/F mátrixhoz, GUI/state/
preset-életciklushoz vagy minden új factory hang zenei elfogadásához.
A korábbi [macOS Reference/Candidate alappróba](NATIVE_HOST_SMOKE_2026_10_27.md)
önálló, más forrású megfigyelés marad.

## Következő ellenőrzés

A konkrét kiadásjelölt azonosítása után a még hiányzó esetekre szükséges
célzott projektmentés/visszatöltés, preset-/GUI-próba és a kijelölt CPU-/
dropout-mérés **(sajátgépes reaper)**. A már közölt pozitív használati
visszajelzést nem kell újra elmondani vagy hibásnak tekinteni a hiányzó
részletes adatlap miatt.

K1 oka továbbra ismeretlen; ez a felhasználói hang-/működési próba nem
numerikus okfeltárás. A visszajelzés önmagában nem Premium-default váltás,
teljes RC-elfogadás vagy publikálási engedély.
