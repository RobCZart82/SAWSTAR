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
- Mind a hat Python riportteszt sikeres, a rossz routing/backend és hiányzó/hibás compiled study marker elutasításával, a gépi riport páros aggregálásának kontrolljával.
- Teljes motor: 120 lifecycle/modulációs eset explicit float-byte egyezéssel; külön páros fixture a teljes negyedmásodperces warmup és leghosszabb modulált mérési szekvencia output/PreFX bytejait is ellenőrzi.
- Független 8× referencia: ugyanaz a 144 komplexforrás-korlát, plusz nulla eltérés a lookup loop kontrollhoz képest. A korábbi minőségi határok változatlanok.
- Windows/macOS Release: fix 2×/4× izolált Drive-párok, majd külön `stationary-v1` és `modulated-v1` teljesmotor-adatok, négy váltakozó párral, p50/p95/p99/max és túllépésszámmal.

A teljesmotor-fordítás és numerikus/8× minősítés a #85 CI-futásában
sikeres; a CPU-elfogadás nyitott. A helyi környezetben nincs pinned DaisySP. A cikluskibontás
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

## #85 teljesmotoros eredmény és döntés

Forrás: `a4270816229b848ed704dcdaf4ed9a20b709be11`; main-be olvasztva:
`6d7c2ac13482c289d45144bd677f6aa309a5ca6a`. Mind a 43 exact-head
ellenőrzés sikeres, beleértve a Windows/macOS Debug/Release és VST3
fordítást, valamint a Linux sanitizer/coverage futásokat. Az új jelölt
négy Release kutatási jobja lefuttatta a 120 teljesmotoros lifecycle/
modulációs esetet, a független 144 komplexforrás-kontrollt és a teljes
mért modulációs timeline-t. A szándékos eltérés negatív kontrolljának
kivételkezelése javítva; a bitazonossági követelmény nem enyhült.

Az archivált táblázatok sikeres CI-jobok naplóiból kiemelt riportok.
A jobok 288 summary-sort és 73 728 nyers blokkot validáltak egyenként.
Ez az archívum nem tartalmazza a nyers block CSV-t vagy annak külön,
helyi hash-újraellenőrzését; a teljes adat az adott futás artifactja.

| Platform / workload | Archivált riport |
| --- | --- |
| Windows / stationary-v1 | [riport](../measurements/premium-unrolled-ci/windows-stationary.md) |
| Windows / modulated-v1 | [riport](../measurements/premium-unrolled-ci/windows-modulated.md) |
| macOS arm64 / stationary-v1 | [riport](../measurements/premium-unrolled-ci/macos-stationary.md) |
| macOS arm64 / modulated-v1 | [riport](../measurements/premium-unrolled-ci/macos-modulated.md) |

A [manifest](../measurements/premium-unrolled-ci/manifest.json) a futás,
jobok és riportok kapcsolatát rögzíti. Study/reference arány: kisebb
kedvezőbb. 192 kHz-en a Windows p50 arányok stationary esetben
1.018972–1.021967, modulált esetben 1.009016–1.010422: a teljesmotoros
eredmény rosszabb a kontrollnál. macOS-en a 192 kHz-es p50 arányok
0.983557–1.004890 közöttiek, a p99/túllépésszámok vegyesek. Például
modulált 192 kHz/128 mellett a túllépések száma 580-ról 662-re nőtt.

48/96 kHz-en mindkét út változatlan 4× kódot használ: az ottani
eltérés kontroll a compiler/kódelrendezés/runner szórására, nem a 2×
kibontás előnye. Windows 96/192 kHz-en mindkét út összesített
4096/4096 túllépése sem realtime elfogadás, sem új funkcionális bug
bizonyítéka: a mért kutatási workload nem fér a runner audioidejébe.
Zöld CI itt numerikus/fixture-helyességet igazol, nem CPU-elfogadást.

Döntés: a jelölt kísérleti marad; ebből a futásból nem indokolt
production aktiválás. A 3. roadmap-lépés teljesítménykapuja nyitott.
Azonos forrású, nyugodt célgépes ismétlés és p99/túllépés-értékelés
szükséges; a következő algoritmikus hipotézis az oszcillátorköltség
csökkentése a komponensprofil alapján. A natív REAPER-próba külön kapu.

## Géppel feldolgozható mérési riport

A deadline-reporter `paired-results.json` fájlt is készít a már
validált adatokból, `schema_version: 1` formátumban. A `metadata`
megőrzi a source SHA-t, workload/backend/routing azonosítókat és a
mérési fájlok byte-hash-eit. A kilenc `paired_results` sor rátánként/
bufferenként 16 páros megfigyelés medián p50/p99 arányát és mindkét
út 4096 blokkjának szigorúan 100% feletti túllépésszámát tartalmazza.
Az arány iránya explicit `study/reference`; natív hostelfogadás false.

A FIR-kibontás vizsgálata egy `DEADLINE_CI_REPORT=` prefixű kompakt
JSON-sort is kiír. A workflow meglévő artifactja a JSON-fájlt is
megőrzi. Így a későbbi értékeléshez nem kell a kerekített Markdown-
táblázatot parse-olni. A JSON nem minősít automatikusan gyorsulást,
és nem helyettesíti a nyers blokkok megőrzését.
