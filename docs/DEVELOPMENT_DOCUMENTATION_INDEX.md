# Fejlesztési dokumentáció — aktuális index és karbantartási rend

Utolsó tartalmi audit: **2026-10-17**. Ez az index választja el az aktuális
végrehajtási dokumentumokat a korábbi kutatási/kiadási feljegyzésektől.
Nem helyettesíti a pontos PR-head/main GitHub-ellenőrzését.

## Aktuális dokumentumok

| Dokumentum | Szerepe |
| --- | --- |
| [Következő release fejlesztési terve](NEXT_RELEASE_DEVELOPMENT_PLAN.md) | Az M0–M9 sorrend, a valós REAPER CPU-terv, a döntési feltételek és a külön végső RC/publikálási kapu. Ez a részletes végrehajtási terv. |
| [Fejlesztési roadmap](DEVELOPMENT_ROADMAP.md) | Kész/implementált/nyitott állapotok rövid követése. A régi dátumozott részek előzmények. |
| [Natív REAPER-adatlap](NATIVE_REAPER_CPU_ACCEPTANCE.md) | Környezet/buildazonosság, teszteset és ablakonkénti bizonyíték, PASS / nyitott / blokkolt döntés. |
| [Ráta-/pufferpolicy előkészítése](RATE_BUFFER_ACCEPTANCE_POLICY.md) | A negatív offline adatokból következő korlátok és a natív döntéshez szükséges következő lépés; nem automatikus működési korlátozás. |
| [Teljes ráta-/pufferösszesítő](measurements/target-rate-readiness-2026-10-17.md) | 576 lezárt cellából 36 körönként külön sor; a referenciaismétlés és a rosszabb eredmények megmaradnak. |
| [Premium Preview](PREMIUM_PREVIEW.md) | A külön tesztplugin hatóköre, identitása, telepítése és korlátai. |
| [Elfogadott hangerőkorrekció](PREMIUM_PREVIEW_LEVELS.md) | A +3 dB wet / dry unity korrekció, factory headroom és Robert 0,44 dB RMS-eltérésre adott elfogadása. |
| [GUI-méret host-state](GUI_SCALE_PROJECT_STATE.md) | A #109-ben implementált javítás és a még nyitott natív projekt-visszatöltési próbák. |
| [Windows célgépes útmutató és állapot](WINDOWS_TARGET_CONTROL.md) | A lezárt Windows-gyűjtés bizonyítéka, azonos forrású kontroll és további natív feladat. |
| [Production benchmark validálása](AUDIO_BENCHMARK_VALIDATION.md) | Eltérő mérési szerepek, régi hibák, változatlan küszöbök, nyers archívum-audit és új időadatok. |

## Ellenőrzött készültség

| Terület | 2026-10-17-i tartalmi állapot |
| --- | --- |
| Nyilvános kiadás | **1.0.4**, 2026-10-02-i publikálás. A következő verzió/dátum nincs kijelölve. |
| Production hangút | Classic. A későbbi teljes Premium-csere cél, nem elkészült integráció. |
| Premium Preview | Külön identitás, alapból OFF; a mostani Preview nem tartalmazza a Saw-dispatch optimalizációt. |
| Zenei hangkarakter és hangerő | Robert elfogadta; a 0,44 dB RMS-különbség marad. További általános gain-emelés nem nyitott feladat. |
| M1 és Windows offline gyűjtés | Mindkét 32 grides kampány, nyers ellenőrzés és forrásazonos összevetés kész. Nem indul újra kedvezőbb számokért. |
| CPU-/natív elfogadás | Nyitott. A jobb p50 nem tünteti el a p99-romlásokat/túllépéseket. A valós REAPER-jegyzőkönyv és a támogatási döntés hiányzik. |
| GUI-scale | Implementált #109; a 75/100/125% natív mentés/újranyitás és példányfüggetlenség elfogadása nyitott. |
| CI-folyamat | #111 beolvadt 15 sikeres workflow / 50 job után, egyszeri tulajdonosi kivétellel a régi piros mainre. A kivétel nem általánosítható. |
| Dokumentációs mérföldkő | #112 beolvadt, 15 workflow / 50 job és az előző main 4 workflow / 11 job sikeres volt. Az új main CI-je külön kapu. |
| Következő M1 részfeladat | #113 numerikus kapuja beolvadt (7 workflow / 18 job). [Külön Candidate-identitás és macOS csomagolási út](NATIVE_PREVIEW_PAIR_QUALIFICATION.md) implementált; friss CI, telepítési próba és Windows VST3-csomagolás szükséges. |
| Release | Native/CPU, minőség/policy, production-integráció, preset/lifecycle, végső RC és külön publikálási engedély még szükséges. |

A CI pillanatképe auditkor a #111 utáni
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
