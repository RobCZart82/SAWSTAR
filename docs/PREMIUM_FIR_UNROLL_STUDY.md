# 2× SIMD interpoláció cikluskibontása — 2026-10-08

## A mérésből következő cél

A #83/#84 komponensprofil elkészült Windows SSE2 és macOS NEON alatt.
Mért forrás: `faecc277dd7ab51961e592d3d77cdd758ffa3b95`, workflow
`37743076008`. A két 360 soros nyers CSV, 120 soros összesítő és eredetjegyzék
az `experiments/premium_filter/measurements/2026-10-08-components-ci-*`
fájlokban tartósan szerepel. A CI-naplóból visszanyert CSV és újra előállított
összesítő pontos byte-hash-a egyezik a mérési metadata-val. A baseline Drive
forráshash-a is ellenőrzött; Windows checkout CRLF, macOS LF sorvégű.

LP24, három ismétlés mediánja, audioidő-százalék; **nem additív CPU-részarány**:

| Platform | Ráta | Oscillator-bank | Drive-bank | Filter-bank | Teljes kutatási motor FX-szel |
| --- | --- | --- | --- | --- | --- |
| Windows | 48 kHz | 17,704 | 23,196 | 2,776 | 54,469 |
| Windows | 96 kHz | 33,542 | 45,571 | 5,525 | 108,588 |
| Windows | 192 kHz | 64,306 | 55,158 | 11,099 | 173,818 |
| macOS | 48 kHz | 8,333 | 14,002 | 1,250 | 27,375 |
| macOS | 96 kHz | 15,210 | 29,496 | 3,285 | 50,460 |
| macOS | 192 kHz | 36,068 | 22,158 | 5,165 | 78,083 |

A Drive/FIR 48/96 kHz-en érdemi optimalizálási cél; 192 kHz-en az
oszcillátorok költsége is jelentős. Egyes macOS kontrollok szórnak: az adapter
gyorsabbnak látszik a külön Drive-nál, és az FX-es motor a no-FX változatnál.
Ez alátámasztja, hogy a külön mérések nem összeadhatók és különbségük sem
bizonyított attribúció. A runner nem natív realtime/REAPER-elfogadás.

## Jelölt

A `FixedRatePremiumDrive` új, alapból false `UnrolledFir` template-opciója
kibontja a 2× SIMD interpoláció négy-tapos csoportciklusait. A comma fold sorrendben
hívja ugyanazokat a FirLanes4 műveleteket: nincs új tapszám, coefficient,
redukciós sorrend, telítés, gain/ramp, puffer, latency vagy rátapolicy.
A meglévő default ág, a teljes 4× ág és a decimátor for-ciklusai megmaradnak.
A kibontó helper explicit inline kérést használ MSVC és GCC/Clang alatt.
Külön rate-router és Synth
névtér használja az opt-in jelöltet; shipping DSP-váltás nincs.

A korábbi Drive-header befagyasztott oracle:
`LoopReferencePremiumDrive.h`, a `6c87906` main Drive-kódjából külön
névtérrel. A teszt a befagyasztott, default és unrolled út float-bytejait
hasonlítja össze, nem csak numerikus toleranciával. Vizsgálja a 2×/4× ágat,
négy rátát, hat inputot, változó Drive/Snap/Clear eseményeket, élő állapot
másolását, hibás csatornabemenetet, csendet és a 176,4 kHz-es routinghatárt.
A méret és a 32 mintás latency fordítási kontroll.

## Kapuk és eredmények

- Helyi MSVC 19.44 x64 Release: 427 008 sztereó frame bitazonos; a célzott C++ teszt sikeres.
- Mind az öt Python riportteszt sikeres, a rossz routing/backend és hiányzó/hibás compiled study marker elutasításával.
- Teljes motor: 120 lifecycle/modulációs eset explicit float-byte egyezéssel; külön páros fixture a teljes negyedmásodperces warmup és leghosszabb modulált mérési szekvencia output/PreFX bytejait is ellenőrzi.
- Független 8× referencia: ugyanaz a 144 komplexforrás-korlát, plusz nulla eltérés a lookup loop kontrollhoz képest. A korábbi minőségi határok változatlanok.
- Windows/macOS Release: fix 2×/4× izolált Drive-párok, majd külön `stationary-v1` és `modulated-v1` teljesmotor-adatok, négy váltakozó párral, p50/p95/p99/max és túllépésszámmal.

A teljesmotor-fordítás, új numerikus/8× minősítés és platformos CPU-eredmény
friss CI-kapu; a helyi környezetben nincs pinned DaisySP. A cikluskibontás
gyorsulási hipotézis: a compiler már kibonthatta a loopot, és a nagyobb
utasításállomány ronthat is. A rosszabb eredményt is megőrizzük. Aktiválás
csak megismételt teljesmotor/célgépes eredmény és minőségelfogadás után.

Az izolált Drive CLI `--benchmark` 48 sort ad (két faktor, három ráta,
négy váltakozó pár, két út), 16 példánnyal, 20 dB Drive-val. Inicializálás,
bemenet-előkészítés és 0,25 s warmup kívül, bankciklus és energiadiagnosztika
belül van a timeren. A szintetikus bemenet ciklikus; ez nem aliasing-teszt
és nem önmagában teljesmotoros nyereség.

## Helyi költségpróba és a jelölt szűkítése

A teljes FIR-kibontás előzetes helyi próbája 41–47% lassulást adott.
Explicit inline mellett is kb. 4–8% regresszió maradt. A nagyobb
decimátorciklus megtartásával a 2× interpoláció kb. 1% javulási jelet,
a 4× ág továbbra kb. 1–2% romlást adott. Ezért a végső jelölt kizárólag
a 2× interpolációt bontja ki; a 4× út változatlan kontroll. Ezek az
előzetes megfigyelések magyarázzák a végső szűkítést; nem CPU-elfogadási adatok.

A végső forrás külön 48 soros páros mérésének nyers CSV-je és source/compiler/
executable byte-hash jegyzéke rögzített. Arány: unrolled / loop, kisebb kedvezőbb.

| Ráta | Faktor | Páros medián arány |
| --- | --- | --- |
| 48000 | 2× | 0.987894 |
| 96000 | 2× | 1.006644 |
| 192000 | 2× | 0.921370 |
| 48000 | 4× | 1.038602 |
| 96000 | 4× | 1.005339 |
| 192000 | 4× | 1.012040 |

A 4× párok szórása változatlan kód melletti kontroll. Ez egy izolált helyi
CPU-kör; a kis különbségekből nem következik stabil nyereség vagy teljesmotoros elfogadás.
