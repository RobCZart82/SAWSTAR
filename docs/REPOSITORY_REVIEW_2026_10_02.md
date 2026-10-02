# SAWSTAR repository ellenőrzés és kiadási javaslat

Dátum: 2026-10-02. Ellenőrzött main: `2d3fd73527ecdad27028792d898b37b2d4bf28de`.

Az ellenőrzés a korábbi 1.0.4 előkészítés óta bekerült #36–#39 változtatásokra,
az érintett forrásokra és regressziókra, a main CI eredményeire és a letölthető
1.0.4 draft csomagjaira terjedt ki. Az átnézett változtatásokban nem igazoltunk
új, publikálást blokkoló regressziót. Ez nem teljes natív host-elfogadás és nem
bizonyítja, hogy minden lehetséges állapotkombináció hibamentes.

## Az új fejlesztések értékelése

| Változás | Ellenőrzött eredmény | Korlát |
|---|---|---|
| #36 MIDI és ARP | ARP transport-stop az editor tulajdonlást is tisztítja; editor offset nulla; VST3 MIDI-kontrollerpontok eredeti időzítéssel jutnak tovább. A célzott regressziók sikeresek. | A hagyományos pluginparaméterek automatizálása továbbra is blokkonként a legutolsó pontot használja. A valódi host/editor életciklus natív próbája külön szükséges. |
| #37 Presetimport | A közös módosítási zár alatt egy batch név- és tartalomindexet használ. Kizáró fájllétrehozás, pontos duplikációkezelés és sikertelen mentés utáni folytatás megmarad. | A hívás továbbra is szinkron GUI-művelet; a gyorsulás nem jelent háttérben futó importot. |
| #38 Draft frissítése | A release script a privát draftot frissítheti, a publikus kiadást nem írja felül. A forrás, CI, assetek, checksumok és csomagtartalom ellenőrzése megmarad. | A publikálási dátum és végleges szövegek egyeztetése még kiadási feladat. |
| #39 Kiadási terv | A draft eredete és a natív N01–N16 elfogadási mátrix dokumentált. | A checklist létezése nem azonos minden eset tényleges végrehajtásával. |

A kódolvasás kiterjedt az Arpeggiator, BlockMidiQueue, EngineControls,
SAWSTAR wrapper, iPlug2 MIDI patch, UserPresets és a release-validáció érintett
útvonalaira, valamint a kapcsolódó új tesztekre. A vizsgált új PR-ek nem vezettek
be új oszcillátort vagy szűrőt; a lezárt click/pop kutatásból nem javasoltunk
újabb hangkarakter-módosítást.

## Ellenőrzési bizonyítékok

- Helyi Release foundation/engine build: **70/70 CTest PASS**, DaisySP-ellenőrzéssel.
  Ez a build nem a teljes natív VST3 plugin.
- Main Code Quality: **69/69 ASan/UBSan PASS**, **69/69 coverage PASS**, **5/5 TSan PASS**.
- Coverage: line **91,6%**, function **95,7%**, branch **62,6%**.
  A Linux coverage build nem fedi le ugyanezzel a mélységgel a teljes natív VST3/editort.
- Main [Windows](https://github.com/RobCZart82/SAWSTAR/actions/runs/36996058633),
  [macOS](https://github.com/RobCZart82/SAWSTAR/actions/runs/36996058826) és
  [Quality](https://github.com/RobCZart82/SAWSTAR/actions/runs/36996058619): sikeres.
- Release metadata szinkronellenőrzés: sikeres.
- A lekérdezéskor nincs nyitott PR vagy GitHub Issue. Ez önmagában nem hibamentességi bizonyíték.

## A kiadás jelenlegi állapota

Nyilvános Latest: **v1.0.3**. A **v1.0.4 továbbra is unpublished draft**.
A [draft előkészítő futás](https://github.com/RobCZart82/SAWSTAR/actions/runs/36991725378)
forrása `1a6a5f2fc13d9b85a4f0ad4afcb191e4f84ddf38`.
A jelenlegi main ehhez képest csak #39 dokumentációváltozást tartalmaz.

Mind a nyolc draft asset letöltve és ellenőrizve: hét kiadási fájl és
`SHA256SUMS.txt`. A hét fájl checksumja egyezik. A három manuális ZIP
produkciós csomagvalidációja sikeres, beleértve a pontos forráscommitot,
verziót, fájllistát, manifest-hashokat, kézikönyveket és licenszanyagokat.
A Windows x64/ARM64 csomagban 28–28, a macOS csomagban 30 hash-elt fájl szerepel.
A ZIP-tartalom ellenőrzése nem helyettesíti minden installer natív telepítési próbáját.

## Kiadás előtt elvégzendő munka

1. A friss `1a6a5f2` build natív Windows és macOS elfogadásának rögzítése.
   A korábbi `3225747` RC pozitív felhasználói próbája már ismert, de azóta
   MIDI/ARP és presetimport kód is változott.
2. Csak az itt reprodukált regressziók javítása. Nem szükséges új feature-t
   hozzáadni ahhoz, hogy az 1.0.4 kiadható legyen.
3. Az Unreleased tételek besorolása az 1.0.4 changelogba; README és release notes
   véglegesítése a tagben is helyes, időtálló megfogalmazással.
4. A `2026.10.01.` előkészítési dátum egyeztetése a tényleges publikálási nappal,
   a kézikönyvek és minden verzióadat szinkronban tartásával.
5. Végleges commit CI, ugyanabból a commitból épített draft és checksum-ellenőrzés,
   majd végleges csomagpróba és publikálás. A publikus Latest és letöltések
   ellenőrzése a publikálás után szükséges.

Részletes sorrend:
[fejlesztési terv](DEVELOPMENT_PLAN_POST_1.0.4.md) és
[kiadási checklist](RELEASE_1.0.4_CHECKLIST.md).

## Kiadás után külön kezelendő témák

Ezek megmaradt tervezési vagy minőségi témák, nem ebben az ellenőrzésben
újonnan bizonyított P1 hibák:

- Presetalkalmazás koherenciája: jelenleg 93 paramétergesztus; a host-automatizálással
  való ütközés szabályát és a részleges állapot reprodukcióját kell előbb rögzíteni.
- Hagyományos VST3 paraméterautomatizálás blokkfüggése: több blokk méret mellett
  mérés, majd az elvárt szerződés meghatározása.
- Szinkron import és zárolási várakozás: GUI-késés profilozása Windows/macOS-en,
  több példánnyal; háttérmunkát csak igazolt igény esetén bevezetni.
- Presetátnevezés hard-link hordozhatósága, backup-megőrzés, symlink- és
  könyvtár-fsync szabályok: hibainjektált fájlrendszertesztekkel továbbvinni.
- Wheel reset/publish határ szemantikája: az atomikus memóriabiztonságot és a
  reset előtti/utáni gesztusok értelmezését külön kezelni.
- Queue szélső eseti rendezési költség és teljes paraméterátadás profilozása;
  optimalizálás mért terhelés alapján.
- Natív VST3 életciklus lefedettsége: állapotkombinációk tesztelése fontosabb,
  mint pusztán a line coverage százalékának növelése.

A prémium filter, gyors presetlista, cutoff presetek és 32 voice külön későbbi
hangminőségi/termékfejlesztési kör. Linux továbbra is halasztott. Az 1.0.4
stabilizációja őrizze meg a GUI-t, hangkaraktert, parameter ID-ket és state-kompatibilitást.
