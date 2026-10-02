# SAWSTAR 1.0.4 — terv a drafttól a publikálásig

Frissítve: 2026-10-02. Ez az 1.0.4 publikálásának közös, végrehajtható terve.
A táblákban a „nyitott” ténylegesen hátralévő feladat; nem feltételezett hiba.
Új eredménynél a konkrét buildet és a bizonyítékot kell rögzíteni, majd a státuszt frissíteni.

## 1. Ellenőrzött kiinduló állapot

| Terület | Igazolt állapot |
|---|---|
| Nyilvános kiadás | GitHub Latest: v1.0.3 |
| Célkiadás | 1.0.4, még nem publikált draft; release ID: 401233074 |
| Jelenlegi draft forrása | `1a6a5f2fc13d9b85a4f0ad4afcb191e4f84ddf38`, #38 squash merge |
| Elkészült javítások | #23–#34, #36 MIDI/editor javítások, #37 presetimport-index, #38 draft-frissítés |
| #38 PR ellenőrzései | 17/17 sikeres az ellenőrzött PR-headen |
| A draft forráscommitjának main ellenőrzései | 11/11 sikeres; Windows, macOS, Quality és draft-előkészítés workflow zöld |
| Szanitizerek | ASan/UBSan: 69/69 teszt sikeres; ThreadSanitizer és coverage is sikeres |
| Csomagok | Mind a 8 fájl újra feltöltve; célcommit, manifest, fájllista és SHA-256 ellenőrzött |
| Dokumentáció | 1.0.4 draft-jegyzetek és EN/HU kézikönyvek elkészültek; README a nyilvános Latestre mutat |
| Kézi elfogadás | A korábbi RC Windows/macOS próbája történeti eredmény; a friss draft natív próbája még nyitott |

A korábbi buildazonosító és a `release.json` 2026.10.01. dátuma nem publikálási bizonyíték.
A terv/dokumentáció későbbi commitja önmagában nem módosítja az itt rögzített csomagok forrását.
A végleges csomagoknak azonban a végleges kiadási commitra kell épülniük.

## 2. Hátralévő feladatok és felelősség

| Sorrend | Feladat | Felelős | Elkészülési feltétel | Státusz |
|---|---|---|---|---|
| 1 | Friss draft letöltése, SHA-256 és About ellenőrzése | Fejlesztő + tesztelő | A tesztelt csomag forrása a fenti SHA; nincs régi párhuzamos VST3 | Nyitott kézi próba |
| 2 | Natív REAPER tesztmátrix és telepítési próba | Tesztelő / Robert | Az alábbi esetekhez konkrét eredmény és környezet rögzítve | Nyitott |
| 3 | Talált hibák reprodukciója, javítása és újratesztelése | Fejlesztő | Minden kiadást blokkoló hiba lezárva, pontos PR-head zöld | Feltételes, csak talált hiba esetén |
| 4 | Végleges dátum, release-scope és dokumentáció egyeztetése | Fejlesztő + kiadásgazda | Egyetlen 1.0.4 scope; nincs tévesen már publikáltnak jelölt draft | Nyitott |
| 5 | Végleges commit CI-je és draft újraépítése | Fejlesztő / Actions | A végleges main commit saját három build/Quality workflow-ja és draft workflow-ja sikeres | Nyitott |
| 6 | Végső csomagellenőrzés és natív smoke | Fejlesztő + tesztelő | Pontos végleges About SHA, egyező hash-ek, működő telepítés és alapfunkciók | Nyitott |
| 7 | Publikálás, Latest és nyilvános oldalak ellenőrzése | Kiadásgazda | Minden előző kapu teljesült, nincs futó draft-frissítés | Nyitott |
| 8 | Letöltéspróba és kiadás utáni megfigyelés | Fejlesztő + tesztelő | Nyilvános csomagok és hash-ek egyeznek; kritikus új hiba nincs | Publikálás után |

## 3. Natív tesztkörnyezet és jegyzőkönyv

Elsődleges környezet: Windows x64 és macOS Apple Silicon, REAPER VST3.
Windows ARM64 és macOS Intel csomagja CI-ben épült; ez nem igazol külön natív
REAPER-próbát ezeken a gépeken. Ha ilyen gép nem elérhető, a natív próbát
„nem tesztelt” állapottal, az ellenőrzött támogatási körrel és a kiadásgazda
maradékkockázatról hozott döntésével kell dokumentálni.

A teljes funkcionális mátrix első köre 48 kHz / 128 sample környezetben fusson.
44,1 kHz / 512 és 96 kHz / 32 sample mellett rövid ismétlés: MIDI/ARP,
presetváltás, bypass és render. Az alacsony buffer gépfüggő túlterhelését
különítsük el az ismételhető pluginhibától; rögzítsük a CPU-terhelést.

Minden futás jegyzőkönyvének mezői:

| Dátum | OS / architektúra | REAPER verzió | SR / buffer | Csomagnév + SHA-256 | About verzió + commit | Teszteset | Eredmény / bizonyíték |
|---|---|---|---|---|---|---|---|
| Kitöltendő | Kitöltendő | Kitöltendő | Kitöltendő | Kitöltendő | Kitöltendő | Az alábbi ID | PASS / FAIL / nem tesztelt; render, kép vagy reprodukció |

Eredeti projekt/preset helyett másolaton teszteljünk. A „korábban működött”
nem helyettesíti a friss csomaggal végzett ellenőrzést.

## 4. REAPER funkcionális tesztmátrix

| ID | Mit kell kipróbálni | Elvárt eredmény |
|---|---|---|
| N01 | Tiszta telepítés, korábbi verzió frissítése, manuális ZIP-telepítés; About és rescan | 1.0.4 és a kijelölt SHA látszik; a host a megfelelő architektúrát tölti; nincs duplikált régi bundle |
| N02 | Host MIDI és editor-billentyű ugyanazon hangon; editor-hang felengedése | Az editor saját hangja felengedhető; a host által tartott hangot nem engedi el tévesen |
| N03 | ARP aktív; editor-hang tartása, transport stop/start, azonos friss host-hang, majd a régi GUI-hang felengedése | A késői GUI-release nem állítja le a friss host-hangot; az ARP/editor tulajdonlás nem marad bent |
| N04 | Gyors sustain on/off, pitch bend és aftertouch események; rövid váltások sűrű MIDI-ben | Az események sorrendje megmarad, nincs elvesző sustain-edge vagy beragadt kontroller; a #36 többpontos regresszió CI-je zöld |
| N05 | ARP HOLD és sustain külön és együtt; stop/start, presetváltás, reset | Nincs váratlan beragadt hang vagy régi esemény újrajátszása; a HOLD tudatos tartását különítsük el a hibától |
| N06 | Editor bezárása aktív mouse-keyboard hang és PITCH-gesztus közben; MOD használata | Saját mouse-hang és pitch-gesztus lezárul; MOD latching megmarad; host-hang nem sérül |
| N07 | Bypass/unbypass MIDI lejátszás közben; plugin offline/online, host reset | Bypass alatt host output néma; visszakapcsolva nincs régi MIDI-roham vagy állapotból eredő beragadt hang |
| N08 | Sűrű MIDI és editor műveletek, majd release; automatizált overflow/zero-frame regressziók áttekintése | Hallható beragadás nincs; az overflow/zero-frame CI-tesztek sikeresek. Nem reprodukált kézi overflow-t ne jelöljünk kézzel teszteltnek |
| N09 | Mind a 24 factory preset, gyors egymás utáni váltás; Volume/Boost, voice-mode és host automation | Nincs crash, NaN vagy váratlan dirty-jelzés; a rendes paraméterautomatizálás jelenlegi blokkos viselkedése ismert |
| N10 | Vegyes batch: jó, hibás, hiányzó fájl; azonos név, Unicode név, pontos/majdnem azonos hang, másolatimport | Jó fájlok importálódnak, hibák jelentése érthető, meglévő név nem íródik felül; csak pontos hangazonosság szűr |
| N11 | 100/400 preset importja; több pluginpéldány; egyszerre indított import; editor kezelhetősége | Könyvtár konzisztens, nincs elvesző preset vagy tartós fagyás/crash; a szinkron import tényleges GUI-késését mérjük, ne ígérjünk háttérmunkát |
| N12 | Save / Save As / overwrite backup; rename, archive, favorites, újranyitás; megváltozott külső fájl | Régi hang visszaállítható, névütközés nem ír felül; stale overwrite elutasítva; kedvencek és hibajelzések következetesek |
| N13 | Régi projektmásolat visszatöltése, új projekt mentése/újranyitása, több pluginpéldány | Paraméterek/state kompatibilisek, minden példány saját állapotát kapja; eredeti projekt/preset nem sérül |
| N14 | Száraz és maximális FX/release preset offline renderje; hosszú FX után száraz preset; eltérő REAPER tail-beállítások | Kezdő tranziens és hallható tail teljes; nincs hallható csonkolás. A 380 s host bound miatti hozzáfűzött csendet külön jegyezzük fel |
| N15 | Editor eltakarása, minimalizálása, újranyitása; ismételt instantiate/remove; hosszabb lejátszás | GUI/hang stabil, nem szaporodik kontrollálatlanul CPU/memória, nincs összeomlás vagy új reprodukálható regresszió |
| N16 | Installer/uninstaller működése, presetek és projektadatok megőrzése | A plugin és dokumentáció helye megfelelő; a felhasználói saját adatok megmaradnak |

A rendernél szándékosan hagyott rövid előfutás és hosszabb lecsengés nem hiba.
Hallásos elfogadáshoz legyen száraz pluck, supersaw lead, pad és ARP példa is.
Az előző RC PCM-azonossága nem bizonyítja automatikusan az új render azonosságát.

## 5. Mit kell javítani, és mi nem kiadási előfeltétel?

Jelenleg nincs igazolt, nyitott, a fenti friss draftot blokkoló kódhiba.
A kiadásig hátralévő munka elsősorban natív ellenőrzés és véglegesítés.
Ez nem jelenti azt, hogy a még nem elvégzett próbák biztosan hibamentesek.

**Blokkoló, ha előkerül:** crash, ismételhető beragadt/idegen hangfelengedés,
preset/projekt-adatvesztés, sérülő state-kompatibilitás, hallható rendercsonkolás,
rossz architektúra/verzió, hiányos vagy rossz hash-ű csomag, telepítési hiba,
sikertelen szükséges CI, vagy a tesztelt és publikálandó commit eltérése.

**Mérendő és eldöntendő:** a szinkron import GUI-késése, a host hosszú tail miatti
csendhozzáfűzése és a gépfüggő CPU-terhelés. Ezeknél konkrét mérést és
felhasználói hatást rögzítsünk; súlyos, ismételhető használhatósági regressziót
javítsunk a kiadás előtt. Merev 48× sebességígéret nincs: az importmérés egyetlen
helyi Windows/MSVC futás eredménye.

**Későbbi fejlesztés, önmagában nem blokkolja az 1.0.4-et:** atomikus preset-
tranzakció/host-automation újratervezése, aszinkron import, hard link nélküli
rename-fallback, sample-accurate rendes paraméterautomatizálás, új filterkarakter,
32 voice és Linux plugin. Igazolt adatvesztés vagy crash ezekben az útvonalakban
viszont a fenti blokkoló hibák közé kerül. A lezárt click/pop kutatás külön scope.

Talált hiba kezelése: pontos build + reprodukció → bukó regresszió vagy
dokumentált natív próba → kis célzott javítás → szükséges helyi teszt →
pontos PR-head teljes CI → zöld PR merge → új main CI és draft → érintett natív
esetek ismétlése. DSP/MIDI/state változás után a kapcsolódó mátrixot is újra kell
futtatni; korábbi más build PASS eredményét nem szabad átírni új SHA-ra.

## 6. Véglegesítés publikálás előtt

- [ ] A natív jegyzőkönyv elkészült, a nem tesztelt környezetek és maradékkockázatok láthatók.
- [ ] Minden blokkoló hiba lezárt; a scope 1.0.4 stabilitási kiadás marad.
- [ ] A tényleges tervezett publikálási napot rögzítettük; a régi 2026.10.01. dátumot nem használjuk automatikusan.
- [ ] `release.json`: version 1.0.4, candidate üres, végleges date; a metadata-sync és `--check` sikeres.
- [ ] Changelog: az 1.0.4-be kerülő Unreleased tételek összevezetve a kiadási fejezettel; nincs kettős vagy kihagyott scope.
- [ ] Release notes, buildmeta, macOS plist/config, kézikönyvek és csomagok dátum/verzió állításai egyeznek.
- [ ] Módosított EN/HU PDF-eket újrageneráltuk és vizuálisan ellenőriztük; a régi képernyőképek továbbra is tényleges verziójukkal vannak jelölve.
- [ ] A végleges, tagelendő commit README-je már kiadás előtt és után is igaz, semleges szöveget tartalmaz: a forrás verziója 1.0.4, a nyilvános letöltés a `/releases/latest` linken ellenőrizhető. Ne maradjon benne „1.0.4 unpublished” vagy „1.0.3 current public” állítás; és ne állítsa idő előtt, hogy 1.0.4 már publikált. A tagelt README-t és a GitHub forrásarchívumait egy későbbi main-only módosítás nem javítja ki.
- [ ] A csomagolt release notes már végleges, semleges státuszú legyen: ne állítson idő előtt megtörtént publikálást, és ne maradjon benne később téves „unpublished” állítás. A jelenlegi draft-státuszszövegek eltávolítása a végleges commit része.
- [ ] A végleges PR pontos headje zöld, a merge commit saját Windows/macOS/Quality push-futásai is sikeresek.
- [ ] A végleges commit draft workflow-ja sikeres; a csomagmanifestek source_commit mezője és a draft target ugyanaz a teljes SHA.
- [ ] A végleges build rövid natív próbája megismételve, About SHA ellenőrizve. Ha csak metadata változott, a korábbi teljes mátrix hivatkozható az eltérés felsorolásával, de a végleges csomag smoke-ja nem hagyható el.
- [ ] A kiadásgazda a konkrét végleges csomagokról és a dokumentált tesztkorlátokról döntött.

## 7. A publikálandó fájlok végső ellenőrzése

Pontosan ezek a fájlok legyenek a draftban, 1.0.4 verzióval:

- `SAWSTAR-1.0.4-Windows-x64-Setup.exe`
- `SAWSTAR-1.0.4-Windows-x64-Manual.zip`
- `SAWSTAR-1.0.4-Windows-ARM64-Setup.exe`
- `SAWSTAR-1.0.4-Windows-ARM64-Manual.zip`
- `SAWSTAR-1.0.4-macOS-Universal.dmg`
- `SAWSTAR-1.0.4-macOS-Universal.pkg`
- `SAWSTAR-1.0.4-macOS-Universal-Manual.zip`
- `SHA256SUMS.txt`

- [ ] Mind a 7 csomag letöltve, hash-e egyezik a SHA256SUMS és a GitHub API SHA-256 adataival.
- [ ] Mindhárom manuális ZIP ép, teljes és megfelelő architektúrájú; a manifest, licencek, installációs leírás és mindkét PDF megvan.
- [ ] A telepítőket a megfelelő platformon próbáltuk; unsigned/notarization állítások pontosak.
- [ ] Nincs „refresh in progress”, hiányos feltöltés, futó draft-workflow vagy ismeretlen extra asset.
- [ ] A v1.0.4 tag nincs más commiton; a kiadási út nem force-mozgat taget és nem ír felül régi publikált release-t.

## 8. Publikálás és azonnali utóellenőrzés

Csak az előző kapuk teljesülése után:

1. A végleges release-scope, dátum, teljes SHA és tesztjegyzőkönyv rögzítése; a draft frissítése közben ne publikáljunk kézzel.
2. A draft publikálása normál 1.0.4 release-ként, nem prerelease-ként, kifejezett **Latest** jelöléssel.
3. A v1.0.4 tag tényleges commitjának feloldása és összevetése a manifest/source SHA-val; draft=false, prerelease=false ellenőrzése. A tagelt README és a GitHub által generált source ZIP/tarball verzió- és státuszszövege is helyes legyen.
4. A semleges, már végleges commitban rögzített README letöltési linkjének ellenőrzése: `/releases/latest` valóban 1.0.4-re mutat. Opcionális „megjelent” bejelentés utána kerülhet a mainre, de ez nem helyettesíti a tagelt README helyességét. A csomagolt release notes már a véglegesítéskor elkészült; publikált csomagot nem módosítunk. Notes-változás publikálás előtt új csomagkört igényel; publikált tagra a draft-frissítő szándékosan blokkol.
5. Nyilvános ellenőrzés bejelentkezés nélkül: repository Releases/Latest, README, `/releases/latest`, tag-oldal, a 8 asset és a dátum/verzió egyezése.
6. A nyilvános Windows/macOS csomagok újbóli letöltése és hash-ellenőrzése; legalább egy telepítési/REAPER smoke platformonként a rendelkezésre álló gépeken.
7. Kiadási jegyzőkönyv lezárása: SHA, workflow-linkek, asset/hash-lista, natív eredmények, elfogadott korlátok és publikálási időpont.
8. Új kritikus hiba esetén reprodukció, a hiba egyértelmű jelzése és célzott következő javítókiadás. Publikált csomagot/taget nem cserélünk le csendben.

A draft elkészülése és a zöld Actions önmagában nem a publikálási döntés.

## 9. Korábbi eredmények — csak a megnevezett buildre

A #23–#34 utáni RC `3225747` esetében korábban rögzített eredmények:
helyi Release 67/67, célzott ASan/UBSan 7/7, macOS VST3 validator 47/47;
platformcsomag/hash és CI installer/uninstaller ellenőrzések; három REAPER-render
PCM-azonossága a tesztelt RC-ággal; egy 385 s FX-kontrollrender utolsó 30 s-a néma.
A tulajdonos 2026-10-01-én Windows és macOS alatt nem tapasztalt hibát.
Ezek nem a #36–#38 utáni teljes natív mátrix eredményei.

Kapcsolódó források: [1.0.4 jegyzetek](RELEASE_NOTES_1.0.4.md),
[fejlesztési háttér](DEVELOPMENT_PLAN_POST_1.0.4.md),
[telepítés](INSTALLATION.md), [követelmények](SYSTEM_REQUIREMENTS.md).
