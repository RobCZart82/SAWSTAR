# Fejlesztési napló — 2026-11-03, Premium production RC

Alap: `d0bad8edd44e418c1a7b3527a2e20f862b0ebc4a` (#132 squash merge).
A base PR-jének 23 workflow / 71 job, majd a main négy workflow-ja sikeres.
Robert kifejezetten kérte az integráció, factory/RC és kiadási dokumentáció
előrehozását; a natív, minőségi és K1-kapuk nem válnak ettől elfogadottá.

## Implementált

- Explicit `SAWSTAR_PREMIUM_PRODUCTION=ON` a SAWSTAR wrapperben, közös
  PluginSynth-kiválasztás a wrapper és tesztek számára. Kalibrált lookup
  referencia; a Saw-dispatch Candidate nem került előléptetésre.
- SAWSTAR név/ID, paraméterek, state/preset-formátum és presetmappa megőrizve;
  jelentett latency 32 minta; Preview/Candidate kapcsolóval ütközés tiltott.
- Premium factory kalibráció és az Init wrapper-default alkalmazva az RC-re,
  változatlan factory trimekkel és védelmekkel.
- 1.1.0 rc1 metadata, üres release_date; Aboutban integration jelölés.
- RC-only csomagoló tényleges VST3-validálással, teljes fájlhash-jegyzékkel,
  natív/zenei/publikálási elfogadás false jelöléssel; felülírás tiltott.
- macOS Universal / Windows x64 integrációs CI, Macen mindkét szelet
  state/smoke/headroom ellenőrzésével. Nincs automatikus telepítés/publikálás.
- [RC és migrációs kézikönyv-kiegészítés](PREMIUM_PRODUCTION_RC.md),
  [factory mérések](PREMIUM_FACTORY_RC_2026_11_03.md),
  [release notes tervezet](RELEASE_NOTES_1.1.0.md), roadmap/index/terv frissítése.

## Helyi ellenőrzés

- macOS arm64 Release Premium engine, smoke és SAWSTAR VST3 sikeresen lefordult.
- Két production identity/latency/state regresszió 2/2 sikeres; minden factory
  preset sound/host/GUI-scale és visszahívott render vizsgálva.
- Kalibráció/guard szerződés és 144/144 limiter előtti factory-headroom eset
  sikeres; legnagyobb csúcs −6,51293 dBFS, változatlan −6 dBFS kapuval.
- RC-csomag build/identity elutasításai és ZIP-integritás két Python-tesztje sikeres.
- Mindkét tiltott Preview/Candidate + production konfiguráció elutasítva.
- actionlint 1.7.12, Python-szintaxis és központi release metadata ellenőrzött.
- Teljes helyi Classic Release regresszió 144/144 sikeres (225,25 s).

A helyi SDK-validátor konfigurálása teljes Xcode hiánya miatt elakadt;
a CLT fordítóval a plugin maga sikeresen elkészült. Emiatt helyileg validált
RC ZIP nincs állítva és nem csomagoltunk ellenőrizetlen terméket. A tényleges
validátor és Windows/Universal csomag távoli CI-kapu.

## Még nem elfogadott

Az új PR távoli CI-je, az RC pontos natív telepítése és REAPER/Windows CPU-,
bypass-, automatizálás-, GUI-/lifecycle-próbája; presetenkénti zenei meghallgatás;
K1 gyökérok/rendezés; végleges rate/buffer policy és PDF-kézikönyv-újrakiadás;
a konkrét végleges RC és publikálási tartalom jóváhagyása.
Az alapértelmezett build Classic. Nincs release/tag, production alapértelmezés-
átkapcsolás vagy publikálási művelet. A korábbi negatív mérési bizonyítékok
változatlanul megmaradnak.


## Windows RC CI-diagnosztika — első head, 2026-11-03

A `3e0037dd` head Windows x64 VST3 fordítása és az identity/latency/state
próbák sikeresek. A factory levels executable indításakor exit 127 jelent meg,
az első CSV-sor előtt; nincs headroom-küszöbtúllépésből eredő hiba.
[Futás](https://github.com/RobCZart82/SAWSTAR/actions/runs/41576883296),
job `125722350364`. A feltöltött 152 bájtos artifact csak üres CSV,
nem validált RC-csomag és nem sikeres mérés.

A fixture több nagy Synth objektumot tett a veremre (egy Premium Synth
helyileg 481 984 bájt). AppleClang 21 arm64 -O3 statikus stack-usage szerint
a main kerete 1 013 744 bájt volt. A nagy tesztmotorokat egyedi heap-tárolóba
helyező korrekcióval 295 632 bájt; a DSP, vezérlések, bemenetek, kalibráció
és minden numerikus/headroom-küszöb változatlan. A Windows veremhiba oka
még hipotézis, távoli új ellenőrzés szükséges; nem minősítjük bizonyítottnak
pusztán az arm64 kódelemzés alapján. A workflow Windows útjai explicit .exe
kiterjesztést kapnak az indítás egyértelműsítésére.

A korrigált fixture helyileg 144/144 esetben sikeres, és a teljes CSV
bájtra egyezik az első sikeres helyi gyűjtéssel. actionlint/diff ellenőrzés
sikeres. A javítás előkészítve; a régi head többi folyamatban levő mérését
nem szakítjuk meg feltöltéssel. A következő ellenőrzéskor, az aktuális kampány
lezárása után ugyanebbe a PR-be kerülhet fel. Új PR és mérési retry nincs.


## #133 lezárás és RC-dokumentációs lépcső

#133 végső PR-head 22 workflow / 67 job sikeres után beolvadt b9f35574 mainre;
a main 5 workflow / 12 job sikeres. Publikálást az rc1 guard kihagyta.
[A tartós csomag-/CI-bizonyíték és az új provenance/PDF munka](PREMIUM_RC_CI_EVIDENCE_2026_11_03.md)
külön választja a régi mérési forrást az új dokumentációs headtől.
A magyar/angol RC-kézikönyv 15-15 oldalon elkészült és vizuálisan ellenőrzött.
A raw dirty mező megtartása mellett új teljesfájl-/pinellenőrzés kerül a
csomagolásba; a helyi pozitív dependency és negatív checkout kontroll sikeres.
Az új head CI-je még függő; natív/zenei elfogadás nem történt.


## #134 P2 review-korrekció

A nem követett iPlug2 header elutasításának hiányát valós helyi fájllal
reprodukáltuk és külön untracked leltárral javítottuk. Csak a külön
pin/tisztaság szerint ellenőrzött SDK fa kivétel; más ismeretlen fájl tiltott.
Öt packaging/provenance teszt, valódi pozitív/negatív dependency-kontroll
és CTest packaging cél sikeres. Feltöltés az aktuális CI lezárása után.


## Új kérés: hat performance factory hang

Alap: #134 squash merge `1660ced6605343d61b485dfca2514547c3414b0f`;
frissen ellenőrzött négy main workflow mind sikeres, nyitott PR nem volt.
Robert korábbi rc1 csomagokra Windows/macOS alapműködési sikert jelzett;
ez nem az új presetbank natív/zenei elfogadása.

- Implementált: 24 → 30 bank, hozzáfűzött stabil kulcsokkal/indexekkel;
  négy filtermód, két LFO/mátrix, wheel/pressure/velocity, glide, effektek,
  két arp és névben játékutasítás. Régi 24 definíció és trim változatlan.
- Helyileg tesztelt: Classic Release 144/144 CTest; Premium production
  state/recall és identity/latency 2/2, névvel jelölt CC1 hangváltozással.
  Az első teljes futás egy elavult négy-lead elvárása megbukott; az ötödik
  leadre igazítás és wheel/hold keresési ellenőrzés után teljes sor sikeres.
- Végleges Premium pre-guard headroom: 252/252 eset, változatlan -6 dBFS határ,
  worst -6.51292972 dBFS. Az eredeti 144 mért sor változatlan; az új hangok
  hangerőhangolása utáni végleges újforrás-mérés, nem CPU-retry.
- A valódi arm64 VST3 lefordult, automatikus install nem történt. A helyi
  validátorhoz szükséges teljes Xcode továbbra nincs; validált Mac/Windows
  RC-t a távoli CI bizonyíthat, helyi fordítás önmagában nem.
- EN/HU PDF 15/15 oldal frissítve, renderelve és szöveghatárokkal ellenőrizve.
  A régi browser-képet a bővült hanglista váltja a 10. fejezetben; a korábbi
  24/144-es RC-bizonyíték történeti jelölése megmarad.
- Távoli CI: az új PR pontos headjére még függő, eredménye a PR-leírásban
  véglegesítendő. Natív/zenei elfogadás: az új hangokra nyitott.

[Hanglista, játékutasítások, teljes helyi CSV és mérési korlátok](EXPRESSIVE_FACTORY_PRESETS_2026_11_03.md).
Nincs DSP/state/ID/threshold változtatás, default Classic marad; release nincs.


### #135: Windows Debug időkorlát és hat új hang zenei jóváhagyása

Robert a hat új exportált hangot meghallgatta és jelezte: „meghallgattam jól
szólnak”. Ez az új hat snapshot zenei elfogadása; a teljes bank/pontos VST3
natív host-/CPU-/lifecycle-/K1-/publikálási kapui ettől nem záródnak le.

Az első head Windows foundation futása `41710536547`: Release és VST3 x64
sikeres; Debug 142/143 sikeres, a `factory_presets` 600,01 másodpercnél timeout.
A külön Premium production RC `41710536432` sikeres. Ez tesztmunka-méretezési
hiba, nem bizonyított hang-/preset-/CPU-regresszió.
A teljes factory render/ráták/akkord/lecsengés-ellenőrzés két diszjunkt,
indexalapú félbank-tesztre bontva fut; a 600 s időkorlát és valamennyi
mintaszám/küszöb/állítás megmarad. A bank globális kulcs/név-egyedisége
mindkét tesztben ellenőrzött. Az argumentum nélküli futtatás továbbra teljes bank.
A javítás feltöltése megvárja a régi head még futó workflow-it.


A félbank-javítás helyi ellenőrzése: Debug 2/2 sikeres (101,01 s és 78,80 s),
Release 2/2 sikeres (13,79 s és 10,36 s). Mind a 30 preset ugyanazokon a
mintaszámokon, rátákon és lecsengési állításokon ment végig.
2026-11-03 21:00 UTC-kor a régi head Windows workflow-ja már 2. kísérletben
futott, GitHub triggering_actor `RobCZart82`, új Debug job `126197624639`;
ezt az agent nem indította. Az első timeout bizonyítéka megmarad, a javítás
feltöltése megvárja e futás végét is.


### 2026-11-04: 001847b CI — megőrzött új K1 negatív eset

A 22 workflow közül kettő piros: Windows foundation `41724068907` Debug
`factory_presets_remaining` 600,01 s timeout (143/144 sikeres); az első
félbank 111,06 s alatt sikeres. Release/VST3 x64/ARM64 és Premium RC sikeres.
A félbank-felosztás Windows Debugban tehát nem elég. A teljes mintaszámú
ellenőrzés presetenként külön CTest-feladatra bontva fut; a globális egyediség
minden feladatban ellenőrzött, a 600 s és numerikus határok maradnak.
A napló flush-ölt presetkulcsot is ír a lassú eset azonosításához.

K1 passive FP observation `41724068835`, job `126224107488`, artifact
`13272794936`: valódi új negatív megfigyelés, nem infrastruktúrahiba.
Rosetta x86_64 observed, triangle/waveform2, 176400 Hz, filter0, poly/voice0,
FX on, reference→candidate sorrend, main frame152: pre-FX right
reference `0x407b4cb2` / candidate `0x407b4cb1` (1 ULP). A kimeneti két
csatorna ebben az első eltérő mintában még bitazonos. MXCSR mindkét process
előtt/után `0x1fb3`, controls `0x1f80`, rounding0; nincs itt vezérlőváltás.
Az arm64 baseline/observed/candidate-first és x86 baseline/candidate-first
sikeres, az x86 observed megszakadt a 49. teljes eset után. Ez nem okbizonyítás.
[Teljes manifest és első eltérés](measurements/k1-passive-001847b-negative.json).
Mind a 20 nyers output/log hash a letöltés után egyezett a manifesttel.

A presetadatok újrahangolása vagy a K1-küszöb lazítása nem történik. A CMake
szükséges tesztfelosztási javításának új CI-je nem változatlan mérés retryja;
egy későbbi zöld K1-futás nem törli ezt a negatív bizonyítékot és nem K1-javítás.

A presetenkénti végleges tesztfelosztás helyben Release 30/30, Debug 30/30
sikeres. A CTest JSON-jegyzék pontosan a 0..29 indexeket tartalmazza, hiány és
ismétlés nélkül; hibás/negatív/tartományon túli selector elutasított. A régi
félbank és argumentum nélküli teljesbank futtatás elérhető marad.
A távoli javítás hatását és a K1 státuszát az új pontos head CI-je külön méri.


### 66c100d: a Windows Debug lassú eset azonosítva

Windows `41783140508`, Debug job `126419088940`: 171/172 sikeres, kizárólag
`factory_presets_25` / Velvet Orbit érte el a 600,01 s időkorlátot. A többi
29 preset sikeres (az új 24/26/27/28/29 indexek 9,01/9,26/15,79/5,90/11,46 s).
A finomabb felosztás tehát azonosította az esetet, de nem javította a lassulást.
A Windows Release, VST3 x64/ARM64 és Premium RC workflow sikeres.
A korábbi „tesztmunka-méretezési” minősítés hipotézis volt; a konkrét ok
még nem bizonyított, nem növeljük az időkorlátot és nem rövidítjük a rendert.

Célzott helyi x86_64 Debug/Rosetta Velvet Orbit teljes próba sikeres, a timeout
nem reprodukálódott. Egy külön helyi állapotmegfigyelő a 48 kHz-es, 18 másodperces
single+release szakaszban 1024 mintánként vizsgálta a név szerint elért engine,
LFO, delay/reverb/chorus, filter, ADSR és oszcillátor állapotokat: subnormal érték
nem volt. Ez ritka/köztes jelenséget, más rátát vagy Windows-különbséget nem zár ki.
A helyi probe outputs/ci-135-velvet-debug alatt van; nem pluginrésze.

A factoryteszt Velvet Orbit esetén most minden ráta/single/chord/release
határon, a release-ben másodpercenként flush-ölt időbélyeges előrehaladást ír.
A DSP, presetadat és valamennyi render/ellenőrzés marad. Ez célzott Windows
hibaazonosítás, nem bizonyított javítás vagy CPU-/natív elfogadási mérés.
A Premium RC workflow pathlistája e factoryteszt-függőséget is figyeli, hogy
az aktuális headhez tartozó csomagok később valóban előálljanak.


### 2026-11-04: a Velvet Orbit hibájának helyi bizonyítása és javítása

Az 1f138ea Windows `41787944470` Debug `126434314232` naplója: 44,1 kHz,
single2s gyors, release0..10s gyors (összesen2,7167s), következő checkpoint hiányzik.
A chorus negatív körpozíciója float kerekítéssel 8192 lehetett, az első
interpolációs cím wrap nélkül érvénytelen lett. Az exact Velvet chorus
ütemezés 565729. mintánál /12,828322s-nél előidézte e címet, a Windows
elakadás intervallumán belül. Külön 48k fixture a régi bal olvasásban a jobb
csatorna -0,75 értékét adta a várt +0,25 helyett. A regresszió a régi
feldolgozási kóddal ténylegesen piros volt; az egyszerűsített „csak hosszú teszt”
hipotézis elvetve.

A fix az első chorus címet is modulo8192 körbeforgatja, az eredeti törtrészt
megőrzi. Ez szükséges közös DSP-javítás a preset-PR-ben; a régi és új
presetadat/ID/trim változatlan. Nem timeout/threshold/FP lazítás.
[Részletes bizonyíték, mérés és teszt](CHORUS_RING_BOUNDARY_2026_11_04.md).
A javított Windows/RC CI és a helyi szélesebb eredmények külön rögzítendők;
a K1 előző negatív nem törlődik és nem válik ettől megoldottá.


## Helyi javítás utáni eredmény

- Teljes Classic Release CTest: **173/173 sikeres** (91,31 s).
- Debug: a chorus regresszió és a 30 factory feladat **31/31 sikeres** (120,54 s).
- Premium production state/identity: **2/2 sikeres**.
- Premium limiter előtti headroom: **252/252 sikeres**, worst **-6.51292972 dBFS**;
  a teljes CSV byte-egyezik a korábbi commitolt rácséval.
- Külön AddressSanitizer/UBSan boundary-probe: sikeres, a korábbi -0,75 helyett
  a helyes +0,25 bal csatorna. A régi feldolgozási kód negatív kontrollja megmarad.
- Valódi arm64 VST3 újrafordult; nincs automatikus install/publikálás.

A távoli Windows Debug és valamennyi pontos-head CI/RC bizonyíték még függő.
A korábbi K1 negatív külön nyitott, a chorus-javítás nem K1-javítás.
