# SAWSTAR Premium production — 1.2.1 maintenance candidate

## 1.2.0 kiadási döntés — 2026-11-07

Az 1.2.0 a legutóbbi nyilvános kiadás. Robert a 003b007 Premium RC-t macOS és
Windows alatt hibamentesnek jelezte, és 2026-11-07-én jóváhagyta annak
publikálását. Az 1.2.1-rc1 kizárólag tesztjelölt; a pontos jelölt natív
elfogadása és a külön publikálási döntés nyitott. Lásd a [1.2.1 jelölt
kiadási megjegyzéseit](https://github.com/RobCZart82/SAWSTAR/blob/main/docs/RELEASE_NOTES_1.2.1.md)
és a [korábbi 1.2.0 kiadás megjegyzéseit](https://github.com/RobCZart82/SAWSTAR/blob/main/docs/RELEASE_NOTES_1.2.0.md).


## Új factory bővítés — 2026-11-03

A fejlesztési bank 30 hangra bővült; az eredeti 24 változatlan. A hat új hang
exportált snapshotját Robert meghallgatta és jóváhagyta. A [factory bővítés és
252 controller-aware ellenőrzés leírása](https://github.com/RobCZart82/SAWSTAR/blob/main/docs/EXPRESSIVE_FACTORY_PRESETS_2026_11_03.md)
a repositoryban érhető el. Az alábbi 24/144-es mérés és a #135 PR-ről szóló
korábbi megjegyzések történeti RC-bizonyítékok, nem a mostani 1.2.1 jelölt
elfogadása.


Állapot: fejlesztési kiadásjelölt, nem publikált végleges kiadás.
Robert 2026-11-03-án kérte a production-integráció, factory/RC és dokumentáció
előrehozását. Ez nem pótolja a natív CPU-, filterminőségi és zenei elfogadást.

## Mit tartalmaz?

A `SAWSTAR_PREMIUM_PRODUCTION=ON` build a kalibrált Premium lookup referencia
motorját köti a valódi SAWSTAR VST3 wrapperhez. A meglévő paraméter-ID-k,
state-formátum, GUI, presetmappa és `SwSt` pluginazonosság megmarad.
A filter LP12/LP24/HP12/BP12, a kalibráció dry unity / wet +3 dB; az elfogadott
0,44 dB-es átlagos RMS-különbséget nem kompenzáljuk újabb hangerőemeléssel.
A Saw-dispatch Candidate külön tesztváltozat marad, itt nincs újra kiválasztva.

A plugin 32 mintás késleltetést jelent a hostnak; a dry és wet filterút egyaránt
igazodik ehhez. A host kompenzációját, bypass-t és automatizálást a végleges
csomag pontos binárisával natívan is ellenőrizni kell.

**Ugyanaz a pluginazonosság, mint a Classic SAWSTAR-é.** A két változat nem
telepíthető biztonságosan egymás mellé ugyanazon SAWSTAR néven. A build és a
CI nem telepít automatikusan. Tesztelés előtt mentsd a régi projektet és
pluginverziót; a tesztpéldányt új projektben nyisd meg. A régi paraméterek és
presetek olvashatók, de a Premium filter miatt a hangzás megváltozik.
Ez formátumkompatibilitás, nem a Classic hangzás bitazonos visszaállítása.
A Preview/Candidate külön pluginazonosságai továbbra használhatók külön.

Az alapértelmezett CMake-build Classic marad a nyitott elfogadási kapukig.
A production-integráció és Preview/Candidate kapcsoló együtt tiltott.
Az About felirat `1.2.0 rc1-premium-integration`, a kiadási dátum üres.
A verzió RC-javaslat; a végleges publikáláskor még jóváhagyandó.

## Fordítás, ellenőrzés és csomag

```sh
python3 scripts/bootstrap-plugin.py
cmake -S . -B build-rc -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DSAWSTAR_BUILD_PLUGIN=ON -DSAWSTAR_PREMIUM_PRODUCTION=ON -DIPLUG_DEPLOY_PLUGINS=OFF
cmake --build build-rc --config Release --target SAWSTAR-vst3 sawstar_premium_preview_smoke sawstar_premium_production_state sawstar_premium_preview_levels --parallel 2
```

A smoke a wrapperrel közös PluginSynth kiválasztását, SAWSTAR-identitást,
32 mintás dry késleltetést és a négymódú motort ellenőrzi. A state-próba
minden factory preset sound/host state visszatöltését, GUI-scale-t és
visszahívott renderének egyezését ellenőrzi. A levels-próba a limiter előtti
kimenetet méri 24 preset × 2 ráta × 3 hangszám = 144 esetben; minden eset
legalább 6 dB headroomot követel. Nem lazítottuk a küszöböt.

A `scripts/package-premium-rc.py BUILD_DIR OUTPUT.zip` ellenőrzi a build
engine/identity kapcsolóit, lefuttatja a tényleges Steinberg VST3-validátort,
és hashjegyzékkel csomagol. Meglévő RC-bizonyítékot nem ír felül.
A PREMIUM-RC-MANIFEST a forrást, dirty állapotot, engine-t és késleltetést
rögzíti; a natív és hallgatási elfogadás, publikálhatóság false.
A `premium-production-rc.yml` macOS Universal és Windows x64 csomagot készít,
Macen mindkét szelet smoke/state/headroom próbájával. Rosetta nem natív Intel
elfogadás; Windows ARM64 Premium RC e lépésben még nem kvalifikált.

## Kézikönyv-kiegészítés és kiadási kapuk

A csomagban levő magyar és angol PDF-kézikönyvek most 1.2.0 rc1 kiadásúak,
15-15 oldallal. Az első két oldal RC-hatókört/telepítést ír le, a 15. fejezet
a Premium motort, migrációt, 32 mintás latencyt és nyitott kapukat rögzíti.
A képek változatlanul jelölt 1.0.3 képernyőképek. Végleges publikálás előtt
a tényleges elfogadott buildhez még egyszer jóvá kell hagyni a kézikönyveket.

Nyitott: a teljes natív Mac/Windows CPU-/dropout-mátrix, filterminőség és
ráta/puffer döntés; K1 numerikus anomália rendezése; GUI/preset/lifecycle
elfogadás; presetenkénti zenei meghallgatás; pontos RC telepítési és hostpróba;
végleges kézikönyvek, csomagok és Robert publikálási jóváhagyása.
A korábbi 32 REAPER-ablak a korábbi külön tesztbinárisokra vonatkozik,
nem ennek az RC-nek az elfogadása. A teljes CI sikere is külön feltétel.

Nincs release/tag/publikálás, nincs aláírt/notarizált végleges installer és
nincs állított natív PASS ebben a fejlesztési lépcsőben.

## RC-provenance

Az első RC-k `source_dirty=true` mezője megmarad történeti bizonyítékként.
A bootstrap szándékosan javít két rögzített iPlug2 fájlt és a placeholder SDK
README helyére a rögzített VST3 SDK-t teszi. Az új csomagoló külön ellenőrzi
a projekt checkout tisztaságát, minden teljes javított fájlt a gitlinkhez
tartozó eredetiből újra előállított elvárt bájtokkal, az SDK/gyermekrepo pinjeit
és a tiszta DaisySP-t. CRLF/LF különbséget normalizál az összevetéshez, de a
nyers és normalizált fájlhashokat is megőrzi. Váratlan változásnál elutasít.
A nyers dirty jelölést nem állítja hamisan false-ra; a külön
`source_provenance` mutatja az ellenőrzött, szándékos függőségeltéréseket.
Ez nem upstream-pristine vagy natív elfogadási állítás.
