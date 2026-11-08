# K1 – átadási jelentés olvasási hozzáférésű AI Debug tanácsadónak

Állapot: 2026-11-08. Repository: https://github.com/RobCZart82/SAWSTAR
Vizsgálati forrásalap: `5af34f1b2dbb7506034e282d0bf8393f821a63a7`.

## 1. Feladat és elvárt válasz

A repositoryt csak olvasni tudod. Kérjük, végezz forrás- és bizonyítékalapú oknyomozást, majd írj tanácsadó jelentést. Ne állíts javítást végrehajtott kísérlet nélkül. Különítsd el a tényt, következtetést, hipotézist és a hiányzó adatot. Nem újabb általános tesztlistát, hanem rangsorolt, konkrét kódhelyekhez kötött és cáfolható hipotéziseket kérünk.

A fő kérdés: **miért térhet el ritkán két elvileg bitazonos motor ugyanabban az offline összehasonlító folyamatban, miközben ugyanaz a bináris más futásokban teljesen egyező eredményt ad?**

K1 jelenleg megőrzött numerikus anomália; a gyökérok ismeretlen. Nem bizonyított, hogy production DSP-hiba, összehasonlítófixture-hiba, inicializálás/UB, fordító-, könyvtár- vagy Rosetta-hiba okozza. A rögzített eltérések valós tesztnegatívok; nem minősíthetők egyszerűen tesztzajnak. Hallható rendellenesség vagy DAW-összeomlás e bizonyítékokból nem következik.

## 2. Mit hasonlít össze a teszt?

A kalibrált Premium lookup Reference és a kísérleti Saw-dispatch Candidate motort. A két motor ugyanabban a folyamatban fut, azonos teszteseményeket kap, output és pre-FX pontjain bitazonosságot várunk. Ez **nem ARM64 Reference és x86_64 Candidate keresztarchitektúrás összehasonlítása**. Mindkét motor fut az adott architektúrájú folyamaton belül. A workflow Reference/Candidate szerepnevei a job/artifact eredetét is jelölhetik; nem jelentik, hogy az összehasonlító executable csak egy motort tartalmaz.

A közös `RateScaledLookupPremiumDrive` út SIMD FIR-t is használ. Az x86_64 `FirLanes4` SSE2 double műveleteket tartalmaz; nem kizárólag Candidate-specifikus SIMD-ről van szó. A Saw-dispatch eltérés vizsgálatakor figyelni kell arra, hogy Triangle és Sine esetekben is történt negatív.

A numerikus teszt main szakasza 4096 minta, tail szakasza rátához kötött. Egy waveform-grid 120 esetet futtat rögzített sorrendben. Az előző esetek setup/render/teardown hatását nem szabad eltüntetni azzal, hogy csak a végül hibásnak jelzett cellát indítjuk külön. A teszt első eltérésnél megállhat: a közölt hiba nem a teljes render maximális hibája.

A publikált 1.2.0 a kalibrált Premium lookup Reference motort használja; a Saw-dispatch Candidate kísérleti maradt. Az eltérés önmagában nem dönti el, hogy a Reference vagy Candidate értéke hibás, és nem igazolja a production motor teljes hibamentességét sem.

## 3. Hogyan fordult elő? Megőrzött negatív esetek

### A. Korai jelzés – `05b730d`

macOS Universal Preview Reference-job, Triangle x86_64/Rosetta összehasonlítás állt meg bitazonossági hibával. Az első eltérő mintát és a hiba nagyságát ebből a korai jelzésből nem sikerült meghatározni; ne pótold későbbi eset számaival.

- [Workflow/job](https://github.com/RobCZart82/SAWSTAR/actions/runs/39126567559/job/117672833349)
- Történeti forrás: [BUGFIX_DEVELOPMENT_PLAN_2026_10_20.md, K1 szakasz](BUGFIX_DEVELOPMENT_PLAN_2026_10_20.md#4-k1--a-116-rosetta-biteltérése).
- További történeti jelentés a lezárt #116 rögzített ágán: [PREVIEW_BIT_IDENTITY_FAILURE](https://github.com/RobCZart82/SAWSTAR/blob/fb3114eb829442223083e103929a15ad25d3efeb/docs/PREVIEW_BIT_IDENTITY_FAILURE_2026_10_18.md). Ez nem a jelenlegi mainben levő fájl.

### B. Részletes Triangle-negatív – `fb3114eb829442223083e103929a15ad25d3efeb`

2026-10-20; [workflow 39250177170](https://github.com/RobCZart82/SAWSTAR/actions/runs/39250177170), Candidate job `118051765492`.

| Tulajdonság | Érték |
|---|---|
| Végrehajtás | x86_64/Rosetta, Triangle cold-run-0 |
| Konfiguráció | Triangle, 48000 Hz, HP12/filter=2, Poly/voice=0, FX ON |
| Első jelzett eltérés | main frame 197, nullától indexelve, output bal |
| Candidate bits | `0x3a5eda95` |
| Reference bits | `0x3a5edab7` |
| Candidate − Reference | `-1.9790604710578918e-09` |
| FP | rounding=0, MXCSR=`0x1fb3` |

A hibás konfiguráció előtt 13 teljes eset futott ugyanabban a Triangle-folyamatban. A későbbi három Triangle-folyamat sikeres volt. A sikeres Reference-job és a hibás Candidate-job **azonos Universal összehasonlító executable-t** őrzött:

`3228d4d226e7e5f6b213773831406b326a74a260340ed738e012efa1009cccd0` (SHA256).

A hibás artifact `12186151118`; ZIP SHA256:
`247fe73cec4300ff828cd529993fdcb9572def3edb9388c9f0918165814fda6c`.
Hibás log SHA256:
`d893ac670bfe10981e3ae55c34593c433363ec7f38ecfd34cd875908c24f2c09`.

Az eredeti parancs **`--diagnostic-waveform 2`**, nem egyszerű `--waveform 2`: az előbbi FNV jelujjlenyomatokat is számol. Más útvonalnak számítanak. Az output ellenőrzés a pre-FX előtt állt, ezért ez a korai hibasor nem bizonyít FX-eredetet.

Részletek: [ROSETTA_NUMERICAL_INVESTIGATION_2026_10_20.md](ROSETTA_NUMERICAL_INVESTIGATION_2026_10_20.md).

### C. Passzív FP-megfigyelés közbeni Triangle-negatív – `001847ba7932a9d65fa38104e7e1b888db770b53`

[Workflow 41724068835](https://github.com/RobCZart82/SAWSTAR/actions/runs/41724068835), job `126224107488`, artifact `13272794936`.

| Tulajdonság | Érték |
|---|---|
| Végrehajtás | Apple Silicon host, x86_64/Rosetta, observed mód |
| Konfiguráció | Triangle/waveform=2, 176400 Hz, filter=0, Poly, FX ON |
| Hívássorrend | Reference → Candidate |
| Első jelzett eltérés | main frame 152, pre-FX jobb |
| Candidate / Reference bits | `0x407b4cb1` / `0x407b4cb2` (1 ULP) |
| Output ugyanennél a mintánál | mindkét csatorna bitazonos |
| FP a négy híváshatárnál | raw `0x1fb3`, controls `0x1f80`, rounding=0, változás nélkül |

Ugyanennek a binárisnak az x86 baseline és candidate-first, valamint az ARM64 próbái sikeresek. Ez egyetlen negatív összevetésből nem bizonyít sorrendfüggő okot. A megfigyelő kód maga is befolyásolhat kódgenerálást/időzítést.

Környezet a manifest szerint: macOS 15.8.2 / build 24H232; Apple clang 17.0.0, Xcode 16.4. Executable SHA256:
`2fb9f21146ade7ee54061846da06886e22460ba90b68490dd754a5831d213115`.

Elsődleges repository-bizonyíték, teljes első eltéréssel és loghashokkal:
[measurements/k1-passive-001847b-negative.json](measurements/k1-passive-001847b-negative.json).

### D. Sine-negatív – `810a38860eaebbdf7660450ccf751cbb13df34e7`

[Workflow 42365411396](https://github.com/RobCZart82/SAWSTAR/actions/runs/42365411396), Reference job `128382317422`, 2026-11-07-i vizsgálati feljegyzés.

| Tulajdonság | Érték |
|---|---|
| Végrehajtás | x86_64/Rosetta a workflow váltakozó sorrendjéből következtetve; nem külön megőrzött translation-probe bizonyítja |
| Konfiguráció | Sine/waveform=3, 48000 Hz, LP24/filter=1, Poly, FX ON |
| Előzmény | 7 teljes eset a hibás folyamatban |
| Első jelzett eltérés | main frame 423 |
| Pre-FX bal Candidate / Reference | `0x3de367ac` / `0x3de367b4` (8 ULP) |
| Output bal Candidate / Reference | `0x3c059802` / `0x3c059808` (6 ULP) |
| Jobb csatorna | pre-FX és output is bitazonos |
| FP | MXCSR `0x1fb3`, rounding=0 |

**Fontos bizonyítékhiány:** a hibás offline összehasonlító executable-t az akkori workflow nem töltötte fel. A teljes eredeti joblog megőrzött, SHA256:
`0abe114f85d085d92e774be262fee2cdd70103b95121a625e54bae2f6144f1cc`.
Újrafordítás e forrásból nem az eredeti executable hashazonos visszajátszása.

Bizonyíték: [measurements/k1-sine-810a388-negative.json](measurements/k1-sine-810a388-negative.json).

**Összegzés:** több, eltérő konfigurációjú tesztnegatív van. A K1 elnevezés közös nyomozási ügyet jelöl, nem bizonyított közös gyökérokot mindegyik esethez.

## 4. Mit próbáltunk, és mit tudtunk meg?

| Vizsgálat | Eredmény | Miért nem okbizonyítás? |
|---|---|---|
| Megőrzött `fb3114e` bináris helyi Triangle replay | 120/120 PASS; külön heap-scribble körrel is 120/120 PASS | Nem az eredeti gépkörnyezet; a heap-scribble nem zár ki stack/inicializálási hibát |
| #124 eredeti bináris CI-replay | 28 folyamat, 3360 esetmegfigyelés, PASS | Hashazonos executable, de más folyamat/környezet; nem reprodukálta a negatívot |
| #126 natív Intel-kontroll | 1680 esetmegfigyelés, PASS | Nem bizonyítja, hogy csak Rosetta alatt lehetséges a hiba |
| #126 Apple Silicon/Rosetta replay | 3360 esetmegfigyelés, PASS | Ritka állapot/környezetfüggést nem zár ki |
| #126 ASan/UBSan: régi és akkori aktuális forrás, ARM64 és Intel | 1920 esetmegfigyelés, nincs motorbeli sanitizer-találat; szándékos hibás kontrollok megfelelően buktak | Más, O1/sanitizer kódgenerálás; nem MSan és nem általános UB-/memóriahelyességi bizonyítás; leak detection OFF |
| Belső mintánkénti jelút-trace | Kijelölt helyi instrumentált Triangle kör: 960 megfigyelés PASS; normál kontroll külön 960 PASS | Instrumentáció módosítja kódot/memóriaképet. Valódi negatív első belső műveletét nem sikerült rögzíteni |
| Passzív MXCSR és render-sorrend | Korai helyi és CI 720-720 megfigyelés PASS; később a C eset negatív lett | Status és controls külön kezelendő; változatlan határponti controls sem zár ki belső átmeneti módváltást |
| Setup FP-proxy #125 | 12 folyamat, 240 teljes grid-eset és rövid próbák PASS; IE megjelenése az első motor Reset() API-intervallumára szűkült | IE itt audioeltérés nélkül jelentkezett; konkrét gépi utasítás és kapcsolat K1-hez nincs |
| #143 célzott Sine helyi kontroll | ARM64/Rosetta: 10 folyamat, 720 megfigyelés PASS | Új executable; az eredeti Sine-bináris hiányzik |
| #143 célzott Sine CI, run 42374811466 | Apple Silicon 10 folyamat/720 megfigyelés; natív Intel 5/360; mind PASS | Normál/passzív/trace egyik sem reprodukálta a negatívot, nem talált gyökérokot |

A darabszámok ismételt **esetmegfigyelések**, nem ennyi különböző konfiguráció. Külön kampányokat nem szabad automatikusan független reprodukcióként összeadni.

A #143 CI összesen 15 folyamat: 6 önteszt + 9 darab 120-esetes grid. Apple clang 17.0.0; megfigyelt x86 controls `0x1f80`. Artifactok: `13575002640` és `13575851496`; a rögzített PR-jelentés szerint mind a 74 payload/loghash és az esetszámok ellenőrzöttek. Forrás: [#143 rögzített eredménye](https://github.com/RobCZart82/SAWSTAR/pull/143), [CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/42374811466). A jelenlegi repository Sine-módszertani lapja még főként a helyi eredményt tartalmazza; a későbbi CI-összegzés a PR-leírásban van.

## 5. MXCSR és korábbi hipotézisek pontosítása

- `0x1fb3`: sticky státuszjelzők IE, DE, UE, PE; FTZ=0, DAZ=0, rounding-control=00; a hat kivétel maszkolt. `status = raw & 0x3f`, `controls = raw & 0xffc0`. A státuszjelző nem FP-módváltás.
- A jelzők korábbi esetekből felhalmozódhatnak. A `0x1fb3` hibamentes futásban is előállt, ezért **nem K1-hibajelző**. A DE/UE nem bizonyítja, hogy a konkrét eltérést denormális aritmetika okozta.
- A passzív kontrollban a DE/UE átmenete az elsőként renderelt motort követte, mindkét sorrendben, audioeltérés nélkül. A setup IE pedig az első Reset() körül jelent meg.
- FTZ/DAZ ON/OFF páros kampányt a hivatkozott korábbi FP-vizsgálatok nem végeztek. Ne írd le sikeres, elvégzett K1-javításként. Egy ON/PASS OFF-negatív kontroll nélkül nem azonosítana okot.
- Saját eredeti executable disassemblyben nem találtak keresett FMA és x87 aritmetikai/load/store mnemonicot. Ez nem vizsgálja az összes dinamikus könyvtárat vagy Rosetta belső fordított kódját. A context-switch/regisztermentési magyarázat nem bizonyított.
- FIR `_mm_loadu_pd` két double-t olvas unaligned módon; nem `_mm_load_ps`. Az `alignas(16)` hiánya önmagában nem hiba ezen az úton. Az akkumulátorok explicit nullázottak.
- A releváns Triangle history és filterállapotok inicializálása át lett nézve; konkrét inicializálatlan függést nem találtunk. Ez nem a teljes objektumgráf memóriahelyességi bizonyítása.
- A FIR-indexképletek modellje 82 944 hivatkozott indexet határon belülinek talált; ez képletmodell, nem a fordított program általános OOB-mentességi bizonyítása.

Részletek: [külső AI-jelentések korábbi értékelése](K1_EXTERNAL_REPORT_REVIEW_2026_10_20.md), [FP-mérés](K1_FP_OBSERVATION.md), [setup-lokalizálás](K1_SETUP_FP_LOCALIZATION.md).

## 6. Mi nem K1-javítás, és mit nem állítunk?

A később reprodukált chorus ring-buffer határhiba külön ügy: post-FX túlindexelés volt, bizonyított kontrollal javítva. Nem magyarázza automatikusan a C és D eset **pre-FX** eltérését. Lásd [CHORUS_RING_BOUNDARY_2026_11_04.md](CHORUS_RING_BOUNDARY_2026_11_04.md).

A B1 vezérlő-lecsengés és B2 SUB-plafon javítása nem magyarázta a B esetet: a releváns vezérlőesemény az 512. mintánál jött, a hiba a 197.-nél, a hangok SUB-célja pedig messze a plafon alatt volt. Preset-lock/GUI/import sem vesz részt a közvetlen egyszálú Synth-API tesztben.

Nem lazítottunk bitazonosságot epsilonra, nem állítottunk át production FTZ/DAZ policyt, és a későbbi zöld futást nem neveztük gyökérok-javításnak. Az 1.2.0 publikálási döntés nem K1 technikai lezárása. A régi dokumentumok egyes „release blokkolt/private draft” mondatai az akkori állapotot őrzik, nem a jelenlegi publikációs státuszt.

## 7. Olvasási sorrend és konkrét kódhelyek

1. `docs/measurements/k1-passive-001847b-negative.json` és `docs/measurements/k1-sine-810a388-negative.json` – géppel olvasható negatívok.
2. `docs/ROSETTA_NUMERICAL_INVESTIGATION_2026_10_20.md` – B eset, eredeti bináris/hash és előzmény.
3. `tests/oscillator_dispatch_engine.cpp` – összehasonlítás, bitguard, grid, main/tail, FP- és trace-módok, CLI. Először ellenőrizd a tényleges eset-/hívássorrendet.
4. `tests/DispatchEngineSetup.h` – közös setup; `tests/k1_setup_fp.cpp` – API-hívás körüli passzív megfigyelő.
5. `CMakeLists.txt` – Reference/Candidate generált források, compile definitions, kapcsolók és célok. A generált fordítási egység és a forrás-template közötti különbség fontos.
6. `src/dsp/Synth.*`, `src/dsp/SevenSaw.*`, `experiments/oscillator/SawDispatchSevenSaw.*` – voice/render/oszcillátor állapot és kísérleti dispatch.
7. `experiments/premium_filter/EnginePremiumFilter.h`, `PremiumDrive.h`, `FirLanes4.h`, `RateScaledPremiumDrive.h`, `LoopReferencePremiumDrive.h`, `ReferencePremiumDrive.h` – közös/eltérő filter- és FIR-utak.
8. `scripts/replay-k1-original.py`, `scripts/run-k1-sanitized.py`, `scripts/investigate-k1-sine.py` és megfelelő `tests/k1_*.py` kontrollok – reprodukciós módszer és eredményvalidálás.
9. `.github/workflows/k1-*.yml`, valamint a macOS Preview workflow – toolchain, futtatási architektúra, környezeti változók és artifactmegőrzés.
10. `docs/K1_ORIGINAL_BINARY_REPLAY.md`, `K1_NATIVE_SANITIZER_RESULTS_2026_10_20.md`, `K1_INTERNAL_TRACE.md`, `K1_SINE_INVESTIGATION_2026_11_07.md` – módszer és korlátok.

Az artifactok lejárhatnak. GitHub Actions-link nem garantáltan letölthető nyers bizonyíték. A verziózott JSON/Markdown megőrzi az ismert adatot; a helyi `outputs` mappák nem feltétlenül verziózottak és a csak-repository olvasó számára nem feltétlenül hozzáférhetők. Ha bináris/disassembly/log kell, pontosan nevezd meg a hiányzó fájlt és az elvárt hashát. Ne állíts elemzett binárist, ha csak a róla szóló manifestet látod.

## 8. Milyen tanácsadó jelentést kérünk?

Kérjük, minden rangsorolt hipotézishez add meg:

1. Konkrét fájl/funkció és az a művelet vagy állapot, amely szerinted eltérhet.
2. Mi támogatja, és mely meglévő negatív vagy sikeres kontroll gyengíti.
3. Magyarázza-e Triangle és Sine előfordulását, pre-FX/kimeneti lokalizációt és a hashazonos executable eltérő futásait; ha nem, ezt írd le.
4. Egy **minimális megkülönböztető kísérletet**, előre rögzített feltételekkel és negatív kontrollal. Mondd meg, melyik eredmény cáfolná a hipotézist.
5. Milyen pontos bizonyítékot kellene elmenteni: első eltérő művelet bemenete/állapota/kimenete, bitminták, definiált mezők, disassembly/compile command, forrás- és executable-hash.
6. Mi változna az instrumentáció miatt, és hogyan maradna meg az eredeti normál kontroll.

Különösen hasznos: annak eldöntése, hogy a fixture/generált motorok ténylegesen azonos szemantikát és állapotelőkészítést kapnak-e; maradhat-e megosztott/statikus/TLS vagy nem inicializált állapot; milyen konkrét aritmetikai út eltérhet azonos bemenetnél. Ezek **vizsgálandó kérdések, nem már bizonyított hibák**.

Ne javasolj korlátlan ismétlést vagy pusztán kedvezőbb eredményért retryt. Ne tekints ASan PASS-t teljes UB-mentességnek, MXCSR sticky biteket oksági bizonyítéknak, vagy sikeres trace-buildet a régi hiba javításának. Elfogadható eredmény az is, hogy a repositoryból nem azonosítható biztos ok – ebben az esetben a legnagyobb információértékű hiányzó adatot és egyetlen következő célzott lépést kérjük.
