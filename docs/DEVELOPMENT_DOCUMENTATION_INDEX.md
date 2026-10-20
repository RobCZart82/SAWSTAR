# Fejlesztési dokumentáció — aktuális index és karbantartási rend

Utolsó állapotfrissítés: **2026-10-20**; a történeti dokumentációaudit 2026-10-17-én készült. Ez az index választja el az aktuális
végrehajtási dokumentumokat a korábbi kutatási/kiadási feljegyzésektől.
Nem helyettesíti a pontos PR-head/main GitHub-ellenőrzését.

## K1 — natív kontrollok és sanitizer-lefedettség

#125 beolvadt (`bef9b8db`), 11 PR-workflow / 26 job sikeres.
Az IE az elsőként futó motor Reset() hívásánál megfigyelt, audioeltérés nélkül;
ez nem bizonyított K1-ok. A [következő elkülönített kampányok](K1_NATIVE_AND_SANITIZER_CONTROLS.md)
az eredeti binárist natív Intel macOS-en, az eredeti és aktuális kalibrált
Preview-párt pedig natív ARM64/Intel ASan/UBSan buildben vizsgálják.
A futó main-ellenőrzések megmaradnak; új PR-head CI-je külön kapu.
K1 továbbra megőrzött, nem újra előidézett Rosetta alatti numerikus anomália.

## K1 — eredeti replay eredménye és setup-lokalizálás

#124 beolvadt (`16f024e3`), öt PR-workflow / 14 job sikeres.
Az [eredeti bináris kampánya](K1_ORIGINAL_BINARY_REPLAY.md) mindkét archívumból,
28 folyamat / 3360 esetmegfigyelés alatt hibamentes: K1 most nem reprodukált.
Következő a külön [setup-műveletek FP-lokalizálása](K1_SETUP_FP_LOCALIZATION.md):
33 hívás motoronként, coarse/observed/fordított setup-sorrend, közös grid-helperrel.
Ez új diagnosztika, nem bizonyított K1-ok vagy javítás; friss CI-je külön kapu.

## K1 — eredeti executable kontrollja a #123 után

#123 beolvadt (`9383d46d`), mind a 19 PR-workflow / 59 job sikeres.
Az [eredeti bináris CI-visszajátszása](K1_ORIGINAL_BINARY_REPLAY.md) külön
kampányként implementált: két eredeti archívum, hashazonos executable,
14–14 rögzített folyamat, az eredeti `--diagnostic-waveform 2` Triangle-úttal.
A kampány eredménye és az új main teljes CI-je külön kapu; K1 továbbra nyitott.

## Aktuális dokumentumok

| Dokumentum | Szerepe |
| --- | --- |
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

## Folytatás ellenőrzési alapja

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

## Passzív FP-folytatás a #122 után

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
