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

## A mérési bizonyíték automatikus megőrzése

A `benchmark-audio.py --evidence-dir UJ_KONYVTAR` új, korábban nem létező
könyvtárba menti mindkét warmup és az összes nyolc mérési pár CSV-kimenetét,
még a parser/DC ellenőrzése előtt. Sikertelen folyamat részleges stdoutja
és exitkódja is megmarad; nincs továbblépés vagy automatikus újrapróbálás.
A fájlok SHA-256 hashét és a tényleges mérési sorrendet `manifest.json`+rögzíti. Meglévő könyvtárat elutasítunk, így egy új próbálkozás nem írja
felül a korábbi bizonyítékot. Megszakított futás `incomplete`, kivételes
kilépés `failed`, teljes siker `success` állapotú.

A manifest tartalmazza a current/baseline commitot, munkaterheléshasht,
binárishasheket, CMake fordítót/verziót és Release flag-eket, valamint az
operációs rendszer/architektúra és CI-futás azonosítóit. Csak felsorolt
CI-környezeti mezőket olvasunk ki; teljes környezetet vagy tetszőleges
CMake-cache-bejegyzéseket nem exportálunk. Makefile-generátornál a motor,
DaisySP és benchmark célok flags.make fájljait is rögzítjük.

A Windows/macOS Release workflow `always()` feltétellel, 90 napos artifactba
teszi az adatokat sikertelen összevetés után is. A hash belső azonossági
bizonyíték, nem önálló hitelességi vagy realtime-elfogadási garancia.

Ez diagnosztikai fejlesztés: a baseline, render, warmupok, nyolc pár,
kiegyensúlyozott sorrend, mediánösszevetés és minden küszöb változatlan.
Az evidencia írása a mért folyamatokon kívül történik. A korábbi piros
main-mérés okát önmagában nem bizonyítja, és nem törli annak blokkoló
állapotát. A friss PR sikeres CI-je sem helyettesíti az ok tisztázását.

A `benchmark_report` regressziói ellenőrzik az összes próbálkozás sorrendjét
és byte-hashét, a hibás warmup/időzített CSV megőrzését elutasítás előtt,
a sikertelen folyamat azonnali leállítását és részadatát, a korábbi
könyvtár felülírásának tiltását, valamint a fordítási adatok szűrését.

## Igazolt build-policy eltérés — 2026-10-15

A javított `ad8aa7ff` PR-head mind az öt workflow-ja sikeres. A hozzá
tartozó `eb80f7aa` ideiglenes PR-merge commit Windows/macOS mérési
archívuma egyenként 18 kimenetet tartalmaz: két warmup és nyolc mérési pár.
Mind a 36 fájl manifest szerinti hashét, valamint az artifact ZIP-ek
GitHub által közölt SHA-256 digestjét függetlenül ellenőriztük.

| Platform | Run | Artifact | ZIP SHA-256 |
| --- | --- | --- | --- |
| Windows | 38794231782 | 12001921891 | `0bc2ffd279e80fac937d3ad4f8c8b72daca56b95a90ea35249be4ee7addd1c8a` |
| macOS | 38794231714 | 12001692369 | `904b4bb9f22f39511d7df0bc415826c5098c1f5b9e9cebb3caec352124862343` |

A munkaterhelés SHA-256 mindkét platformon
`ccdcd4533fc0f5d298da5d0e0a1ba2ef7f91e00405182ea785751601609e006e`.
A referencia commit továbbra `0873064487e02b2e1da49b42d0fd12ca83e11ec7`.
Windows: mindkét út MSVC 19.44.35229.0, `/O2 /Ob2 /DNDEBUG`.
macOS: mindkét út AppleClang 15.0.0.15000309, `-O3 -DNDEBUG`, arm64.
Viszont a baseline `CMAKE_OSX_DEPLOYMENT_TARGET` üres, a current értéke
`11.0`; csak a current parancsban szerepel `-mmacosx-version-min=11.0`.
Ez igazolt összehasonlíthatósági hiba, nem a korábbi piros CPU-arány
bizonyított kizárólagos oka. A két friss gyűjtés sikere nem törli a hibát.

A baseline konfigurálása most a current CMake cache-ből örökli a macOS
célverziót és az explicit architektúrákat. Időzítés előtt külön kapu
követeli a build típusának, generátornak, globális/Release C++ flag-eknek,
macOS célverziónak és architektúráknak az egyezését. Eltérésnél a mérés
el sem indul; az addigi manifest megmarad. A fordítóazonosítók és a
célonkénti flag-ek továbbra is külön, olvasható bizonyítékok.

Nincs baseline-forráscsere, kimeneti normalizálás, DSP-módosítás,
küszöblazítás vagy kedvező kör kiválasztása. Az új mérés indoka a
bizonyított build-policy javítása. A régi futások változatlanul maradnak.
A helyi regresszió a valóban megfigyelt üres/11.0 eltérést elutasítja,
az egyező policy-t elfogadja, és a `-O0`/`-O3` eltérést is észleli.
A Windows út nem kap mesterséges macOS opciót. A CPU-/natív elfogadás
és a main korábbi hibájának végső lezárása továbbra is külön kapu.
