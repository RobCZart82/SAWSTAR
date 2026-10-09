# Oszcillátormérés saját célgépen

A `scripts/run-oscillator-target.py` ugyanazon tiszta commitból készít friss
Release buildet, futtatja a SevenSaw, a megosztott frekvenciaszámítás és a
riport numerikus ellenőrzéseit, majd külön referencia- és jelöltkampányokat
gyűjt. Nem szükséges REAPER; ez offline oszcillátorbank-mérés.

## Előkészítés

Python 3, Git, CMake/CTest és C++17 fordító szükséges. Windows alatt például
Visual Studio 2022 Build Tools C++ workload és Windows SDK; macOS-en Xcode
Command Line Tools. A tiszta checkout rekurzív submodule-jai legyenek
inicializálva. A DaisySP HEAD-nek és forrásainak a rögzített gitlinket kell
követniük. Az eszköz nem frissít függőséget, nem módosít forráskódot.

Állítsd le a DAW lejátszását, zárd be a CPU-t terhelő alkalmazásokat és
használd ugyanazt az energiaellátási beállítást minden összehasonlításnál.
Ez kézi előkészítés: az eszköz nem állít CPU-affinitást, realtime prioritást
vagy energiagazdálkodást, és nem állítja, hogy a háttérterhelés kontrollált.

A checkout szülőmappájába kerüljön a build és az eredmény, egymástól külön.
Mindkét könyvtárnak újnak kell lennie; korábbi eredményt nem írunk felül.
A repository gyökeréből:

```powershell
python scripts/run-oscillator-target.py --build-dir ../sawstar-target-build-01 --output-dir ../sawstar-target-results-01
```

macOS-en ugyanígy, `python3` paranccsal. Ha CMake/CTest nincs a PATH-ban,
az abszolút elérési út külön megadható `--cmake` és `--ctest` argumentummal.
A futás hosszú lehet; a két kör összesen négy teljes, hosszabb kampányt mér.

## Eredmények és korlátok

Alapértelmezésben két kör fut: referencia → jelölt, majd jelölt → referencia.
Minden kampány 384 nyers sort, 24 cellát és cellánként nyolc váltakozó párt
tartalmaz, páronként 131 072 frame-mel. Minden kampány külön mappában marad;
a körök eredményeit nem olvasztjuk egy kedvezőbb mediánba. `--rounds` 2–10
között adható meg.

A `manifest.json` rögzíti a commitot, platformot, CPU-leírást, parancsokat,
executable/runner/reporter/source hash-eket, compiler-fájlt, tesztnaplót és
minden kampány fájlhash-eit. A részleges vagy hibás futás állapota
`incomplete` vagy `failed`; kizárólag az összes validált kampány után lesz
`complete`. A teljes eredménymappát őrizd meg, ne csak a Markdown-táblázatot.

A referenciaismétlés mindkét útján ugyanaz a SevenSaw-függvény fut. A
szóródás a mérés értelmezését segíti, nem kivonható zajkorrekció és nem
számszerű hibakorlát. A páros energia egyezése önmagában nem helyettesíti
a mérés előtt futtatott mintánkénti bitazonossági tesztet.

Egy `complete` mérés sem production CPU-elfogadás. A shipping hangmotor,
a filter minőség/rátapolicy és a natív REAPER callback külön kapu marad.
Először a negatív Windows SAW/Square eredmények ismétlését és az alternatív
hullámformák következetességét kell értékelni, majd indokolt esetben külön
teljesmotoros modulált deadline-kísérlet következhet.
