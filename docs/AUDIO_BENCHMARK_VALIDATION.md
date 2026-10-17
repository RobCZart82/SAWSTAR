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
A fájlok SHA-256 hashét és a tényleges mérési sorrendet `manifest.json`
rögzíti. Meglévő könyvtárat elutasítunk, így egy új próbálkozás nem írja
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

## Azonos binárisú A/A kontroll — 2026-10-16

A #105 `66f41f8d` head macOS production benchmarkja ismét hibázott:
`dry_fx / 8 voice`, baseline 6,162273%, current 7,845225%, +27,31%.
A main ugyanazon két binárissal sikeres volt. A független archívumellenőrzés
mindkét oldalon 18 teljes CSV-t, a párosítás sorrendjét és a ZIP/fájlhashokat
ellenőrizte; az egyik futás current binárisa a másik current binárisával,
valamint a két baseline bináris egymással byte-azonos:

- main artifact: 12032371927, ZIP SHA-256 `2c105eabecd82d5064f86f413f41ad737ccd74e4080ead0147ee1d16426f01d9`;
- PR artifact: 12033055245, ZIP SHA-256 `832555ac684b171952ec99bfc9a65aa6f46ccb6bf6ea5bd2622109a8668aa138`;
- baseline executable: `b07e537e3da257e2cc94cef118e6fac3eca4cc3715b34699759e5090eb4e866a`;
- current executable: `e2d492f9b3271050ea7d64e797fddb2b2d8bb0dcd6957478981798e01c255de7`.

Az új `scripts/control-audio-timing.py` **egyetlen abszolút executable
útvonalat** futtat mindkét címkével. Ugyanaz a két warmup és nyolc,
fordított sorrendű pár, teljes hat jelenettel; minden nyers jelentés
megmarad. A bináris hashét a futás előtt és után ellenőrzi; a módosult
bináris és az eltérő peak/RMS/DC kimenet hibás kontroll. Az új könyvtár
követelménye megakadályozza a régi adatok felülírását.

A kontroll az eredeti +25% ÉS +1 audioidő-százalékpontos határt használja,
sikertelenségnél nem ismétel és nem szűr. Eredménye diagnosztika: még
siker esetén is `production_regression_accepted=false` és
`native_cpu_accepted=false`. Nem törli a korábbi hibás production futást,
és nem helyettesíti annak sikeres pontos-head CI-kapuját.

Első helyi arm64 kontroll: mind a hat jelenet határon belül, mediáneltérés
-0,2068% és +0,1200% között; `dry_fx / 8 voice`: 5,64910835% és
5,65141460%, relatív +0,040825%. A helyi Classic könyvtárakból linkelt
bináris nem a CI runner executable-je; nincs ilyen eredet- vagy célgépes
elfogadási állítás. A [nyers helyi bizonyíték](measurements/production-aa-local-arm64-2026-10-16.json)
a fordítót, a linkparancsot, workload-/könyvtár-/binárishashokat, az összes
18 nyers CSV-t és az eredményeket együtt őrzi meg. Ez egy megfigyelés,
nem általános stabilitási garancia.

A külön `Production identical-binary timing control` workflow friss
Windows/macOS Release builddel futtatja a kontrollt; a kontrollfolyamatok
platformonként szigorúan sorosak. A siker és hiba egyaránt 90 napos
artifactba kerül. A normál production benchmark és küszöbei változatlanok.
A négy új teszt bizonyítja az egyetlen bináris használatát, a fix 18
hívást, az összes eredmény megőrzését és a hibás kontroll elutasítását.


## Új negatív main-eredmény — 2026-10-17

A #110 squash utáni main `8ad1b835690ce0486e7bdbaad79551da59737874`
[macOS Release futása](https://github.com/RobCZart82/SAWSTAR/actions/runs/39008233417/job/117325007221)
a `dry_fx / 16 voice` esetben hibázott: baseline 11,1816896%, current
14,14344585%, +26,4876%; a megengedett határ 13,977112%. A Universal
plugin-build, Debug és a többi fő workflow sikeres, de a main kapuja nem zöld.

A [PR-futás](https://github.com/RobCZart82/SAWSTAR/actions/runs/39005768567)
ugyanezen esetben 11,01943125% → 12,2346417% (+11,0279%) eredménnyel
sikeres volt. A két artifact ZIP-digestje és mind a 36 nyers CSV hash-e
ellenőrizve; minden hatjelenetes warmup/timed jelentés, a nyolc fordított
sorrendű pár, a szintek és a production medián/hard-limit újraszámolva.
A main hibáját a független audit is ugyanúgy visszaadja.

Mindkét futás ugyanazt a baseline-hash-t (`b07e537e...`) és ugyanazt a
current-hash-t (`e2d492f9...`) rögzíti; ez a korábban dokumentált binárispár.
A két motor egymással nem azonos. Az eredmény futások közötti szóródást
bizonyít, nem a baseline/current különbség kizárólagos okát, nem CPU-elfogadást.
A PR tesztmerge commitja és a main squash commitja külön azonosság;
a manifestek mindkettőt az eredeti értékekkel őrzik.

[Teljes tartós bizonyíték](measurements/production-ci-variance-2026-10-17.json):
artifactazonosítók, teljes ZIP-hashok, eredeti manifest-szövegek, compiler/flags,
18+18 eredeti CSV, és újraszámolt eredmények. Nem csak a kedvező kör marad meg.
A binárisbyte-ok nincsenek ebben az artifactban: a hashok rögzített azonosságot,
nem önálló eredethitelesítést jelentenek.

Az új `scripts/review-audio-benchmark.py EVIDENCE_DIR` csak olvas:
nem fordít, nem mér, nem írja át a fájlokat és nem indít retry-t. Elutasítja
az átírt hashokat, hiányos párokat, hibás sorrendet/CSV-t, extra fájlokat,
path-escape-et, módosított küszöböt és a számított döntéssel ellentétes státuszt.
Exit 0 a nyers bizonyíték konzisztenciáját jelenti: a JSON-ban a hibás
benchmark továbbra `regression_detected=true`, `recorded_status=failed`.
Nem alkalmas hiányos, végrehajtási hibával megszakadt kampány sikeresre
minősítésére. Az eredeti production gate, mérési eljárás és küszöb változatlan.

Nem indult új Mac/Windows célgépes kampány vagy a hibás main újrapróbálása.
A pontos ok még nyitott; a következő merge/release előtt szabályosan rendezni
kell a main hibáját. A ráta-/pufferpolicy és a natív REAPER-próba ettől külön
nyitott, CPU-/native-/production-elfogadás nélkül.

### Futáspáronkénti diagnosztika — új mérés nélkül

Az auditor a nyolc eredeti időzített párt külön is kiírja: ismétlésszám,
elsőként futó motor, mindkét CPU-adat, relatív eltérés és a változatlan
CPU-határ páronkénti túllépése. A két motor minimuma/maximuma és a teljes
tartomány saját mediánhoz viszonyított mérete is megmarad. Ez tartomány,
nem szórásbecslés, p99 vagy realtime dropout. Nincs körök eldobása.

| dry_fx / 16 voice | PR #110 | main #110 |
| --- | ---: | ---: |
| Baseline minimum–maximum (%) | 10,686825–11,6510542 | 10,6453916–15,6788125 |
| Current minimum–maximum (%) | 11,1737125–13,5829459 | 11,3804542–17,1935 |
| Baseline tartomány / saját medián | 8,75% | 45,01% |
| Current tartomány / saját medián | 19,69% | 41,10% |
| Páronként határt túllépő eset | 1/8 | 2/8 |
| Eredeti production döntés | sikeres | **hibás** |

A main 1. párjában +41,42%, a 6. párjában +59,88% az eltérés; a két
kiugrás eltérő futási sorrendben keletkezett. A baseline 4. és 8. köre is
lassabb volt. Ezek megfigyelések; a runner terhelését vagy a konkrét okot
nem igazolják. A benchmark nem az egyedi arányok többségét, hanem a két
motor nyolc értékből számított külön mediánját hasonlítja: a main +26,49%-os
hibája érvényben marad. Az új regressziós teszt az eredeti nyolc-nyolc
értékkel tiltja a páronkénti többségre alapozott utólagos elfogadást.

### Eltelt idő és gyermekfolyamat CPU-ideje

A gyűjtő `--evidence-dir` használatakor minden meglévő benchmark-hívás
köré két szülőoldali időpontot és POSIX rendszereken két
`getrusage(RUSAGE_CHILDREN)` pillanatképet tesz. Az `invocation_timing`
rekordban a teljes hívás eltelt ideje és a gyermek user/system CPU-idejének
különbsége szerepel. A gyűjtő egy benchmarkot futtat egyszerre; ebben az
intervallumban nem indít fordítást vagy másik gyermekfolyamatot. A szülő
parserének és a bizonyíték írásának ideje kívül marad. Sikertelen
gyermekfolyamat részleges CSV-je, exitkódja és időadata is megmarad.

Ez a teljes folyamat ideje: indulás, mind a hat jelenet inicializálása,
warmupja és renderelése. **Nem jelenetenkénti CPU-idő**, nem REAPER CPU%,
és az eltelt idővel való különbsége önmagában nem bizonyít runnerzajt.
Más gyermekfolyamatot párhuzamosan indító jövőbeli gyűjtőben az összesített
POSIX különbség nem lenne elkülönített bizonyíték. Windows alatt a POSIX
adat nem érhető el: `child_cpu_available=false`, a CPU-mezők hiányoznak;
nem írunk helyettük nullát vagy a szülő Python CPU-idejét.

Az időbélyegek a benchmark-folyamaton kívül készülnek; a C++ workload,
CSV-formátum, két warmup, nyolc pár, sorrend és küszöb változatlan.
Az új adat nem módosítja a production döntést és nem enged újrapróbálást.
Az auditor a régi, időadat nélküli archívumokat is ellenőrzi, az új
időadatokat külön megőrzi, hibás/nem véges/negatív időt és „nem elérhető”
állapot mellett kitalált CPU-adatot elutasít.

A korábbi main-artifactban ilyen adat nincs; utólag nem pótoljuk.
A következő rendes PR-CI adhat új diagnosztikát, a régi hibás main futást
nem indítjuk újra. CPU-elfogadás és a konkrét hibaok bizonyítása nyitott.
