# SAWSTAR 1.0.4 — kiadási terv és elfogadási kapuk

Frissítve: 2026-10-02. A cél a még nem publikált 1.0.4 draft frissítése a
#36 és #37 javításaival. A zöld PR-ek mainbe olvaszthatók; a draft-előkészítés
nem publikálás. A korábbi teszt és felhasználói elfogadás konkrét buildre
vonatkozott, nem helyettesíti a friss csomagok ellenőrzését.

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
- #36: ARP stop/editor ownership, minden VST3 MIDI-kontrollerpont offsetje és editor overflow.
- #37: batch presetimport indexelése, pontos duplikációval és sikertelen mentés utáni helyreállítással.

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

## Aktuális draft-frissítési kapuk

- A workflow a kiadási script/jegyzetek változására is indul; manuálisan a
  `Prepare verified release draft` workflow indítható a `main` ágon.
- Minden frissítés a saját main commitjának három sikeres push-workflow-ját és
  csomagjait használja. Egy PR zöld CI-je nem helyettesíti ezt.
- Meglévő, nem publikált draft frissíthető; nyilvános release nem írható felül,
  eltérő commitra mutató meglévő tag nem mozgatható.
- Upload előtt a draft ideiglenesen „refresh in progress” jelölést kap; a végső
  cím/jegyzet csak egyező célcommit, assetlista, méret és elérhető API-hash után
  kerül vissza. Uploadhiba után a draft hiányos lehet; nem publikálható.
- A frissítés alatt ne történjen kézi publikálás. Az ellenőrzött frissítés végén
  nézzük meg a telepítőket, hash-eket, About commitot és a natív REAPER-eseteket.
- Publikáláskor a tényleges dátum, release/tag, Latest, README letöltési szöveg és
  csomag/About verzió egyezzen. A README `/releases/latest` linkje a nyilvános
  kiadáshoz vezet; az 1.0.4 addig kifejezetten draftként szerepel.
- A 2026-10-01 metaadatdátum a korábbi build része; nem publikálási bizonyíték.

## Korábbi build ellenőrzései — történeti eredmények

- #23–#34 mainben; az RC main `3225747` teljes Windows/macOS/Quality CI-je zöld.
- Helyi Release 67/67, célzott ASan/UBSan 7/7, natív macOS VST3 validator 47/47.
- Mindhárom RC platformcsomag pontos commitja, verziója, architektúrája,
  manifesztje, licencei és hash-ei ellenőrizve; CI installer/uninstaller sikeres.
- Három main REAPER-render PCM-bitazonos a tesztelt ág RC-renderjével.
  A 385 s explicit FX-render utolsó 30 s-a néma. Ez egy kontrollpreset;
  nem bizonyít minden presetet vagy a host automatikus tail-hozzáfűzését.
- A tulajdonos 2026-10-01-én macOS-en és Windows-on kipróbálta a jelöltet:
  nem hallott és nem tapasztalt hibát. Ez felhasználói elfogadás, nem minden
  lifecycle-kombináció külön dokumentált natív regressziója. A célzott
  lifecycle-esetek automatikus regresszióval/CI-vel fedettek; részletes
  platformonkénti kézi jegyzőkönyv és host tail-trimming vizsgálat maradék
  tesztkorlátként megmarad, új hiba esetén célzottan újravizsgálandó.
- Végleges 1.0.4 metaadat és dátum: 2026-10-01. EN/HU kézikönyv frissítve.
- A végleges commit CI-je, ellenőrzött draft és publikálás a kiadási workflow
  következő kapuja; az RC CI önmagában nem helyettesíti a végleges CI-t.

