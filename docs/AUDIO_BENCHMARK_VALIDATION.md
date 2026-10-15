# Production audio-benchmark CSV-szerződés

A `scripts/benchmark-audio.py` a rögzített production baseline és a jelenlegi
motor nyolc, váltakozó sorrendű mérési párját hasonlítja össze. Ez külön
ellenőrzés a kísérleti prémium filter célgépes kampányától.

A korábbi parser elfogadta a nem üres, véges értékű CSV-részhalmazokat.
Ha mindkét útból ugyanaz a jelenet hiányzott, a körök és a két motor
kulcshalmazának összevetése egyezést mutatott, így hiányos mérés is
sikeresnek tűnhetett. Az új ellenőrzés minden warmup és időzített futásnál
pontosan hat jelenetet követel: `dry_fx` és `full_fx`, mindkettő 1/8/16 voice.
Hiányzó vagy ismeretlen jelenet, duplikáció, hibás fejléc/sorszélesség,
érvénytelen szám, nem véges érték vagy nem pozitív CPU-idő hibát jelent.
A teljes CSV sorainak sorrendje tetszőleges; a metrikák előjele nem korlátozott
a pozitív CPU-idő kivételével (negatív dBFS/DC érvényes).

A jelenlegi motor minden warmup és időzített renderének DC-értékét külön
ellenőrizzük: `abs(dc) <= 0.01`. Egy hibás kör nem tűnhet el a mediánban;
például hét nulla és egy 0,2 DC-jű kör mediánja ugyan nulla, de a mérés hibás.
A ±0,01 határérték megengedett. A régi baseline nem kap új, abszolút
DC-elfogadási feltételt; a jelenlegi kimenet biztonságát vizsgáljuk.

Nem változik a DSP, baseline, workload, warmup, ismétlésszám vagy a küszöb:
a CPU-regresszió akkor hibás, ha egyszerre több mint +25% és +1,0 realtime
százalékpont. A ±1 dB szint- és ±0,01 DC-korlát is megmarad. Nincs automatikus
újrapróbálás vagy kedvezőbb mérési kör kiválasztása.

A `benchmark_report` CTest a hiányos warmup/időzített CSV-ket és parserhibákat
szintetikus adatokkal ellenőrzi. Ez adatérvényességi javítás, nem CPU-gyorsulás,
natív REAPER-elfogadás vagy a prémium filter CPU-kapujának lezárása.

## Kutatási deadline CSV-k

A `report-premium-deadline.py` a `summary.csv`, `blocks.csv` és `factors.csv`
fájlokon pontos, egyedi oszlopkészletet és minden sornál egyező mezőszámot
követel. Az oszlopsorrend tetszőleges. Duplikált vagy hiányzó oszlop, idegen
mező és hibás CSV-idézés hibát jelent, a nyers ellenőrzési módban is.
Az ismételt oszlopnév többé nem fedhet el hibás eredeti értéket. A numerikus
gridek, párosítás, percentilisek és elfogadási küszöbök változatlanok.

## Megőrzött CI-megfigyelés

A `1ed6d698` commit külön macOS push-futásán a `dry_fx / 16 voice`
CPU-ellenőrzés hibát jelzett: baseline 11,6468%, current 15,0679%,
határ 14,5584%. [Eredeti futás](https://github.com/RobCZart82/SAWSTAR/actions/runs/38620306599).
Ugyanezen commit PR-futásai sikeresek voltak. Ez eltérő runnereredmény,
nem igazolt DSP-gyorsulás vagy bizonyítottan zajos mérés. A küszöb nem változik,
az eredeti hibát nem írja felül egy későbbi sikeres futás.



## A #104 utáni main CPU-hibája — 2026-10-15

A `3ae88958910a647228602c6e97cf86a1d90930d7` main
[macOS Release jobja](https://github.com/RobCZart82/SAWSTAR/actions/runs/38777419785/job/116568558159)
a `dry_fx / 8 voice` CPU-kapun állt meg. Az előző,
`3c8b31631df28d4f483a67a256360a58cc0e6911` main
[azonos jobja](https://github.com/RobCZart82/SAWSTAR/actions/runs/38765437361/job/116527282882)
sikeres volt.

| Futás | Referencia audioidő (%) | Jelenlegi audioidő (%) | Relatív eltérés |
| --- | ---: | ---: | ---: |
| Előző sikeres main, 8 voice dry_fx | 6,607206 | 7,098519 | +7,44% |
| #104 utáni main, 8 voice dry_fx | 5,991469 | 7,566858 | +26,29% |

A hibás futás határa 7,489336%; a 7,566858% ezt 0,077522
audioidő-százalékponttal lépi túl. A relatív és abszolút küszöb egyaránt
érvényes; a különbség kis határtúllépése nem ok a kapu megkerülésére.
A táblázat offline audioidő, nem a REAPER CPU-kijelzője.

A két main commit közvetlen fájleltérése kizárólag
`docs/DEVELOPMENT_ROADMAP.md` és `docs/GUI_SCALE_PROJECT_STATE.md`.
A benchmark munkaterhelése, kódja, workflow-ja és a production DSP nem
változott. Az összesített audio-metrikák a naplóban közölt pontosságig
egyeznek: mind a hat jelenet peak/RMS/DC adatai azonosak a két futásban.
Ez az időmérés változékonyságát mutatja, de nem bizonyítja a konkrét okot
vagy a regression check téves riasztását. Az archivált production
referenciához viszonyított tényleges regresszió lehetősége nyitott.

A funkcionális Release suite, source quality, engine audit, macOS Debug,
Universal VST3, Windows, Code quality és workflow-validálás sikeres.
A CPU-ellenőrzés hibája ettől érvényben marad.

**Állapot:** a soros merge megállítva, automatizmus szüneteltetve.
Nincs rerun kedvezőbb mérésért, baselinecsere vagy küszöblazítás.
A #105 mainre frissítése és friss PR-CI-je integrációs ellenőrzés,
nem a hibás main-mérés törlése vagy CPU-elfogadás.

Következő diagnosztikai feladat: a két bináris buildazonosságának,
fordító-/optimalizálási beállításainak és mérési környezetének rögzítése,
azonos binárisú kontrollal és minden nyers kör megőrzésével.
A meglévő nyolc mérési pár, ±1 dB szint-, ±0,01 DC- és CPU-kapu maradjon meg.
A Premium hangerőkorrekció elfogadása ettől külön, lezárt fejlesztési döntés.
