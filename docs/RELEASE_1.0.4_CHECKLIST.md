# SAWSTAR 1.0.4 — kiadási terv és elfogadási kapuk

Frissítve: 2026-10-01. Kiinduló main: `45f11dc` (#32), mindhárom main
workflow sikeres. A tulajdonos engedélyezte a javítást és az 1.0.4 publikálását;
a zöld PR-ek mainbe olvaszthatók. Ez az engedély nem helyettesíti a teszteredményeket.

## 1. Stabilitási scope

A v1.0.3 utáni, célzottan tesztelt javítások kerülnek a kiadásba:

- #23: editor MIDI recovery ownership és ADSR precision boundary.
- #24: fejlesztői/RC/kiadási buildazonosítás.
- #25–#26: végső pitch/mod controller state megőrzése túlcsorduláskor.
- #27–#29: reset előtti editor FIFO, késői saját hang nélküli felengedés,
  keyboard és aktív PITCH-gesztus befejezése editorbezáráskor.
- #30: zero-frame overflow és bypass MIDI/DSP életciklus.
- #31: véges VST3 tail-bejelentés a teljes soros FX-láncra.
- #32: diszkrét presetértékek kanonizálása, téves dirty-marker megszüntetése.
- #33: szigorú standalone presetimport, változatlan host-state migrációval.

A paraméterazonosítók, plug-in identity, wire format, GUI elrendezés és alapvető
hangkarakter megmaradnak. Filterkarakter, automatikus pitch smoothing, 32 voice,
Linux plugin, sample-accurate automation és preset-tranzakció újratervezése
nem feltétele ennek a stabilitási kiadásnak. A kattogáskutatás lezárt marad.

## 2. Fájldekóder javítása és összeolvasztása

Regresszió: szöveg/headerless adatok, üres/unknown-only modern payload,
host trailer, hibás hossz/verzió/known rekord, NaN, minden csonkolt hossz.
Kontroll: jelenlegi és régi verziózott részleges presetek, jövőbeli rekordok,
diszkrét kerekítés, kevert jó/rossz fájlok importja. Host legacy és +4 byte
trailer kompatibilitás külön, változatlan teszttel.

Kapuk: régi fájlbetöltőn bukó regresszió; helyi Release és célzott ASan/UBSan;
pontos PR-headen teljes platform/Quality CI; konfliktusmentes normál merge.

## 3. 1.0.4 jelölés és dokumentáció

Külön kiadási ágban `release.json` verzió 1.0.4, kezdetben `dev`/`rcN`,
üres dátummal. A szinkronizáló frissíti a framework configot és macOS plistet.
Új changelog és `RELEASE_NOTES_1.0.4.md`; a régi kiadások jegyzetei változatlanok.
Friss angol/magyar PDF kézikönyv, generált PDF-ek vizuális ellenőrzése,
installációs/kompatibilitási leírás és a tail/import viselkedés dokumentálása.
A régi screenshotot 1.0.3-ként kell jelölni, vagy tényleges RC screenshot váltja fel.

## 4. Aktuális RC ellenőrzése

Az ellenőrzések a konkrét RC commitra és buildre vonatkoznak; az 1.0.3 korábbi
elfogadását nem tekintjük 1.0.4 elfogadásnak.

- Release, ASan/UBSan és TSan; Windows x64/ARM64, macOS Universal.
- Valódi VST3 validator, archivum/manifeszt/hash/licencek és mindkét kézikönyv.
- Installer és uninstaller CI-próbák; a támogatott platformok hiánytalan csomagjai.
- Natív REAPER: host/editor azonos hang, késői Note Off, reset, editorbezárás,
  aktív pitch-visszaállítás, MOD latching, bypass/unbypass, sustain és ARP HOLD.
- Legacy projektmásolat visszatöltése; eredeti projekt és preset nem íródik felül.
- Tail: normál száraz és maximális FX/release preset, hosszú FX utáni száraz
  presetváltás, offline export és a host tényleges trimming/append szabálya.
  A 380 másodperces bejelentés konzervatív; a hallható lecsengést és az esetleges
  hosszú hozzáfűzött csendet külön ellenőrizni kell.
- Hallásos és editor eltakarás/minimalizálás/visszanyitás elfogadás az RC-vel.

Ha valamely natív platformpróba nem végezhető el, az nyitott kapu; nem írjuk
automatikusan sikeresnek. A maradék kockázatról konkrét eredményekkel kell dönteni.

## 5. Végleges build és publikálás

Csak az RC-elfogadás után: üres candidate, tényleges kiadási dátum,
egyező notes/kézikönyv/buildmeta. A végleges commit mindhárom push-workflow-ja
sikeres legyen; régi vagy más commit CI-je nem használható helyettesítésként.
A kiadási workflow ellenőrzött draftot készít; a teljes assetlista, hash-ek,
platformok, licencfájlok és verzió ellenőrzése után a draft publikálható.
A `v1.0.4` tag és kiadás a végleges commitra mutasson; korábbi release-t nem írunk felül.

## Jelenlegi státusz

- #23–#32 mainben, legutóbbi main platform és Quality CI sikeres.
- #33 mainben (`645a994`): strict file-dekóder; régi betöltőn bukó regresszió,
  helyi Release 67/67, célzott ASan/UBSan 7/7 és PR CI 17/17 sikeres.
- 1.0.4-rc1 metaadatok és kiadási jegyzet elkészültek a külön kiadási ágon.
  Az EN/HU kézikönyv 14/14 oldala renderelve és vizuálisan ellenőrizve;
  teljes helyi Release tesztsor 67/67 sikeres.
- RC platformcsomagok, natív elfogadás és végleges publikálás: hátravan.
- Nincs még 1.0.4 tag vagy nyilvános kiadás.
