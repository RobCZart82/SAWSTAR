# Fejlesztési dokumentáció — aktuális index és karbantartási rend

## Aktuális cél: 1.2.1 hibajavító kiadás — 2026-11-08

Az 1.2.0 2026-11-07-én megjelent, a kalibrált Premium hangúttal és 30 factory
presettel. A #144 diagnosztika beolvadt `c82e26d` mainre; mind a négy főági
workflow sikeres. K1 továbbra ismeretlen eredetű, nem javított.
A mostani végrehajtási alap az [1.2.1 javítási és elfogadási terv](RELEASE_1.2.1_PLAN.md).
Az alábbi korábbi RC-célok és kapuk dátumozott történeti állapotok; nem az
aktuális publikációs állapotot vagy újonnan megadott elfogadást jelentik.


## K1 név szerinti filter/FIR-diagnosztika — 2026-11-08

Az [AI-átadási jelentés](K1_AI_DEBUG_HANDOFF_2026_11_08.md) megőrzi a külön
negatívokat és a sikertelen okfeltárási próbák korlátait. A
[mély filterállapot-nyomkövetés](K1_FILTER_STATE_FIRST_DIVERGENCE_2026_11_08.md)
implementált és helyileg ellenőrzött: az aktuális és öt korábbi minta név szerinti
bitmintái, FIR-lane és SVF köztes pontok, külön offline kapcsoló. Egy véges helyi
Rosetta-körben nincs reprodukció; távoli CI még külön kapu, K1 nem javított.
Az alábbi dátumozott állapotok történeti feljegyzések.


## Célzott K1 Sine-vizsgálat — 2026-11-07

Robert a publikálás elé helyezte a konkrét 810a388 negatív oknyomozását.
[Előre kijelölt kontrollok, bizonyítékok és eredmények](K1_SINE_INVESTIGATION_2026_11_07.md).


## 1.2.0 kiadási döntés — 2026-11-07

Robert a 003b007 Premium RC Windows/macOS működését hibamentesnek jelezte és
engedélyezte a publikálást. A végleges kiadás explicit Premium buildet és
telepítőket kap. [Kiadási döntés és ellenőrzések](RELEASE_1.2.0_CHECKLIST.md),
[ismert K1 eltérés és használati korlátok](RELEASE_NOTES_1.2.0.md).
Az alábbi korábbi RC-bejegyzések dátumozott történeti állapotok.


## Történeti kiadási cél: 1.2.0 rc1 — 2026-11-05

Robert az 1.2.0 publikálását jelölte ki célként és engedélyezte az előkészítő
fejlesztés folytatását. [Kiadási sorrend, bizonyítékok és nyitott kapuk](RELEASE_1.2.0_CHECKLIST.md).
[Az előkészítés naplója](DEVELOPMENT_LOG_2026_11_05_RELEASE.md).
Az akkori végrehajtási alap; az alábbi dátumozott feljegyzések
történetiek. A hat új exportált preset zenei jóváhagyása megtörtént; a teljes
pontos RC elfogadása ettől külön marad. Az 1.1.0 rc1 korábbi fejlesztési jelölt,
nem publikált kiadás. Az új metaadat 1.2.0 rc1, üres kiadási dátummal.


## Forrásaudit és ARP bypass teljesítményjavítás — 2026-11-05

[Igazolt találatok, javítások, helyi tesztek és nyitott vizsgálati lépések](SOURCE_AUDIT_2026_11_05.md).
A kikapcsolt ARP felesleges mintarendezése megszűnik; nyolc elavult src README
frissül. A teljes natív CI és a kiadási kapuk külön ellenőrzendők.


## Windows/macOS működési visszajelzés — 2026-11-05

Robert a legutóbb linkelt main buildet Windows és macOS alatt is
hibamentesnek jelezte. [Visszajelzés, csomagkapcsolat és hatókör](NATIVE_HOST_FEEDBACK_2026_11_05.md).
Mindkét platform alapműködésére van pozitív felhasználói tapasztalat.
A részletes CPU-/dropout-/GUI/state/preset adatok és a külön Premium RC
elfogadása ettől még nem teljes. Következő: a konkrét RC fennmaradó
célzott ellenőrzései **(sajátgépes reaper)**; külön publikálási jóváhagyás.


## Új kérés: performance factory bank — 2026-11-03

Robert kérésére hat új, modulált/effektezett/arp hang bővíti a bankot 24-ről
30-ra, a korábbi hangok megőrzésével. [Hanglista, játék és ellenőrzési kapuk](EXPRESSIVE_FACTORY_PRESETS_2026_11_03.md).
[Velvet Orbit: bizonyított chorus-puffercímzési hiba és regresszió](CHORUS_RING_BOUNDARY_2026_11_04.md).
A hat exportált új hang zenei elfogadása megtörtént; a teljes RC natív elfogadása külön kapu.


## #133 beolvadt, main CI sikeres — 2026-11-03

A Premium integráció és RC-előkészítés #133-ként beolvadt `b9f35574` mainre.
A pontos PR-head 22 workflow / 67 job, az új main 5 workflow / 12 job sikeres.
A tényleges macOS Universal/Windows x64 RC-k és a három factory CSV ellenőrzött.
[Teljes CI-/csomagbizonyíték, dirty-provenance és RC-PDF állapot](PREMIUM_RC_CI_EVIDENCE_2026_11_03.md).
A 15 oldalas magyar/angol RC-kézikönyvek elkészültek; a végleges publikáláskor
a jóváhagyott buildhez újra felülvizsgálandók. A native/zenei/K1 kapuk nem
zárultak le. Az alábbi korábbi „helyi/függő” bejegyzések történeti állapotok.


[Aktuális fejlesztési napló és ellenőrzési állapot](DEVELOPMENT_LOG_2026_11_03.md).

## Aktuális fejlesztési lépcső — 2026-11-03

#132 beolvadt `d0bad8edd44e418c1a7b3527a2e20f862b0ebc4a` mainre:
a végső PR-head 23 workflow / 71 job sikeres; a merge utáni main mind a négy
workflow-ja sikeres (macOS 41570979286, Windows 41570979317,
Code quality 41570979245, workflow-validálás 41570979369).
A két korábbi negatív CPU-futás megmarad, a későbbi zöld nem gyökérok-bizonyítás.

Robert kérésére az M4/M6/M8 előkészítése előrekerült az M2/M3 lezárása elé.
[Premium production RC: működés, migráció és kapuk](PREMIUM_PRODUCTION_RC.md).
Az 1.1.0 rc1 explicit SAWSTAR-identitású integráció implementált; a kalibrált
lookup referencia motor, eredeti paraméter-ID-k és state-formátum, 32 mintás
késleltetés. Az alapértelmezett Classic build megmarad az elfogadásig.
[Factory műszaki mérés és nyitott hallgatási kapu](PREMIUM_FACTORY_RC_2026_11_03.md).
A helyi ellenőrzés nem távoli CI, natív host-, K1- vagy publikálási elfogadás.
Következő: új integrációs PR minden CI-je, validált RC-csomagok, majd a nyitott
M2/M3/M5 és pontos RC host/zenei elfogadási feltételek teljesítése.
A lentebb dátumozott korábbi állapotok történetiek.


Utolsó állapotfrissítés: **2026-11-08**; a történeti dokumentációaudit 2026-10-17-én készült. Ez az index választja el az aktuális
végrehajtási dokumentumokat a korábbi kutatási/kiadási feljegyzésektől.
Nem helyettesíti a pontos PR-head/main GitHub-ellenőrzését.

## Helyi auditjavítások — 2026-11-02

A #131 utáni `b24f445` mainre készülő javítások: a maradék macOS 14
workflow-k migrációja, LFO zero-target underflow, kedvencek korlátlan
mutexvárakozásának eltávolítása és CHANGELOG-frissítés.
[Implementáció, negatív kontroll, helyi ellenőrzés és nyitott kapuk](AUDIT_FIXES_2026_11_02.md).
Ezek helyi változások; távoli CI és natív elfogadás külön szükséges.
K1 és a release-kapuk státusza változatlan.

## K1 — aktuális állapot a #126 után

#126 beolvadt (`e2b27054`), hét PR-workflow / 22 job sikeres.
Az eredeti bináris natív Intel-kontrollja 1680, újabb Apple Silicon/Rosetta
replay-e 3360, a négy kalibrált ASan/UBSan konfiguráció 1920 esetmegfigyeléssel
hibamentes. A nyers csomagok helyileg újraellenőrzöttek.
[Eredmények, bizonyíték és következő döntés](K1_NATIVE_SANITIZER_RESULTS_2026_10_20.md).

K1 megőrzött, nyitott, most nem újra előidézett numerikus anomália.
A kijelölt diagnosztikai kör befejezett; további Rosetta/IE-próba új negatív
esethez vagy konkrét hipotézishez kötött. A következő prioritás a natív
Reference/Candidate host-/REAPER-elfogadás. Production/RC/publikálási kapu
ettől nem teljesült; az új main és e dokumentáció friss CI-je külön kapu.

## Aktuális dokumentumok

| Dokumentum | Szerepe |
| --- | --- |
| [1.2.1 release-előkészítési napló — 2026-11-08](DEVELOPMENT_LOG_2026_11_08_RELEASE.md) | A jelen hibajavítási lépcső helyi implementációja, ellenőrzése és még nyitott távoli/felhasználói kapui. |
| [1.2.1 javítási és elfogadási terv](RELEASE_1.2.1_PLAN.md) | Kiadás előkészítési sorrendje; candidate-, natív host- és külön publikálási kapuk. |
| [macOS felhasználói alappróba — 2026-10-27](NATIVE_HOST_SMOKE_2026_10_27.md) | Robert sikeres Reference/Candidate működési/hang-visszajelzése; ellenőrzött telepített hashok, hiányzó mérési adatok és következő sajátgépes lépés. |
| [K1 natív és sanitizer-eredmények](K1_NATIVE_SANITIZER_RESULTS_2026_10_20.md) | A #126 mért eredményei, helyi raw-data audit, nyitott K1 és a natív host-elfogadásra visszatérő prioritás. |
| [K1 natív/sanitizer módszertan](K1_NATIVE_AND_SANITIZER_CONTROLS.md) | Az eredeti Intel-bináris és az új instrumentált kalibrált pár külön tesztterve és értelmezési korlátai. |
| [Következő release fejlesztési terve](NEXT_RELEASE_DEVELOPMENT_PLAN.md) | Az M0–M9 sorrend, a valós REAPER CPU-terv, a döntési feltételek és a külön végső RC/publikálási kapu. Ez a részletes végrehajtási terv. |
| [Átfogó hibajavítási munkacsomag — 2026-10-20](BUGFIX_DEVELOPMENT_PLAN_2026_10_20.md) | B1/B2 reprodukált numerikus jelenségek, K1 ismert Rosetta-blokkoló, R1 zárolási gyanú, D/Q/T dokumentációs és validációs feladatok; bizonyíték, sorrend és lezárási feltétel. |
| [Fejlesztési roadmap](DEVELOPMENT_ROADMAP.md) | Kész/implementált/nyitott állapotok rövid követése. A régi dátumozott részek előzmények. |
| [Natív REAPER-adatlap](NATIVE_REAPER_CPU_ACCEPTANCE.md) | Környezet/buildazonosság, teszteset és ablakonkénti bizonyíték, PASS / nyitott / blokkolt döntés. |
| [Ráta-/pufferpolicy előkészítése](RATE_BUFFER_ACCEPTANCE_POLICY.md) | A negatív offline adatokból következő korlátok és a natív döntéshez szükséges következő lépés; nem automatikus működési korlátozás. |
| [Teljes ráta-/pufferösszesítő](measurements/target-rate-readiness-2026-10-17.md) | 576 lezárt cellából 36 körönként külön sor; a referenciaismétlés és a rosszabb eredmények megmaradnak. |
| [Premium Preview](PREMIUM_PREVIEW.md) | A külön tesztplugin hatóköre, identitása, telepítése és korlátai. |
| [Elfogadott hangerőkorrekció](PREMIUM_PREVIEW_LEVELS.md) | A +3 dB wet / dry unity korrekció, factory headroom és Robert 0,44 dB RMS-eltérésre adott elfogadása. |
| [GUI-méret host-state](GUI_SCALE_PROJECT_STATE.md) | A #109-ben implementált javítás és a még nyitott natív projekt-visszatöltési próbák. |
| [Windows natív Preview-tesztcsomag](PREMIUM_PREVIEW_WINDOWS.md) | Külön Reference/Candidate x64 build, factory-azonossági és csomagolási kapuk, telepítési smoke. |
| [Windows célgépes útmutató és állapot](WINDOWS_TARGET_CONTROL.md) | A lezárt Windows-gyűjtés bizonyítéka, azonos forrású kontroll és további natív feladat. |
| [Production benchmark validálása](AUDIO_BENCHMARK_VALIDATION.md) | Eltérő mérési szerepek, régi hibák, változatlan küszöbök, nyers archívum-audit és új időadatok. |

## Történeti folytatási alapok — #117–#125

Az alábbi induláskori CI-pillanatképek történetiek; az akkori „még futott”
mondatok nem a jelenlegi függő feladatokat jelölik. Az aktuális K1-eredmény
és döntés a dokumentum elején található.

A #117 beolvadt, új main `ea4961c544ae4f01eec1b610ab229511074e9cd7`.
A B1 erre az alapra készült és #118-ként beolvadt. A folytatáskor ellenőrzött
main `feda3ea9d3f62f01905b16f187e8490fe7088a4c`: mind a négy fő workflow
sikeres, a #118 mind a 17 PR-workflow-ja szintén sikeres.
A B2 #119-ként beolvadt, mind a 12 PR-workflow sikeres. Új main:
`661fe6147fe3539ee160e7236ecb555c98053f1b`; ellenőrzése a munka kezdetén
még futott (lint sikeres). Az R1 részjavítás erre készül, saját CI-je külön kapu.
#120 időközben beolvadt, mind a hét PR-workflow sikeres. A K1-folytatás
alapja `c6507db0886096ec999d2dd2517c2776f3702f86`; új main CI-je induláskor
még futott, lintje sikeres. #116 beolvasztás nélkül lezárult; K1 továbbra nyitott.
[Belső K1 trace és következő döntés](K1_INTERNAL_TRACE.md).

## Történeti passzív FP-folytatás a #122 után

#121 beolvadt, 18 PR-workflow sikeres. A külső jelentések értékelése #122-ben
beolvadt, négy PR-workflow sikeres. Az új alap
`848e4fccc495a46d4e1c2f8b83970cb2cf70cb3e`; friss main CI-je külön kapu.
A [passzív FP-/sorrendmérés](K1_FP_OBSERVATION.md) implementált és helyileg
ellenőrzött. A `8f617709` FP-workflow 720 nulla eltérésű esetet adott;
az ugyanazon CI-binárissal végzett helyi 120 esetes visszajátszás teljes
FP-naplója egyezik a CI-ével. Az IE már setup alatt megjelenik, audiohiba nélkül.
[CI-bizonyíték](measurements/k1-passive-fp-ci-2026-10-20.json).
Új PR-head teljes CI-je külön kapu. K1 továbbra nyitott.

## Ellenőrzött készültség

| Terület | 2026-10-20-i tartalmi állapot |
| --- | --- |
| Nyilvános kiadás | **1.0.4**, 2026-10-02-i publikálás. A következő verzió/dátum nincs kijelölve. |
| Production hangút | Classic. A későbbi teljes Premium-csere cél, nem elkészült integráció. |
| Premium Preview | Külön identitás, alapból OFF; a mostani Preview nem tartalmazza a Saw-dispatch optimalizációt. |
| Zenei hangkarakter és hangerő | Robert elfogadta; a 0,44 dB RMS-különbség marad. További általános gain-emelés nem nyitott feladat. |
| M1 és Windows offline gyűjtés | Mindkét 32 grides kampány, nyers ellenőrzés és forrásazonos összevetés kész. Nem indul újra kedvezőbb számokért. |
| CPU-/natív elfogadás | Nyitott. A jobb p50 nem tünteti el a p99-romlásokat/túllépéseket. A valós REAPER-jegyzőkönyv és a támogatási döntés hiányzik. |
| GUI-scale | Implementált #109; a 75/100/125% natív mentés/újranyitás és példányfüggetlenség elfogadása nyitott. |
| Új audit / B1 és B2 | [B1](B1_CONTROL_SMOOTHING_FIX.md) #118-ként beolvadt, CI sikeres; [B2](B2_SUB_FREQUENCY_FIX.md) #119-ként beolvadt, 12 PR-workflow sikeres, új main CI külön. Natív elfogadás és REAPER CPU-hatás nincs kimérve. |
| R1 többpéldányos preset-lock | [Process-local részjavítás](R1_PRESET_MUTATION_CONTENTION.md) #120-ként beolvadt, hét PR-workflow sikeres; új main és natív kétpéldányos/I/O-elfogadás külön. |
| K1 / #116 | PR beolvasztás nélkül lezárt, megőrzött head `fb3114eb829442223083e103929a15ad25d3efeb`; az eredeti Rosetta-biteltérés oka ismeretlen. [Új belső trace](K1_INTERNAL_TRACE.md) implementált; saját CI és K1-okfeltárás külön, későbbi PASS nem feloldás. |
| CI-folyamat | #111 beolvadt 15 sikeres workflow / 50 job után, egyszeri tulajdonosi kivétellel a régi piros mainre. A kivétel nem általánosítható. |
| Dokumentációs mérföldkő | #112 beolvadt, 15 workflow / 50 job és az előző main 4 workflow / 11 job sikeres volt. Az új main CI-je külön kapu. |
| Következő M1 részfeladat | #114 és #115 beolvadt, a külön macOS/Windows x64 csomagolási út implementált. #116 numerikus ügye nyitott; a mindenkori tesztbuildhez friss CI és mindkét célgép telepítési/REAPER-próbája szükséges. |
| Release | Native/CPU, minőség/policy, production-integráció, preset/lifecycle, végső RC és külön publikálási engedély még szükséges. |

**Friss ellenőrzési alap, 2026-10-20:** main
`547f84d4f72a792aaf0b229c9d73b7a8a00fae40` (#115). A négy fő workflow
completed/success: [Code quality](https://github.com/RobCZart82/SAWSTAR/actions/runs/39081228932),
[lint](https://github.com/RobCZart82/SAWSTAR/actions/runs/39081229021),
[macOS](https://github.com/RobCZart82/SAWSTAR/actions/runs/39081229022),
[Windows](https://github.com/RobCZart82/SAWSTAR/actions/runs/39081228948).
Ez a kiinduló main ellenőrzése, nem az új dokumentációs PR/merge CI-je és nem natív elfogadás.

**Történeti CI-pillanatkép:** a 2026-10-17-i dokumentációaudit a #111 utáni
`f1b559fa6a74f0931bfe748e432c93644cfcce06` main volt:
[Windows](https://github.com/RobCZart82/SAWSTAR/actions/runs/39037844252),
[macOS](https://github.com/RobCZart82/SAWSTAR/actions/runs/39037844306) és
[workflow lint](https://github.com/RobCZart82/SAWSTAR/actions/runs/39037844286)
sikeres, [Code quality](https://github.com/RobCZart82/SAWSTAR/actions/runs/39037844321)
a #112 merge-je előtt szintén sikeres volt. A #112 új mainje
`d434bf03c179afa4d09717ec796bd9eb825d0861`, annak CI-jét külön kell ellenőrizni.
Ez dátumozott ellenőrzési alap, nem állandó zöld állítás.
Új main/head esetén a futásokat újra kell olvasni; függő CI nem tekinthető késznek.

## Tartós mérési bizonyíték

- [M1 eredmény és eredet](measurements/mac-target-arm64-2026-10-16.md),
  [géppel olvasható 288 cella](measurements/mac-target-arm64-2026-10-16.json).
- [Windows eredmény és eredet](measurements/windows-target-x64-2026-10-16.md),
  [géppel olvasható 288 cella](measurements/windows-target-x64-2026-10-16.json).
- [Korábbi zöld PR / hibás main nyers production bizonyíték](measurements/production-ci-variance-2026-10-17.json).
- [Első külön processz-CPU / eltelt idő CI-bizonyíték](measurements/production-ci-invocation-timing-2026-10-17.json).

A CSV-k, hashok, manifestek és régi negatív eredmények nem írhatók át az
aktuális státusz megváltoztatásáért. Egy későbbi zöld futás önmagában nem
magyarázza meg a korábbi hibát. Az offline motoridő, a REAPER CPU% és a
hallható dropout külön bizonyíték. A hiányzó adat N/A/nyitott, nem kitalált nulla.

## Történeti dokumentumok és eszközreferenciák

- [1.0.4 utáni fejlesztési napló](DEVELOPMENT_PLAN_POST_1.0.4.md),
  [1.0.3 utáni roadmap](DEVELOPMENT_ROADMAP_POST_1.0.3.md),
  [első release terve](FIRST_RELEASE_PLAN.md): az akkori készültség és döntések.
- [Premium filter kutatási napló](PREMIUM_FILTER_DEVELOPMENT.md),
  komponens-, FIR-, gain/lookup-, cache/shared/dispatch tanulmányok: a
  konkrét jelöltek és mérési lépések bizonyítékai, nem mai feladatlisták.
- [LF/CRLF hiba és javítás](DISPATCH_CHECKOUT_BYTES.md): a régi hibás
  kampány és a már sikeres javítás története; nem ismételt kampányindítási feladat.
- [Teljesmotoros gyűjtő](DISPATCH_ENGINE_TARGET_MEASUREMENT.md),
  [izolált oszcillátorgyűjtő](OSCILLATOR_TARGET_MEASUREMENT.md): eszközleírás,
  nem a lezárt kampány újrafuttatására adott utasítás.
- [RC1](RELEASE_CANDIDATE.md), [RC2](RELEASE_CANDIDATE_2.md),
  [régi distribution terv](DISTRIBUTION_PLAN.md),
  [1.0.4 publikálási jegyzőkönyv](RELEASE_1.0.4_CHECKLIST.md): megőrzött
  verzióspecifikus feljegyzések; a korábbi engedély nem új publikálási engedély.

A történeti dokumentumokban maradó „következő”, „még nincs” vagy „aktuális”
szó az ott megnevezett korábbi dátumra vonatkozik. A naprakész feladatot a
fenti release terv és roadmap adja meg; nincs szükség a régi nyers eredmények átírására.

## Karbantartási szabály minden fejlesztési fázishoz

1. Frissen olvasd a main és az érintett PR state/draft/head/base adatait,
   az exact head összes releváns workflow-ját, checkjét és jobját.
2. Vezesd át a roadmapben az implementált és a ténylegesen elfogadott állapotot
   külön. Az új kód, a CI-siker és a natív elfogadás ne legyen egyetlen „kész”.
3. Ha változik a következő munka vagy a tesztmátrix, az aktuális release tervet
   és a natív adatlapot együtt frissítsd; az eszközreferenciák mutassanak ezekre.
4. A régi naplót/megszakadt vagy hibás mérést őrizd meg dátummal és forrással.
   Elavult bevezetés elé kerüljön egyértelmű történeti státusz és aktuális link.
5. Merge csak az aktuális fej minden releváns sikeres workflow-ja/jobja és a
   szükséges main-kapuk után, squash + expected head SHA-val. Nincs vak auto-merge,
   force vagy védelmi bypass. #111 egyszeri kivétele nem használható újra.
6. Merge után az új main CI-jét is ellenőrizd; a függő futást ne nevezd sikeresnek.
7. A README fejlesztési linkjei az aktuális indexhez/tervhez vezessenek;
   a kiadott termék kézikönyve ne állítson befejezetlen fejlesztést kiadott funkciónak.
