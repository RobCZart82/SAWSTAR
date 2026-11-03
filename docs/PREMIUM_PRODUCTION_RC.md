# SAWSTAR 1.1.0 rc1 — Premium production-integráció

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
Az About felirat `1.1.0 rc1-premium-integration`, a kiadási dátum üres.
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

A csomagban levő korábbi PDF-kézikönyvek a változatlan kezelőszervekhez és
presetkezeléshez használhatók. A Premium filter, RC státusz, 32 mintás latency
és migráció szempontjából ez a kiegészítés az irányadó; a PDF-ek végleges,
verziózott újrakiadása a release előtt még szükséges.

Nyitott: a teljes natív Mac/Windows CPU-/dropout-mátrix, filterminőség és
ráta/puffer döntés; K1 numerikus anomália rendezése; GUI/preset/lifecycle
elfogadás; presetenkénti zenei meghallgatás; pontos RC telepítési és hostpróba;
végleges kézikönyvek, csomagok és Robert publikálási jóváhagyása.
A korábbi 32 REAPER-ablak a korábbi külön tesztbinárisokra vonatkozik,
nem ennek az RC-nek az elfogadása. A teljes CI sikere is külön feltétel.

Nincs release/tag/publikálás, nincs aláírt/notarizált végleges installer és
nincs állított natív PASS ebben a fejlesztési lépcsőben.
