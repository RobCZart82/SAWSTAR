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
