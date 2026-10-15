# Premium Preview — hangerőkorrekció és factory headroom

## Kiindulás és felhasználói döntés — 2026-10-15

Robert a natív REAPER-próbában a Premium filtert a Classicnál zeneileg
használhatóbbnak és jobb hangúnak ítélte; az elvárt karakter teljesült.
A további munka ezt a karaktert őrzi meg. A Classicnál halkabb kimenet
korrekciót igényel, de nem cél a hangosság maximalizálása vagy a factory
presetek csúcsvédelembe hajtása.

A korábbi, azonos beállítású LP24 / 25% rezonancia / 6 dB Drive renderekben
a Premium teljes renderre számolt RMS-szintje három hangnál 4,02 dB-lel,
16 hangnál 3,43 dB-lel kisebb volt. A második projektben mindkét Boost
18-ról 5 dB-re csökkent; a renderhatárok is eltértek. Ezek nem a két
polifóniaszint közvetlen hangerő- vagy CPU-összehasonlításai.

A saját REAPER-kijelzőn megfigyelt Serum 3,2% / Premium 2,5% külön,
felhasználói pillanatnyi összevetés, hasonló saw hanggal; nem illesztett
benchmark vagy általános szintetizátorrangsor. A hangkarakter pozitív
értékelése nem zárja le a CPU-, magasráta-, lifecycle- és release-kapukat.

## Korrekció a külön Preview buildben

- Teljes wet filternél **+3 dB** kimeneti korrekció, a mért teljes eltérésnél
  tudatosan kisebb emeléssel. Nincs pontos Classic-hangerőegyezési ígéret.
- A korrekció a szintézis és FX után, a meglévő sztereó csúcsvédelem előtt
  történik. A Drive bemenete, telítése, FIR-je, filteregyütthatók és az
  effektek gerjesztése változatlan.
- Dry filterállásnál az erősítés pontosan 1; a köztes Filter Mixhez
  lineáris erősítéskeverés tartozik, 5 ms-os simítással. Nincs cutoff-
  vagy jelszintfüggő AGC, kompresszor vagy új kezelőszerv.
- A +3 dB alatti lineáris mintakapcsolatot külön, korrekció nélküli Preview
  engine-hez mérjük, minden módban 44,1/48/96/192 kHz-en, 0/50/100% mixszel.
  A dry kimenet bitazonos; a wet és köztes utak csak skaláris erősítést kapnak,
  amíg a meglévő csúcsvédelem nem avatkozik be.
- Ez a külön Preview korrekciója. A shipping Classic és a kutatási
  filter-/Drive-referenciák változatlanok; nincs production-promóció.

## Factory presetek: a védelem ne takarja el a problémát

A +3 dB hozzáadása factory-kalibráció nélkül a helyi gridben akár
**+7,51 dBFS** csúcsvédelem előtti csúcsot adott. Ez nem a WAV-on mért
clipping: a normál engine ilyenkor korlátoz. A méréshez külön, a VST3-ba
nem linkelt engine-másolat kikapcsolja ezt a védelmet, hogy a presetpróba
ne tudjon a limiter mögé rejtőzni.

A 24 Preview factory preset csak Output/Volume csillapítást kap, külön
kulcshoz rögzített táblával; a Boost és a többi hangparaméter megmarad.
A szükséges csillapítás 0–15 dB, a helyi legrosszabb mért csúcsból felfelé
egész dB-re kerekítve, -6,5 dBFS céllal. A halkabb preseteket nem emeljük
egységes hangosságra. Az új pluginpéldány a kalibrált Init hanggal indul.

A régi projektek és importált/saját presetek mentett Output/Boost értékei
nem íródnak át. Hangjuk teljes wet állásban +3 dB-lel nőhet; az új factory
Volume-értékek csak egy factory preset újbóli betöltésekor lépnek életbe.
A korábbi factory-értékekkel mentett hang ezért Customként jelenhet meg.
Régi projekt betöltésekor a korábbi limiter működése megmarad.

## Helyi mérés és elfogadási határ

- AppleClang 21.0.0 (`clang-2100.3.34.2`), macOS arm64, CMake Release.
- 24 preset × 44,1/48 kHz × 1/8/16 lenyomott MIDI-gyökér = **144 cella**.
- Velocity 127, 136 BPM; hangok: MIDI 48,52,55,57,59,62,64,67,69,71,74,76,79,81,83,86.
- Négy másodperc kitartás és két másodperc lecsengés; a preset saját
  voice módja, ARP-ja, modulációja és FX-beállításai aktívak. A mono/ARP
  preset nem szükségképpen szól 16 aktív voice-szal 16 lenyomott gyökérnél.
- Átmenő kapu: minden minta véges; minden factory cella csúcsvédelem
  előtti csúcsa legfeljebb **-6 dBFS**.
- Eredmény: **144/144 megfelelt**, legnagyobb csúcs **-6,51293 dBFS**
  (`supersaw_one`, 48 kHz, 16 gyökér).
- Külön maximális Output/24 dB Boost stressz ellenőrzi a normál Preview
  meglévő 0,98-as sztereó csúcsvédelmét négy mintavételi frekvencián.

[Nyers helyi CSV](measurements/premium-preview-levels-2026-10-15-local-arm64.csv).
SHA-256: `c66789ab4445891b02338ac387008bbc96168c89567c7d36cc329aef69c0b9e7`.

A grid egy megadott hangterjedelmet és szekvenciát fed le. Nem minden
regiszter, automatizálás, hosszú sustain/feedback vagy kézzel felhangosított
preset bizonyítása; nem intersample/true-peak mérés, nem DAW-master-
clippinggarancia és nem CPU-mérés. A normál csúcsvédelem továbbra is kell.
A végleges factory library szélesebb zenei újrahallgatása és szintellenőrzése
a production-integráció után is külön feladat.

## Reprodukció és CI

```sh
cmake -S . -B build-premium -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DSAWSTAR_PREMIUM_PREVIEW=ON
cmake --build build-premium --target sawstar_premium_preview_smoke sawstar_premium_preview_levels --parallel 3
build-premium/sawstar_premium_preview_smoke
build-premium/sawstar_premium_preview_levels > premium-levels.csv
```

A macOS Preview workflow natív arm64 és Rosetta x86_64 szeleten is futtatja
a szint- és headroom-kaput, a CSV-ket külön artifactban megőrzi, és csak
siker után validálja/csomagolja a VST3-at. A PR és az új csomag CI-állapota
külön ellenőrizendő; a helyi PASS nem állítja a natív REAPER-próba lezárását.
