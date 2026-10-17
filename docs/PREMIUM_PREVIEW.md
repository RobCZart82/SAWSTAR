# SAWSTAR Premium Preview — ideiglenes macOS tesztplugin

**Fejlesztési állapot — 2026-10-17:** a Preview külön, alapból kikapcsolt;
a production Classic marad. A hangkarakter és a hangerőkorrekció Robert által
elfogadott, a megmaradó 0,44 dB RMS-különbség lezárt döntés. A GUI-méret
host-state javítása implementált, natív visszatöltési elfogadása nyitott.
A M1/Windows offline gyűjtés és nyers összevetés kész; a formális natív
CPU-kapu nyitott, a Saw-dispatch nincs ebben a Preview-ban.
[Release terv és REAPER-mátrix](NEXT_RELEASE_DEVELOPMENT_PLAN.md).
A lenti 256/512-es gyorsindító puffer óvatos hallásos indulás; nem azonos
az előre kijelölt 48 kHz /128, majd /64 CPU-elfogadási mátrixszal.

Ez egy külön, kísérleti VST3 hangszer hallásos próbához. Nem végleges kiadás,
és a CPU-, preset-/projektkompatibilitási és natív host-elfogadása még nyitott.
A Classic SAWSTAR mellé telepíthető; nem helyettesíti a meglévő példányokat.

## Letöltés és telepítés REAPER-hez

1. Az Actions `macOS Premium Preview` sikeres futásának Artifacts részéből
   töltsd le a `SAWSTAR-Premium-Preview-macOS-Universal` csomagot. Az Actions
   artifact letöltéséhez GitHub-bejelentkezés szükséges lehet.
2. Bontsd ki az artifactot, majd a benne lévő preview ZIP-et is.
3. Zárd be a REAPER-t. A teljes `SAWSTARPremiumPreview.vst3` csomagot másold
   ide: `~/Library/Audio/Plug-Ins/VST3/`. A meglévő `SAWSTAR.vst3` maradjon meg.
4. Indítsd el a REAPER-t; szükség esetén Preferences → Plug-ins → VST → Re-scan.
   Az FX-listában keresd: **SAWSTAR Premium Preview**.
5. Ha macOS blokkolja a letöltött, aláíratlan plugint, a Privacy & Security
   beállításainál engedélyezd az adott csomagot. Globális biztonsági beállítást
   ne kapcsolj ki. Ez a build nincs Apple által aláírva vagy notarizálva.

A csomag Universal: Apple Silicon és Intel szeletet tartalmaz. A CI-s Intel
validálás Rosetta alatt fut; natív Intel REAPER-próbát nem helyettesít.
A ZIP mellett SHA-256 fájl és a ZIP-ben forráscommitot tartalmazó manifest van.
Eltávolításához csak a `SAWSTARPremiumPreview.vst3` csomagot kell törölni.

## Mit lehet meghallgatni?

### Korrigált kimeneti szint — 2026-10-15

A friss Preview teljes wet filternél +3 dB kimeneti korrekciót használ,
dry állásban nem emel. A factory presetek külön Volume-kalibrációt kapnak:
a megadott 144 cellás helyi gridben legalább 6,5 dB headroom maradt a
csúcsvédelem előtt. Ez a hangosság helyett a kivezérlési tartalékot helyezi
előtérbe; az elfogadott filter/Drive karaktere változatlan.

Régi projekt vagy saját preset mentett Volume/Boost értékeit nem írjuk át,
ezek teljes wet hangja legfeljebb +3 dB-lel nő a korábbi Preview-hoz képest.
Frissítés előtt ments másolatot a tesztprojektről, majd ellenőrizd a kivezérlést.
A factory-kalibráció egy factory preset újbóli kiválasztásakor lép életbe.
[Módszer, nyers adatok és korlátok](PREMIUM_PREVIEW_LEVELS.md).

- Prémium LP12, LP24, HP12 és BP12, a meglévő Filter Mode választóval.
- Prémium Drive a meglévő Filter Drive szabályzóval; a Filter Mixet emeld
  100%-ra a teljes wet hanghoz. Mixer/ADSR/LFO/ARP/FX kezelése a megszokott.
- A rátafüggő SIMD/reciprok/lookup Drive: 176,4 kHz alatt 4×, onnantól 2×.
- Az oszcillátorok a megszokott SevenSaw motort használják. A még kutatott
  SAW-dispatch optimalizálás nincs ebben a preview-ban.

A host felé jelzett késleltetés **32 minta**, 48 kHz-en kb. 0,67 ms.
A dry filterút is késleltetett, így a dry/wet keverésen belül egyeznek az utak.
A prémium hang és a Classic presetek egyensúlya eltérhet; ez nem végleges
factory-preset hangolás vagy kompatibilitási ígéret.

## Külön identitás és presetek

- Hostnév: `SAWSTAR Premium Preview`.
- Bundle: `SAWSTARPremiumPreview.vst3`.
- Bundle ID: `io.github.robczart82.sawstarpremiumpreview`; plugin ID: `SwPP`.
- Saját presetmappa: `~/Library/Application Support/SAWSTAR Premium Preview/Presets/`.
- A meglévő SAWSTAR presetkönyvtára és projektpéldányai nem kerülnek át automatikusan.
  A .sawstar fájlok kézzel importálhatók, de a hangzásuk megváltozhat.

Hallásos teszthez kezdj 48 kHz-en, 256/512-es pufferrel, mérsékelt hangerővel
és új tesztprojektben. Hasonlíts össze hasonló hangerőre állított Classic és
Preview példányokat: cutoff/resonance, mind a négy mód, Drive 0/12/24 dB,
filterburkoló, akkordok és lecsengések. A jobb hangosság nem önmagában jobb hang.
Figyeld a CPU-kijelzést és a recsegést is; a 16 hangos prémium út nagy terhelésű.

## Build és ellenőrzés

Alapból minden build továbbra is Classic. A preview külön buildkönyvtárban:

```sh
python3 scripts/bootstrap-plugin.py
cmake -S . -B build-premium -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DSAWSTAR_BUILD_PLUGIN=ON -DSAWSTAR_PREMIUM_PREVIEW=ON '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64' -DIPLUG_DEPLOY_PLUGINS=OFF
cmake --build build-premium --config Release --target SAWSTARPremiumPreview-vst3 sawstar_premium_preview_smoke sawstar_premium_preview_levels --parallel 3
build-premium/sawstar_premium_preview_smoke
build-premium/sawstar_premium_preview_levels > premium-levels.csv
python3 scripts/validate-plugin.py --bundle build-premium/out/SAWSTARPremiumPreview.vst3
```

Az engine-smoke az identitást, presetmappa-szétválasztást, 32 mintás dry
késleltetést, négy filtermódot, véges kimenetet és MIDI panicot ellenőrzi.
A Steinberg validator az elkészült VST3-at vizsgálja. Ezek nem helyettesítik
a REAPER-es hallásos és életcikluspróbát. Nincs automatikus telepítés/publikálás.

## English quick start

Download the successful `macOS Premium Preview` workflow artifact, unzip twice,
and copy the complete `SAWSTARPremiumPreview.vst3` bundle to
`~/Library/Audio/Plug-Ins/VST3/` with REAPER closed. Keep `SAWSTAR.vst3` in place.
Rescan VST3 and load **SAWSTAR Premium Preview**. This unsigned experimental
Universal build uses a separate plugin ID and preset folder. It reports 32 samples
of latency and uses the four-mode rate-scaled lookup premium filter/Drive.
Start at 48 kHz with a 256/512 buffer in a separate test project. CPU and native
acceptance remain open; presets may sound different from the Classic plugin.
