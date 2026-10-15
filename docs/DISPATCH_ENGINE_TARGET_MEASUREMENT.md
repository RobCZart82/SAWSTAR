# Teljesmotoros SAW-dispatch célgépes gyűjtés

## Teljes hosted CI-kampány — 2026-10-15

A `.github/workflows/dispatch-cpu-campaign.yml` a meglévő gyűjtőt futtatja
változatlan workload/numerikus szerződéssel Windows 2022 és macOS 14 runnereken.
Platformonként egyetlen job egymás után méri mind a 32 gridet: két kör,
külön referenciaismétlés/jelölt, két terhelés és négy hullámforma.
Minden platform 2 359 296 nyers blokkot gyűjt, a két platform együtt
4 718 592-t. A collector 90 perces jobkeretet kap; nincs automatikus retry
vagy kedvezőbb kör kiválasztása. A workflow új commit miatt nem szakít meg
egy már futó kampányt.

A numerikus kvalifikáció az első mérés előtt fut. A lezárt archívumot
platformonként újraellenőrizzük `--verify-raw` módban; a külön összevető job
mindkét artifact letöltése után ismét ugyanazt a nyers validátort használja.
Az összevetés eltérő forráscommitot/forráshashokat vagy hiányos archívumot
nem fogad el. A review és comparison kimenete mindig a mérési archívumon
kívül készül, hogy ne változtassa meg annak fájlkészletét.

Artifactok: `dispatch-cpu-windows-2022`, `dispatch-cpu-macos-14`,
`dispatch-cpu-comparison`. Megőrzésük 90 nap; az első kettő hibás gyűjtéskor
is feltöltődik, ha már keletkezett adat. A comparison JSON minden kört,
kontrollt, workloadot, hullámformát, rátát és puffert külön tart meg.
A futási napló `DISPATCH_CPU_CAMPAIGN=` sora ugyanennek a JSON-nak a másolata.

A workflow saját fájlját érintő PR-en automatikusan, később az Actions
`Run workflow` lehetőségével, a kívánt forráság kiválasztásával indítható.
Ez a ritka, hosszú kampány nem fut automatikusan minden DSP-PR-en;
a meglévő rövidebb kutatási workflow-k változatlanul megmaradnak.

**Értelmezés:** a zöld workflow a teljes adatgyűjtést és validálást igazolja,
nem a jelölt CPU-elfogadását. A p50/p99 arányoknál az 1 feletti érték lassulás;
a referencia és jelölt >100%-os blokkjai külön értékelendők. A referencia-
ismétlés szóródását nem vonjuk le a jelöltből. A körökből, hullámformákból
és terhelésekből nem képzünk kedvezőbb közös mediánt. A nyers sorok a négy
filtermód külön vizsgálatához is megmaradnak.

A hosted runner hardvercímkéje nem hitelesít saját M1 célgépet, és a manifest
továbbra is `background_load_controlled=false`, `native_host_acceptance=false`,
`production_promotion_allowed=false` értékeket tartalmaz. A célgépes CPU- és
natív elfogadás ezekből az adatokból önmagában nem zárható le.

A `scripts/run-dispatch-engine-target.py` a roadmap 3. lépésének ismétlési
kapuját támogatja. A #93 teljesmotoros kampánya elkészült és beolvadt;
a kedvező SAW-medián mellett a p99/túllépés és az alternatív hullámformák
CPU-elfogadása nem teljesült. Az új gyűjtő nem új DSP-optimalizálás:
a változatlan lookup referencia és a dispatch jelölt megismétlését segíti.

## Futtatás saját Windows vagy M1 gépen

Python 3, Git, CMake/CTest és C++17 fordító szükséges (Windows: Visual Studio
C++ Build Tools és SDK; macOS: Xcode Command Line Tools). A kiválasztott commit
checkoutja és a rögzített DaisySP submodule legyen tiszta, inicializált.
A gyűjtő elutasítja a módosított vagy nem követett fájlokat és a hibás DaisySP
commitot; nem frissít függőséget. REAPER ehhez az offline méréshez nem kell.

Zárd be a CPU-t terhelő alkalmazásokat, állítsd le a DAW lejátszását, és
használd ugyanazt az energiaellátási beállítást a két gépes ismétlésnél.
A gyűjtő nem állít affinitást, prioritást vagy energiagazdálkodást; a manifest
nem állítja, hogy a háttérterhelést automatikusan kontrollálta.

A repository gyökerében, két új, checkouton kívüli, egymástól külön mappával:

```powershell
python scripts/run-dispatch-engine-target.py --build-dir ../dispatch-engine-build-01 --output-dir ../dispatch-engine-results-01
```

macOS-en `python3` használható. A `--cmake` és `--ctest` opció abszolút
programutat is fogad. A `--rounds` 2–10 közötti egész szám, alapértéke 2.
A teljes gyűjtés hosszú: 32 teljes grid készül alapértelmezésben.

## Mérési és numerikus szerződés

Friss Release build készül a teljesmotoros és banki bitazonossági targetekkel,
a deadline-fixture-rel és a független lookup complex kontrollal. Minden
numerikus CTest, a külön referenciaismétlés-modulációs kontroll, az 8× complex
kontroll és mind a négy waveform modulációs timeline-ja sikeres kell legyen
**az első időzített grid előtt**.

Minden kör külön méri a `reference-repeat` és `candidate` összehasonlítást,
stationary-v1 és modulated-v1 terheléssel, SAW/Square/Triangle/Sine (0/1/2/3)
hullámformánként. Az első kör referenciaismétlés → jelölt, a második fordított;
a nyolc workload/waveform jelenet sorrendje is megfordul. A grid belső négy
párjának sorrendje változatlanul váltakozik. Körökből, hullámformákból vagy
terhelésekből nem képzünk összevont kedvezőbb mediánt.

A `--reference-repeat` a mérőprogram utolsó argumentuma: mindkét időzített
út ugyanazt a `experimental_lookup_engine::Synth` típust futtatja, új állapottal.
A `candidate` út változatlan: lookup SevenSaw → lookup SawDispatchSevenSaw.
A program kiírja a `comparison.txt` azonosítót; a reporter nem fogad el pusztán
környezeti változóval referenciaismétlésnek átnevezett jelöltet. A régi,
azonosító nélküli candidate archívum továbbra is olvasható, a gyűjtő viszont
minden új gridnél megköveteli a compiled azonosítót.

Gridenként 288 összesítő sor és 73 728 nyers blokk: 16 voice, FX be,
48/96/192 kHz, négy filtermód, 32/64/128 buffer, négy pár, 0,25 s warmup,
256 blokk/path/pár. A Drive/filter/latency/rátapolicy nem változik. Két kör
összesen 2 359 296 nyers blokk. A reporter az összes sort, percentilist,
>100% túllépést, routingot, waveformot és páros jeldiagnosztikát ellenőrzi.
A referenciaismétlés szóródása külön kontroll, nem levonható zajkorrekció,
CPU-küszöb vagy számszerű hibakorlát.

## Eredet és hibás futások

Minden grid külön mappát, nyers CSV-ket, compiled azonosítókat, compiler-
fájlt, mérést/riportot és byte-hashokat kap. A gyökér `manifest.json`
megőrzi a pontos commitot, platform/CPU/Python adatokat, parancsokat,
executable/compiler/tool/source hashokat és közös numerikus naplókat.
Minden mérés előtt és után újra ellenőrzi a checkoutot és a bináris/tool
byteokat; a lezáráskor a korábban gyűjtött fájlokat is ellenőrzi.

Csak az összes validált grid után lesz az állapot `complete`. Hiba esetén
`failed`, megszakításnál `incomplete` maradhat; a részadatokat és a naplókat
megőrzi. Korábbi buildet/eredményt nem ír felül. A teljes eredménymappát
őrizd meg, ne csak a riport táblázatát.

A gyűjtő regressziója szimulált folyamatokkal ellenőrzi a sorrendet, korai
numerikus hibát, rossz compiled azonosítót, bináris-/forrás-/eredetváltozást és
részadat-megőrzést. A reporter külön tesztje valódi teljes szintetikus CSV-
gridből validál. Ezek nem Windows/M1 célgépes CPU-eredmények.

A `complete` állapot adatgyűjtési siker, nem production CPU-elfogadás.
A célgépes grid értékelése, minőség/magasráta-policy és natív REAPER-próba
után következhet a production integráció és a kiadási kapuk lezárása.

## A teljes archívum ellenőrzése és áttekintése

A gyűjtés után az archívum másik gépen, build és REAPER nélkül ellenőrizhető:

```powershell
python scripts/review-dispatch-engine-target.py ../dispatch-engine-results-01 > ../dispatch-engine-review-01.md
```

macOS-en `python3` használható. A riportot az archívumon **kívül** mentsd;
az ellenőrző nem módosítja az eredményeket, és elutasítja a nem jegyzett fájlokat.
Csak teljes, 2–10 körös gyűjtést fogad el. Ellenőrzi a kvalifikációs naplók és
minden grid fájlhashát, a kampánysorrendet, forrás/compiler/compiled azonosítók
egyezését, a kilenc ráta/puffer cellát és az értékek érvényességét. Hiányos,
módosított, duplikált vagy egymásnak ellentmondó archívum hibával áll meg;
symlinket sem fogad el. Hibánál nincs részleges sikeres riport.

A táblázat minden kör, referenciaismétlés/jelölt, workload és hullámforma
p50/p99 arányát és túllépésszámát külön megőrzi. Nincs zajlevonás, outlier-
kizárás, körökből összevont medián vagy automatikus CPU-/production-elfogadás.

Ez archiválási épség- és riportkontraktus-ellenőrzés: nem számolja újra a nyers
CSV-k percentiliseit, és nem futtatja újra a numerikus kvalifikációt. A forrás
és executable eredeti bytejai nincsenek a gyűjtési mappába csomagolva, így
azokat itt nem hash-eli újra. A hashok belső egyezést bizonyítanak, nem a
manifest hitelességét; a megőrzött manifest SHA-256 és az eredeti, megbízható
gyűjtés együtt szükséges. A regresszió szintetikus archívumokkal és a valódi
CSV-riportoló szintetikus teljes gridjével dolgozik, nem célgépes CPU-adattal.

Az adatértékelés előtt a teljes nyers CSV-ellenőrzés is bekapcsolható:

```powershell
python scripts/review-dispatch-engine-target.py ../dispatch-engine-results-01 --verify-raw > ../dispatch-engine-review-raw-01.md
```

Ez ugyanazzal a validátorral ellenőrzi minden grid 288 összesítő sorát és
73 728 nyers blokkját, mint az eredeti gyűjtés: blokkazonosítók és sorrend,
p50/p95/p99/maximum, szigorúan >100% túllépések, páros jeldiagnosztikák,
compiled címkék és rátafüggő routing. Újraszámolja a kilenc páros eredménycellát,
és összeveti a mentett JSON-nal. Hibás nyers adat vagy eltérő mentett eredmény
esetén nincs sikeres riport. A mód nem ír az archívumba, nem futtat mérőprogramot,
és nem igényli az eredeti checkoutot; a hitelességi és CPU-elfogadási korlátok
változatlanok. Két körben összesen 2 359 296 nyers blokkot olvas újra.

## Windows és Apple Silicon archívum közös áttekintése

Két teljes célgépes gyűjtés után, a riportot mindkét archívumon kívül mentve:

```powershell
python scripts/compare-dispatch-engine-targets.py ../windows-results ../m1-results > ../target-comparison.md
```

macOS-en `python3` használható. A `--format json` géppel feldolgozható
kimenetet ad. Az első bemenet Windows x64, a második Apple Silicon macOS.
Mindkettő kötelező nyers CSV-újraellenőrzést kap; nincs gyorsított, csak
hashokat ellenőrző mód. Hiányos vagy hibás második archívumnál sincs
részleges sikeres stdout. A parancs egyik archívumba sem ír.

Azonos commit, forrásfájl-hashjegyzék, körszám, Release konfiguráció és
kutatási variáns szükséges. A gridek platform/gép címkéinek egyezniük kell
a saját manifesttel. A fordítóhashok platformonként eltérhetnek: minden
archívum a saját fordítóeredetét ellenőrzi. A platformcímkék archivált
állítások, nem függetlenül hitelesített hardverazonosítók; az ARM64 címke
önmagában nem bizonyítja, hogy a gép M1.

A riport minden kört, terhelést, hullámformát, ráta/puffer cellát és
referenciaismétlést külön tart meg. Az arányok az adott gépen belüli
jelölt/referencia arányok, nem a két gép abszolút sebességének arányai.
Nincs összevont medián, zajlevonás vagy automatikus CPU-/production-elfogadás.
A regresszió szintetikus archívumokat és külön raw-validator híváskontrollt
használ; ez nem új Windows/M1 mérés. A CSV-validátor valódi szintetikus
teljes gridpróbája a meglévő archívum- és deadline-tesztekben marad.
