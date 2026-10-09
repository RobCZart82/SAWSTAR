# Teljesmotoros SAW-dispatch vizsgálat - 2026-10-09

## Hatókör és döntés

A #92 izolált SAW-mérése kedvező, az alternatív hullámformák eredménye
vegyes. Ez a következő, külön kutatási kapu; CPU-elfogadás vagy shipping
aktiválás még nincs. A cache/shared-frequency és FIR-unroll jelöltek
eredményei nem kerülnek az új mérésbe.

CMake a jelenlegi motorból két külön namespace-ű kutatási Synth-et generál.
A referencia `experimental_lookup_engine`, a jelölt
`experimental_dispatch_engine`. Mindkettőben ugyanaz a
`RateScaledLookupEnginePremiumFilter` és SIMD FIR marad, 176,4 kHz alatt
4×, onnantól 2× Drive, azonos 32 mintás adapterrel. Csak OSC1 és OSC2
`SevenSaw` típusa cserélődik `SawDispatchSevenSaw`-ra. A SUB, LFO,
moduláció, voice/MIDI és FX kód megmarad; a production target változatlan.

## Numerikus kapu

`oscillator_dispatch_engine_identity`: 480 eset, négy kezdő hullámforma,
48/96/176,4/192/384 kHz, négy filtermód, Poly/Mono/Legato és FX ki/be.
Cutoff/Drive/LFO, wheel/pressure/bend, sustain, stealing/fallback,
release/retrigger/reset és OSC1/OSC2 különböző hullámformákra váltása is
szerepel. A kimenet és pre-FX minden mintája véges, bitazonos a referenciával;
az objektumméret és voice/MIDI tulajdonlás egyezik. Eltérő jelet a negatív
kontroll elutasít. Nem lazítjuk a korábbi minőségi toleranciákat.

Helyi Windows MSVC 19.44 Release: a 480 eset és az alap deadline-fixture
sikeres. A repository top-level CMake-konfigurációja és az új kutatási targetek
Release buildje is sikeres. A négy modulált waveform-fixture bitazonos;
a hibás selectorok elutasítottak. Nyolc deadline-reporter és 15 oszcillátor-
reporter/archívumteszt sikeres, helyi workspace-fixture adapterrel.
A macOS regresszió és a platformos CPU-mérés még a workflow feladata;
a helyi build nem teljes plugin-build vagy natív hostpróba.

## Külön waveform/workload mérések

Target: `sawstar_dispatch_engine_deadline`.
Study: `rate-saw-dispatch`; fordított jelölő: `saw-dispatch-v1`.

```sh
./sawstar_dispatch_engine_deadline --modulation-contract --waveform 0
./sawstar_dispatch_engine_deadline --deadline-modulated results --waveform 0
SAWSTAR_PREMIUM_STUDY=rate-saw-dispatch SAWSTAR_PREMIUM_WORKLOAD=modulated-v1 SAWSTAR_OSC_WAVEFORM=0 python scripts/report-premium-deadline.py results
```

A workflow Windows/macOS × stationary-v1/modulated-v1 négy külön jobot
futtat. Jobonként SAW/Square/Triangle/Sine (0/1/2/3) külön mappába kerül,
nem egy összevont mediánba. A selector más értéket elutasít.
Minden hullámforma előtt ugyanaz a mért modulációs timeline kap bitazonossági
kontrollt. Időzített hullámformaváltás nincs: azt a motorregresszió fedi le.
A filter változatlan, független 8× complex-reference kontrollja külön fut.

Egy grid: 16 voice, setup 20 dB Drive/50% Resonance, FX be; 48/96/192 kHz,
négy filtermód, 32/64/128 buffer, négy váltakozó sorrendű pár, 0,25 s warmup
és 256 mért blokk/path/pár. A modulated-v1 a meglévő pitch/filter/Drive/mix/
LFO/controller timeline-t használja; Drive céljai 0/12/24 dB.
Gridenként 288 összesítő és 73 728 nyers blokk, pathonként 36 864 blokk.
Nincs outlier-kizárás. A deadline-túllépés szigorúan >100% audioidő.

A reporter ellenőrzi a teljes gridet, nyers percentiliseket, sorrendet,
workload/study/waveform/backend/routing jelölőket és az azonos páros
peak/RMS/checksum diagnosztikát. A `DEADLINE_CI_REPORT` megtartja a p50/p99
páros arányokat és minden túllépést. A forráscommit, forrásfájlok és nyers
mérési fájlok SHA-256-a a metadatába kerül. A compiler, numerikus kapuk és
8× kontrollok közös artifactban maradnak; részadat hiba esetén is feltöltődik.

A platformos teljesmotoros eredmény még nincs elfogadva. Külön értékelendő
a SAW-nyereség, minden alternatív ág, p99 és túllépés; jobb izolált medián
nem bizonyít teljesmotoros vagy natív realtime nyereséget. Következik az
azonos forrású célgépes ismétlés, magasráta/minőségpolicy és natív REAPER,
majd a roadmap 5-8. production/kiadási kapuja. Kiadási verzió nincs kijelölve.
