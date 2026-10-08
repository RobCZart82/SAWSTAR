# Oszcillátor frekvenciaszámítás megosztása

A korábbi cache-jelölt modulált Windows SAW-nál lassult, ezért production
bevezetése elhalasztott. Ez a külön kutatás ugyanazon mintában számítja ki
és osztja meg a frekvenciát az aktív hullámformák között. A shipping
SevenSaw, a teljes motor és a filter minőség/rátapolicy változatlan.

## Algoritmus és referencia

A `SharedFrequencySevenSaw` a main `e830145` SevenSaw algoritmusát követi.
A detune simítása után egyszer számolja ki a `min(hz*pitch*ratio, rate*0.45)`
értéket sávonként. Ezt kapja a SAW és az aktív alternatív hullámformák setterje;
a háromszög integrátora is ugyanebből számít fázislépést. A műveleti sorrend,
fázisok, hullámforma-súlyok, detune, normalizálás és pan megmaradnak.
Nincs minták közötti cache, plusz állapot vagy változásfigyelő ág.
A referencia és jelölt objektummérete fordításkor ellenőrzötten azonos.

Ez optimalizálási hipotézis: a compiler már megoszthatja az eredeti számítást,
és a kódelrendezés önmagában is változtathat időt. Egyetlen kedvező medián
nem indokol teljesmotoros vagy production aktiválást.

## Numerikus és mérési kapuk

Az `oscillator_shared_bit_identity` 96 jelenetben, 795 648 sztereó frame-en
követel float-byte egyezést. Hat ráta (8–384 kHz), négy hullámforma, statikus,
lépcsős, clamp/hibás vezérlő és folyamatos pitch-moduláció szerepel, továbbá
Snap, reinit, élő állapot másolása és hullámformaváltás. A jeleneteknek szólniuk
kell; a negatív kontroll szándékos hangeltérést utasít el.

A benchmark 32 oszcillátorbankot (16 voice OSC1+OSC2) mér, három rátán,
négy hullámformával, statikus és audio-rate pitch mellett, négy váltakozó
párral. Init és 0,25 s warmup kívül, 8192 frame bankfeldolgozás, pitch-setter
és energiadiagnosztika belül van a timeren. Ez izolált költség, nem teljesmotor
vagy REAPER callback. A 192 soros CSV compiled `shared-frequency-v1` markerét
a reporter külön ellenőrzi; a cache v2 eredménye nem címkézhető át erre.

A Windows/macOS Release workflow a bitazonossági és riportkontroll után
méri a párokat. Nyers CSV, forrás/compiler hash, kontraktusnapló és minden
lassabb cella megmarad. Nincs CPU pass/fail küszöb. Teljesmotoros és
production bevezetés kizárólag későbbi elfogadással történhet.

## Következő döntés

Először helyi Release/sanitizer és platformos eredmények. Következetes
izolált nyereség esetén külön lookup-motor kontroll és modulált deadline
vizsgálat következik; vegyes vagy rosszabb eredménynél a jelölt kísérleti marad.
A CPU/minőség kapu, a natív REAPER-próba és a tényleges filtercsere nyitott.
Linux/32 voice halasztott; a lezárt click/pop kutatás nem nyílik újra.

## Helyi numerikus ellenőrzés

Mac mini M1, Apple Clang Release: a 96 jelenet / 795 648 frame bitazonos.
A shipping SevenSaw, a korábbi cache és a hét Python riportteszt is sikeres.
A külön új jelölt ASan/UBSan alatt is megfelelt. A CPU-adat és a platformos
CI külön következő eredmény; ezek nélkül nincs teljesmotoros aktiválás.

## Két helyi CPU-mérési kör

Mért forrás: `d8513f9fbcde08a5783edc46dfc7236525f186f1`, Mac mini M1,
Apple Clang 21.0.0, Release `-O3 -DNDEBUG`. Mind a 118 Release teszt sikeres.
Két egymás utáni kampány ugyanazt az executable-t használta, fordítás és
tesztfuttatás befejezése után; nincs CPU-affinitás vagy realtime prioritás.
A háttérben futó OS-munkák nem kontrolláltak.

Mindkét 192 soros gridben pontosan egyezett a páros energia, és a 24 cella
medián study/reference időaránya egy alatt volt. A két kampány celláinak
szélsőértékei, három ráta és statikus/modulált pitch szerint:

| Hullámforma | Medián időarány tartománya |
| --- | ---: |
| SAW | 0,980374–0,997881 |
| Square | 0,903664–0,973859 |
| Triangle | 0,870323–0,992741 |
| Sine | 0,972130–0,986670 |

A SAW út eleve csak egyszer számolta a frekvenciát, ezért kis eltérése
kódelrendezési/mérési kontroll, nem az új megosztás bizonyított nyeresége.
Egyes párok lassabbak voltak; a teljes páros adatok megmaradnak, a medián
nem szignifikanciateszt. A korábbi cache Windows-eredménye nem vihető át
az új algoritmusra. Friss Windows/macOS CI és kontrollált platformismétlés
kell a továbblépéshez. Ez nem teljesmotoros CPU- vagy natív elfogadás.

[Nyers első kör](../experiments/oscillator/measurements/2026-10-08-shared-frequency-local-m1-first.csv),
[ismétlés](../experiments/oscillator/measurements/2026-10-08-shared-frequency-local-m1-repeat.csv)
és [eredetjegyzék](../experiments/oscillator/measurements/2026-10-08-shared-frequency-local-m1-provenance.json).
A két JSON-riport mind a 24 cellát és négy páros arányát megőrzi.

## Platformos rövid mérés

A #88 beolvadt; az `eae61b720ba3b5f1c1aacc49936d9701580984f8` PR-head
mind a 47 CI-ellenőrzése sikeres. A külön CPU-mérés
[37829386024](https://github.com/RobCZart82/SAWSTAR/actions/runs/37829386024)
forrása ugyanez. Mindkét teljes 192 soros CSV páros energiája egyezik;
a letöltött CSV és compiler byte-hash egyezik a futam riportjával.

| Platform | Mediánban lassabb cella | Minden párban lassabb cella |
| --- | ---: | ---: |
| Windows 2022 AMD64 | 21 / 24 | 13 / 24 |
| macOS 14 arm64 | 13 / 24 | 0 / 24 |

Windows alatt a statikus SAW mediánja 4,22–5,68%, a modulált SAW-é
9,05–10,00% lassulás. A SAW-ban nincs elhagyott ismételt számítás: ez
a kódváltozat tényleges megfigyelt költsége, de az okát az időarány nem
azonosítja. A macOS párok széles tartományban szóródnak, ezért a korábbi
kedvező helyi M1-medián nem általánosítható. A funkcionális CI zöld, a
CPU-elfogadás nyitott; a jelölt nem kerül a teljes motorba.

A teljes bizonyíték és a negatív eredmények megmaradnak:
[eredetjegyzék](../experiments/oscillator/measurements/2026-10-08-shared-frequency-ci/provenance.json),
[Windows-riport](../experiments/oscillator/measurements/2026-10-08-shared-frequency-ci/windows/report.json),
[macOS-riport](../experiments/oscillator/measurements/2026-10-08-shared-frequency-ci/macos/report.json).
A fájlok sorvége is megőrzött; a regresszió ellenőrzi a hash-eket és a páros arányokat.

## Hosszabb mérés és változatlan referencia

Az `--extended` kampány ugyanazon ráták/hullámformák/pitch-terhelések mellett
131 072 frame-et mér nyolc váltakozó párban: 384 nyers sor, 24 cella.
Init és 0,25 s warmup továbbra is kívül van a timeren. A korábbi default
8192 frame / négy pár / 192 sor változatlanul futtatható. A rövid és hosszú
kampány nem egyesíthető egy eredményrácsba.

A `--extended --reference-repeat` mindkét útján ugyanaz a SevenSaw és
ugyanaz a Measure-függvény fut ugyanazon executable-ben. Ez időzítési
kontroll: a megfigyelt eltérés nem jelöltgyorsulás. A jelöltfutás külön
`comparison=candidate` markerrel következik. A reporter ellenőrzi a kampányt,
a 131 072 frame-et, a nyolc párt, a warmupot, a teljes rácsot és a páros
energiát; a schema 3 minden arányt és a cella minimum/maximum arányát is
megőrzi. A régi schema 2 riportolás megmarad.

A Windows/macOS workflow a kontrollt és a jelöltet egymás után futtatja,
fordítás és numerikus ellenőrzés után. A hosszabb mérési idő csökkenti a
rövid timer érzékenységét, de CPU-affinitás, realtime prioritás és kontrollált
OS-háttérterhelés nincs. Nincs párok eldobása vagy a kontroll automatikus
kivonása; a kontroll nem számszerű hibakorlát. Az izolált grid továbbra sem
helyettesít teljesmotoros p99/túllépés vagy natív REAPER-elfogadást.
