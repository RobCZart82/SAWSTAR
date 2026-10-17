# Kalibrált Preview Reference/Candidate — numerikus előfeltétel

Állapot: **az M1 mérföldkő első fejlesztési lépése; nem natív host- vagy CPU-elfogadás**.
A külön telepíthető Candidate VST3-identitás/csomag még nincs implementálva.

## Miért kell külön kapu?

A lezárt célgépes kampány a Premium lookup kutatási referenciát és a Saw-dispatch
kutatási jelöltet vizsgálta. A valódi Preview azóta a jóváhagyott +3 dB wet /
dry unity kimeneti kalibrációval működik. A natív tesztpár előkészítéséhez a
jelöltet a **tényleges kalibrált Preview motorhoz** is külön kell hasonlítani.
A régi 480 eset nem címkézhető át automatikusan erre az új fordítási változatra.

## Implementáció és elhatárolás

- A referencia a meglévő `sawstar_premium_preview_engine` / `PremiumPreviewSynth`.
- A külön `sawstar_premium_native_candidate_engine` ugyanebből a generált,
  már kalibrált motorból készül. Csak az OSC1/OSC2 `SevenSaw` típusa cserélődik
  `SawDispatchSevenSaw`-ra. A filter, Drive, latency-adapter, kalibráció,
  envelope/MIDI/moduláció, SUB, FX és state ugyanaz marad.
- A két motor külön namespace-ben fordul. A jelölt könyvtár csak a páros
  tesztexecutable függősége; **a VST3-target nem linkeli**. A production Classic
  és a már létező Premium Preview identitása/hangútja ettől nem vált át.
- A közös `oscillator_dispatch_engine.cpp` fixture külön fordítási definícióval
  választja ezt a párt. Az eredeti lookup/dispatch kutatási fixture megmarad.
  Nem keverjük a régi célgépes forrás/hashjegyzékkel az új tesztforrást.

## Numerikus szerződés

Mind a 480 eset megmarad: négy hullámforma, öt ráta (48/96/176,4/192/384 kHz),
négy filtermód, Poly/Mono/Legato, FX ki/be. Mintánkénti bitazonosság a
kimeneten és a pre-FX ponton; véges, védett kimenet; moduláció, Drive-,
filtermód-/waveformváltás és dry/wet átmenet; MIDI ownership, voice steal,
sustain, release/tail, reset és negatív módosított-jel kontroll.
Hullámformánként 120 eset; hiányos mátrix nem sikeres.

Helyi ellenőrzés: GNU 13.3.0, Linux, Release; a kalibrált pár 480/480 esete
sikeres, a négy hullámforma naplója együtt 480 egyedi kombinációt őriz,
minden kimeneti/pre-FX hibamérték nulla. A meglévő Preview identity/32 mintás
dry latency/négymódú smoke is sikeres. Ez nem macOS/Windows natív futás,
nem CPU-időmérés, nem validator- vagy REAPER-eredmény.

## CI és futtatás

A külön `Calibrated Preview engine pair qualification` workflow Windows x64
és macOS runneren fordítja/futtatja a numerikus párt, plugintelepítés nélkül.
A normál Universal Preview workflow a valódi Preview VST3 építése mellett
mindkét arm64/x86_64 szeleten futtatja a páros ellenőrzést is.
A külön workflow manifestje megőrzi a pontos checkout SHA-t, executable-
és naplóhashokat, siker/hiba állapotot; minden eredmény kísérleti, a
`native_host_acceptance` és `production_promotion_allowed` flag false marad.
Új head minden releváns CI-jének tényleges sikerére várni kell.

```sh
cmake -S . -B build-pair -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DSAWSTAR_PREMIUM_PREVIEW=ON -DSAWSTAR_BUILD_PLUGIN=OFF
cmake --build build-pair --config Release --target sawstar_premium_native_pair sawstar_premium_preview_smoke --parallel 2
```

A létrejött pair executable `--waveform 0`, `1`, `2`, `3` argumentummal fut,
argumentum nélkül mind a 480 esetet végrehajtja. A futtatási útvonal a
CMake-generátortól függ (Windows esetén jellemzően `Release/` almappa és `.exe`).
BUILD_TESTING=ON esetén négy külön `premium_native_pair_waveform_*` CTest is
regisztrált. Ez kvalifikáció, nem új lezárt M1/Windows mérési kampány.

## Hátralévő M1 munka

1. A kalibrált Reference/Candidate numerikus kapu pontos-head platform-CI-je.
2. Külön natív Candidate VST3-identitás, presetmappa és buildazonosítás;
   a Classic és meglévő Preview megőrzése. A két natív buildazonosság dokumentálása.
3. Csomagolás/forrás-/binárishashok, validator és telepítési smoke Windows/macOS alatt.
4. Ezután a [valós REAPER-mátrix](NEXT_RELEASE_DEVELOPMENT_PLAN.md) végrehajtása,
   a [natív adatlap](NATIVE_REAPER_CPU_ACCEPTANCE.md) kitöltésével.

A régi p99/túllépés-eredmények és a 0,44 dB RMS-korrekció elfogadása megmarad.
Nincs automatikus production-integráció, CPU-/natív elfogadás, release vagy publikálás.
