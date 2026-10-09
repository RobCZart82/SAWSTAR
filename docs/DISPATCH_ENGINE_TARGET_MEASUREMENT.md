# Teljesmotoros SAW-dispatch célgépes gyűjtés

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
