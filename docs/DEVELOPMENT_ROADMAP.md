# Elfogadott fejlesztési sorrend — állapot: 2026-10-20

**A release-ig vezető részletes végrehajtási terv és a valós REAPER CPU-elfogadás:**
[NEXT_RELEASE_DEVELOPMENT_PLAN.md](NEXT_RELEASE_DEVELOPMENT_PLAN.md).
[Dokumentációs index és aktuális/historikus besorolás](DEVELOPMENT_DOCUMENTATION_INDEX.md).
Ez írja le a mérföldköveket, a 28 esetes natív mátrixot, a döntési feltételeket
és a külön végső RC-kaput. A lenti roadmap tömör állapotkövetés.

**Új audit javítási munkacsomagja:**
[BUGFIX_DEVELOPMENT_PLAN_2026_10_20.md](BUGFIX_DEVELOPMENT_PLAN_2026_10_20.md).
B1 [#118-ként beolvadt](B1_CONTROL_SMOOTHING_FIX.md), CI sikeres; B2 [#119-ként beolvadt](B2_SUB_FREQUENCY_FIX.md), 12 PR-workflow sikeres; új main CI külön. Natív elfogadás nyitott. R1 [#120-ként beolvadt](R1_PRESET_MUTATION_CONTENTION.md), hét PR-workflow sikeres; új main/natív/I/O-próba
külön; K1 a #116 ismert numerikus blokkolója. A D/Q/T feladatok külön követik a
dokumentációt, metadata-policyt és a tényleges natív lefedettséget. A részletes
M0–M9 kiadási tervbe bekötve; az implementáció és az elfogadás külön követendő.

## K1 passzív FP-/sorrendmérés a #122 után

#121 és #122 beolvadt; a következő diagnosztika alapja `848e4fcc`.
A [passzív FP-mérés](K1_FP_OBSERVATION.md) helyileg elkészült: a státuszjelzők
beállása az elsőként futó motort követi, a vezérlők/kimenetek nem tértek el.
Ez új adat, nem K1-gyökérok vagy javítás. Pontos-head CI és az eredeti
negatív eset vizsgálata továbbra szükséges.

## K1 belső diagnosztika a #120 után

A #120 R1-részjavítás beolvadt, mind a hét PR-workflow sikeres. Új main:
`c6507db0886096ec999d2dd2517c2776f3702f86`; induláskor három fő ellenőrzése
még futott, lintje sikeres. A [K1 belső trace](K1_INTERNAL_TRACE.md) külön
offline diagnosztikai munka, nem DSP-javítás vagy production-promóció.
A K1 oka és a numerikus kapu továbbra nyitott.

## B1 implementáció a #117 után

A folytatás alapja main `ea4961c544ae4f01eec1b610ab229511074e9cd7` (#117).
A [B1 javítás](B1_CONTROL_SMOOTHING_FIX.md) #118-ként beolvadt, mind a 17
PR-workflow sikeres. Az új main `feda3ea9d3f62f01905b16f187e8490fe7088a4c`
négy fő workflow-ja sikeres. A [B2 javítás](B2_SUB_FREQUENCY_FIX.md) erre az
alapra készült és #119-ként beolvadt, 12 PR-workflow sikeres. Új main
`661fe6147fe3539ee160e7236ecb555c98053f1b`; ellenőrzése induláskor még futott.
Az R1 részjavítás erre készül; saját CI és natív/CPU-elfogadás külön.
A lenti korábbi sikeres CI-pillanatkép a `547f84d4` commithoz tartozik.

## Aktuális állapot és publikálási cél

Robert 2026-10-15-én elfogadta az alábbi fejlesztési tervet; a cél a
Premium filteres SAWSTAR következő nyilvános kiadása. A verziószám még
nincs kijelölve; a jelenlegi nyilvános kiadás 1.0.4. A publikálás külön döntés.

Korábbi auditmain (2026-10-20): `547f84d4f72a792aaf0b229c9d73b7a8a00fae40` (#115).
Négy fő workflow completed/success; [ellenőrzési hivatkozások](BUGFIX_DEVELOPMENT_PLAN_2026_10_20.md#10-az-audit-ellenőrzési-alapja-és-korlátai).
#116 beolvasztás nélkül lezárt; megőrzött head `fb3114eb829442223083e103929a15ad25d3efeb`. Az eredeti Rosetta-
biteltérés oka ismeretlen. M1 numerikus elfogadása ezért továbbra blokkolt.
A #115 Windows x64 csomagolási út beolvadt. A jövőbeli tesztbuild pontos-head
CI-je és a mindkét célgépes telepítési/natív próba külön kapu.

**Korábbi, 2026-10-18-i implementációs bizonyíték (#114):**
A #114 16 sikeres releváns workflow és 55 job után beolvadt; előtte a #113
utáni main négy fő workflow-ja és 11 jobja is sikeres volt. Az új main CI-je
külön kapu; a PR eredményét nem másoljuk át automatikusan rá.
Az M1 numerikus ellenőrzője és külön Candidate-identitása implementált;
a macOS Universal Reference/Candidate tesztcsomagok CI-je és mindkét szelet
validátora sikeres. A [Windows x64 build-/csomagolási út](PREMIUM_PREVIEW_WINDOWS.md)
implementált, friss pontos-head CI/validator-ellenőrzése szükséges.
Mindkét célgép tényleges telepítési smoke-ja és a
[natív REAPER-/CPU-elfogadás](NATIVE_PREVIEW_PAIR_QUALIFICATION.md) nyitott.
Production továbbra Classic; a meglévő Preview marad a változatlan referencia.
A #111 az összes 15 workflow és 50 job sikere után squash merge-gel beolvadt.
Robert 2026-10-17-én kizárólag erre a diagnosztikai PR-re engedélyezte a korábbi,
piros main előfeltételének egyszeri feloldását; a GitHub-védelmek megmaradtak.
A merge utáni main négy fő workflow-ját külön kell ellenőrizni; az egyszeri
kivétel nem vihető tovább a következő PR-re. A régi `8ad1b835` main
`dry_fx / 16 voice` +26,49%-os CPU-hibája megőrzött negatív bizonyíték;
a #111 nem DSP-gyorsítás és nem CPU-/natív elfogadás.
[Bizonyíték és diagnózis](AUDIO_BENCHMARK_VALIDATION.md#új-negatív-main-eredmény--2026-10-17).
A gyermekfolyamat-CPU-idő és a teljes hívás eltelt ideje külön rögzül;
az importált gyűjtő módosítása az A/A workflow-t is indítja. A friss PR-head
Windows/macOS kontrolljai sikeresek voltak, az eredeti hibát nem törlik.

A #112-vel elkészült a [ráta-/pufferpolicy előkészítése](RATE_BUFFER_ACCEPTANCE_POLICY.md)
és a [teljes, körönként külön célgépes összesítő](measurements/target-rate-readiness-2026-10-17.md).
A lezárt 576 cellából generált 36 sor nem indít új mérést és nem jelöl
működési tartományt elfogadottnak; következő a natív REAPER-bizonyíték.
**#102, #103, #104, a javító #106, #105, #107, #108, #109, #110, #111, #112, #113, #114, #115, #117, #118, #119 és #120 beolvadt; #116 beolvasztás nélkül lezárt.**
A #109 GUI-state implementációja mainben van; natív REAPER-elfogadása nyitott.
A #109 utáni main Code quality első futása 600 másodperces tesztidőtúllépéssel hibás volt;
a második kísérlet sikeres, az eredeti negatív napló megmarad.
[A teljes 480 esetet megtartó tesztfelbontás](DISPATCH_IDENTITY_CI_TIMEOUT.md)
a #110-ben beolvadt javítás; nem nyitott implementációs feladat.
A jelen fejlesztési fázis külön PR-head-/main-ellenőrzést igényel.
Minden új head összes releváns workflow-ját újra ellenőrizzük; merge után
a main négy fő workflow-ja legyen sikeres a következő merge előtt.

- A #102 LF/CRLF checkout-javítása és a teljes `38735317728` kampány
  mindkét platformgyűjtése és Linuxos nyersellenőrzése/összevetése sikeres.
  Ez CI-bizonyíték, nem célgépes CPU-elfogadás. A korábbi hibás kampány
  és negatív mérési eredmények megmaradnak.
- A #103 külön, alapból kikapcsolt Premium Preview buildet ad; a normál
  production SAWSTAR továbbra is Classic filteres.
- **Hangerőkorrekció: kész, Robert elfogadta és lezárta.** A #105 korrigált
  Preview-jának 16 hangos, cutoff-automatizált REAPER-próbájában a Premium
  RMS-szintje 0,44 dB-lel alacsonyabb a Classicnál; ez tudatosan elfogadott,
  további általános hangerőemelés nem szükséges. A részletes bizonyíték a
  [hangerő-dokumentumban](PREMIUM_PREVIEW_LEVELS.md) szerepel.
- A hangkarakter pozitív natív visszajelzést kapott a kipróbált környezetben.
  A CPU-, további mód-/rátaminőségi és végső production-kapuk külön nyitottak.

- **Mac M1 célgépes motoridő-gyűjtés és nyers kiértékelés: kész.** 2026-10-16-án
  32/32 grid, két teljes kör, minden nyers hash és percentilis/túllépés ellenőrizve.
  A Saw p50 javult, de p99-romlás és túllépések maradtak; **CPU-elfogadás továbbra nyitott**.
  [Eredmények és korlátok](measurements/mac-target-arm64-2026-10-16.md).
  **Windows x64 gyűjtés és nyers/Mac-összevetés is kész:** ugyanaz a commit és
  mind a 20 forráshash, 32/32 grid, két kör. Saw p50 mindkét körben javult
  2,8–4,8%-kal, de p99-romlások és sok túllépés maradt; ez nem CPU-elfogadás.
  [Windows eredmény és korlátok](measurements/windows-target-x64-2026-10-16.md).
  A ráta-/pufferkészenlét összesítése kész; következő a végleges policy döntése és a
  [natív REAPER/dropout-jegyzőkönyv](NATIVE_REAPER_CPU_ACCEPTANCE.md) kitöltése.
  Nem indítunk újra lezárt Mac-/Windows-kampányt kedvező eredményért.
- A #106 buildpolicy-/nyersbizonyíték-javítása és a #105 azonos binárisos CI-időzítési
  kontrollja beolvadt. A korábbi piros mérések történeti bizonyítékai megmaradnak.

## Elfogadott hátralévő munkasorrend

| Sorrend | Feladat | Befejezési feltétel |
| --- | --- | --- |
| M0 audit | B1 javítás és K1 külön okfeltárás; R1 vizsgálat | A [hibajavítási terv](BUGFIX_DEVELOPMENT_PLAN_2026_10_20.md) bizonyíték/teszt/CI és érintett natív feltételei teljesülnek. A K1 numerikus blokkoló nem zárható le pusztán új zöld futással. |
| 1 | Korábbi #102–#105 PR-sorozat — kész | #102/#103/#104/#106/#105 beolvadt, a végső main négy fő workflow-ja sikeres. #104 dokumentáció, nem GUI-fix. |
| 2 | Célgépes CPU-elfogadás — nyitott | M1 és Windows azonos forrású gyűjtése, nyers ellenőrzése és összevetése kész, p99/túllépés-elfogadás nincs. Következő a ráta-/pufferpolicy és natív REAPER-kontroll; 16 hang, statikus/modulált terhelés, ráták és kis pufferek; p99, időkeret-túllépés és dropout értékelve. Dokumentált elfogadási döntés, nem végtelen optimalizálás. |
| 3 | Filterminőség és magasráta-policy | LP12/LP24/HP12/BP12, rezonancia, Drive, moduláció és módváltás; a minőség/költség és támogatott működési tartomány dokumentált elfogadása. |
| 4 | Premium production-integráció | Elfogadott motor és hangerőkalibráció beépítése; meglévő GUI/ID-k, latency, bypass, state, automatizálás és régi projektbetöltés ellenőrzött. A Classic teljes cseréje a korábbi tulajdonosi döntés szerint. |
| 5 | GUI-méret host-state javítása — implementálva, natív próba nyitott | 75/100/125% külön editorrekordban, hangszínformátum változatlan; codec/host-átadási regresszió kész. Még szükséges a macOS/Windows REAPER-próba, példányfüggetlenség, presetváltás és projektmentési jelzés natív ellenőrzése. |
| 6 | Presetkoherencia és plugin-életciklus | Mentés/import lejátszás alatt, editor bezárás/újranyitás, unload, több példány, külső meghajtó és projekt-visszatöltés rendben. |
| 7 | Végleges factory presetkészlet | A production motoron minden preset zenei és szintellenőrzése, megfelelő headroom, mentés/visszatöltés; az elfogadott általános korrekció megtartása. |
| 8 | Végső natív Windows/macOS REAPER-elfogadás | Pontos production RC telepítése, GUI, MIDI, automatizálás, lejátszás/render, ráták/pufferek, state és lifecycle dokumentált ellenőrzése. |
| 9 | Dokumentáció és kiadási csomag | Roadmap/kézikönyv/install/release notes friss, verzió/RC kijelölt; a hallható filtercsere leírva; végleges csomagok, hashok és VST3-validator sikeresek. |
| 10 | Publikálás | Pontos végleges commit minden kiadási kapuja teljesült, RC elfogadott, külön publikálási jóváhagyás. |

Kritikus bug és piros funkcionális CI megelőzi a következő lépést.
Linux plugin, 32 voice és függőségfrissítés halasztott; a lezárt click/pop
kutatást nem nyitjuk újra.

## GUI/state feladat

A jelentett hiba a DAW-projekt újranyitásakor a 75%-os GUI alapméretre visszaállása volt.
A javítás implementálva és helyi codec/host-átadási regresszióval ellenőrizve;
a natív Windows/macOS REAPER-elfogadás még nyitott.
[Hibaleírás és elfogadási feltételek](GUI_SCALE_PROJECT_STATE.md).

## Történeti állapotfeljegyzések

Az alábbi korábbi állapotok mérési és fejlesztési előzmények. A bennük
szereplő „nyitott”, „következő” és korábbi main-azonosítók a feljegyzés
idejére vonatkoznak; az aktuális sorrendet a fenti táblázat határozza meg.


## Kampány-ellenőrzési hiba és javítás — 2026-10-15

A #102 mindkét teljes platformgyűjtése sikeres, de az összevetés a Windows
CRLF és macOS LF forrásbyte-ok miatt piros. Mind a 20 eltérést az eredeti
Git blobokból és a letöltött artifactok hash-eiből igazoltuk. LF checkout-
szabály és valós Git-regresszió készült; a javított head teljes új kampánya
még szükséges. Nincs hash-/küszöblazítás, CPU-elfogadás vagy soron kívüli merge.
[Bizonyíték, javítás és új kapu](DISPATCH_CHECKOUT_BYTES.md).

Ez az aktuális sorrend; kritikus bug vagy piros funkcionális CI megelőzi a
következő fejlesztési lépést. A [részletes terv](DEVELOPMENT_PLAN_POST_1.0.4.md)
korábbi dátumozott részei történeti naplóként maradnak meg.

## Aktuális állapot — 2026-10-15

Ellenőrzött main: `767a478a6646d7740a96d65ac74ef3e43018db5e` (#101).
A main Windows/macOS, Code quality és workflow-validálási futásai sikeresek.
Nyilvános kiadás: **1.0.4**; a következő kiadás verziója még nincs kijelölve.
A kiadott csomag és a main, illetve a kutatási prémium motor külön állapotok.

| Mérföldkő | Lezárt fejlesztés | Fennmaradó korlát |
| --- | --- | --- |
| #93 (`ec3c637`) | A négy hullámforma külön statikus/modulált teljesmotoros Windows/macOS kampánya elkészült: 16 grid, sikeres numerikus kontrollok. | A CPU-elfogadás nem teljesült; jobb SAW-medián mellett p99/túllépések és alternatív lassulások maradtak. |
| #94 (`504f070`) | Classic filter inaktív módjainak denormáljavítása, underflow-regresszióval. | Nem prémium production integráció. |
| #95 (`cf1fae0`) | Azonos forrású, friss Release buildes célgépes gyűjtő; külön referenciaismétlés és jelölt, két fordított sorrendű kör. | Az eszköz elkészülte nem Windows/M1 célgépes mérési eredmény. |
| #96 (`933882a`) | Teljes archívum hash-, kampány- és riportazonosító-ellenőrzése; körönként külön p50/p99 és túllépések. | A hashok belső egyezést, nem az archívum hitelességét bizonyítják. |
| #97 (`61e9205`) | `--verify-raw`: nyers gridek újraellenőrzése és a páros eredmények összevetése, változatlan numerikus szerződéssel. | A 32 grides regresszió szintetikus adatpróba; nincs új célgépes CPU- vagy natív elfogadás. |

### Beolvadt: megerősítő párbeszéd hosszú presetnevekkel

A mentés megerősítő szövege hosszú, szóköz nélküli presetnévnél csak
szóközökre támaszkodva tördelődött, így kifuthatott a szövegdobozból.
A külön javítás UTF-8-kódponthatáron is tördel, a betűszélességet méri,
és a szükséges sorszámhoz igazítja az ablakmagasságot. A helyi regresszió
ellenőrzi a szöveg megőrzését, Unicode karaktereket és a gombok előtti helyet.
A #99 javítás beolvadt; a main Windows/macOS buildje, Code quality és
workflow-validálása sikeres. A natív, skálázott GUI-ellenőrzés még nyitott;
a DSP, paraméterazonosítók és a 3. lépés CPU-elfogadása nem változik.
[Részletek és ellenőrzési korlátok](CONFIRMATION_TEXT_WRAP.md).

**Következő nyitott feladat:** ugyanazon forrás kontrollált Windows/M1
teljesmotoros CPU-ismétlése, mind a négy hullámformán, statikus és modulált
terheléssel; kis pufferek p99-ideje és határidő-túllépései külön értékelendők.
[Gyűjtés és archívumellenőrzés](DISPATCH_ENGINE_TARGET_MEASUREMENT.md).
Az offline méréshez nem kell REAPER; a korai natív hostpróbához igen.

A két célgép lezárt archívumához külön összevető parancs is használható:
`scripts/compare-dispatch-engine-targets.py`. Mindkét teljes archívum nyers
adatait újraellenőrzi, azonos commitot/forráshashokat és körszámot követel,
és megőrzi a platformonként külön referenciaismétlést, jelöltet és köröket.
Ez kiértékelési eszköz, nem új CPU-eredmény vagy elfogadás; a Windows/M1
lezárt célgépes mérési pár továbbra sincs e dokumentumban igazolva.

A production audio-benchmark CSV-ellenőrzése külön teljességi javítást kap:
minden warmup/időzített futás mind a hat `dry_fx`/`full_fx`, 1/8/16 voice
jelenetet tartalmazza, és a CPU-idő pozitív legyen. A CPU-, szint- és
DC-küszöbök változatlanok; ez nem gyorsulás vagy prémium CPU-elfogadás.
[Hiba, javítás és tesztszerződés](AUDIO_BENCHMARK_VALIDATION.md).

### Beolvadt: mérési validátorok és CI karbantartása

A #101 a production benchmark hat jelenetének teljességét és CSV-szerkezetét
ellenőrzi. A folytatás minden jelenlegi warmup/időzített render DC-jét külön
vizsgálja, és a kutatási deadline-riportokban kizárja a duplikált oszlopokat,
eltérő sorszélességet és hibás CSV-t. Regressziós tesztek igazolják a régi
hibákat és a megengedett határeseteket. A Windows/macOS build workflow-k
pushra csak a mainen futnak; PR- és kézi indítás továbbra is elérhető.
A #101 beolvadt; a `70a7fab` PR-head mind a hét workflow-ja és a
`767a478` main mind a négy workflow-ja sikeres.

A korábbi `1ed6d698` macOS push-futás CPU-regresszióját megőrizzük:
`dry_fx / 16 voice`: 11,6468% → 15,0679%, határ 14,5584%.
A PR-futások sikere nem törli ezt a mérést; nincs automatikus újrapróbálás,
kedvező kör kiválasztása vagy küszöblazítás. A változtatás nem érinti a DSP-t.
[Részletek, regressziók és CI-bizonyíték](AUDIO_BENCHMARK_VALIDATION.md).

### Folyamatban: teljes páros CI CPU-kampány — 2026-10-15

Az új `Full dispatch CPU campaign` a meglévő gyűjtővel platformonként
két teljes kört futtat: referenciaismétlés és jelölt, statikus és modulált
terhelés, négy hullámforma, 48/96/192 kHz és 32/64/128 buffer. A 32 grid
platformonként 2 359 296 nyers blokkot őriz meg. A numerikus kapuk minden
időzítés előtt futnak; a gyűjtésen belül nincs párhuzamos CPU-mérés.
Mindkét teljes archívum független nyers újraellenőrzést kap, majd azonos
forrású platformközi összevetés készül, minden kört és negatív eredményt
megtartva. A mérési hibák részadatai is artifactként megmaradnak.

Ez hosted Windows/macOS CI-bizonyíték. Nem bizonyít saját M1 hardvert,
kontrollált háttérterhelést, CPU-nyereséget vagy natív REAPER-elfogadást.
A kampány eredménye és a Windows/M1 célgépes ismétlés továbbra is nyitott;
a 3. lépést nem jelöljük készre az adatgyűjtés puszta sikerétől.
[Futtatás, eredet és kiértékelés](DISPATCH_ENGINE_TARGET_MEASUREMENT.md).

A roadmap 1–2. lépése elkészült, a 3. lépés CPU-elfogadása nyitott.
Ezt követi a minőség/magasráta-policy és korai REAPER-próba, a prémium filter
teljes Classic-cseréje a meglévő GUI/ID-kkel, latency/state/automation,
presetkoherencia és plugin-életciklus, végleges factory presetek, majd natív
Windows/macOS kiadási kapuk és külön publikálási döntés. A production motor
még a Classic filtert használja. Függőségfrissítés, Linux plugin és 32 voice
továbbra is halasztott; a lezárt click/pop kutatást nem nyitjuk újra.

| Lépés | Állapot | Feladat | Befejezési feltétel |
| --- | --- | --- | --- |
| 1 | Kész | #82 modulált deadline-fixture stabilizálása, mérések értékelése | A pontos PR-head minden ellenőrzése sikeres; beolvasztás; statikus/modulált workload külön értelmezése. |
| 2 | Kész | Lookup motor komponensenkénti CPU-profil | Ellenőrzött Windows/macOS grid, compiler/forrás/nyers eredet, domináns költségre adat alapú hipotézis. A profil nem additív CPU-felosztás. |
| 3 | Nyitott | Domináns költség célzott optimalizálása | Változatlan minőségi korlátok, teljesmotor-regresszió, páros kis-bufferes p99/túllépés és kódazonos célgépes ismétlés. |
| 4 | Nyitott | Filter/minőség/magasráta-policy; korai natív REAPER-próba | Mac mini M1-en telepített REAPER, tesztplugin és saját audioeszköz: hang/minőség/terhelés ellenőrzése; Windows célgépes kontroll. Dokumentált policy-döntés. |
| 5 | Nyitott | Prémium LP12/LP24/HP12/BP12 production integráció | Meglévő GUI/ID-k megőrzése, Classic-csere, latency/state/automation és régi projektkompatibilitás ellenőrzése. |
| 6 | Nyitott | Presetkoherencia és plugin-életciklus | Import lejátszás alatt, editor bezárás/újranyitás, unload, több példány, külső meghajtó és régi projekt/preset tesztek. |
| 7 | Nyitott | Végleges factory presetek | Az elfogadott hangúton kiegyensúlyozott, ellenőrzött presetkészlet és mentés/visszatöltés. |
| 8 | Nyitott | Végső natív Windows/macOS REAPER és kiadási kapuk | Production build célgépes elfogadása, csomagok/kézikönyvek/verzió/RC; kiadás külön döntés. |

A 4. lépéshez a felhasználó gépén REAPER szükséges, a fejlesztés és offline
CI 1–3. lépéséhez nem. A korai hostpróba és a végső production-elfogadás
külön feladat. Egy GitHub Actions runner zöld tesztje vagy jobb mediánideje
nem bizonyít natív realtime működést. Új policy/integráció csak az addig
nyitott CPU- és minőségkapuk teljesülése után következhet.

Linux plugin és 32 voice továbbra is halasztott; a lezárt click/pop kutatást
nem nyitjuk újra. A következő mérföldkő állapotát a PR/CI és a mérési
forrásazonosító rögzíti, nem a tervezett feladat készre jelölése.

A 2. lépés módszere: [komponensprofil](PREMIUM_COMPONENT_PROFILE.md).

## Történeti fejlesztési napló

Az alábbi szakaszok a korábbi munkacsomagok sorrendjét és mérési eredményeit
őrzik. A bennük szereplő „következő” feladatok az akkori állapotra vonatkoznak;
az aktuális nyitott kapukat a fenti összefoglaló és táblázat adja. A #93
kampánya és a #95–#97 eszközei már elkészültek, a célgépes CPU-elfogadás nem.
A [2026-10-09-i audit](REPOSITORY_REVIEW_2026_10_09.md) korábbi megállapításait
és mérési forrásazonosítóit nem írjuk át későbbi állapotként.

## A 2–3. lépés korábbi állapota

A Windows/macOS komponensprofil két teljes adatsora és provenance-a
rögzített; a nyers és összesítő byte-hash ellenőrzött. 48/96 kHz-en a
Drive/FIR, 192 kHz-en az oszcillátorok költsége is érdemi célpont.
A #85-ben a 2× SIMD interpoláció cikluskibontása elkészült, változatlan
műveleti sorrenddel és befagyasztott bitazonossági oracle-lel. Az
`a427081` forrás mind a 43 ellenőrzése sikeres; main: `6d7c2ac`.
A teljesmotoros és független referencia-minősítés sikeres, a CPU-elfogadás
viszont nem teljesült: Windows 192 kHz-en 1–2% mediánregresszió, macOS-en
vegyes p99/túllépés. A jelölt kísérleti marad. Részletek és mind a négy
platform/workload riport: [FIR cikluskibontás](PREMIUM_FIR_UNROLL_STUDY.md).

A 3. lépés következő kapuja reprodukálható, azonos forrású célgépes
CPU-ismétlés; a géppel feldolgozható páros riport ezt támogatja. Az
oszcillátorköltség a következő optimalizálási hipotézis, de előbb a
komponensprofil és teljesmotoros eredmények alapján kell kiválasztani a
konkrét algoritmust. A 4. lépés natív REAPER-próbája és az 5. lépés
production integrációja továbbra is nyitott.


## Oszcillátorvizsgálat és következő munkacsomag

A #87 beolvadt (`e830145`); a végleges `f8609b0` PR-head mind a 45
ellenőrzése és a main Windows/macOS/Code quality futása sikeres.
A cache v2 statikus SAW-nál gyorsulási jelet adott, de Windows alatt
modulált SAW-nál 4,5–5,7% lassulást mutatott. A teljesmotoros elfogadás
nem teljesült: a cache kísérleti marad. A v1/v2 nyers és platformos
negatív eredmények megőrzendők; nem kerülnek más jelölt mérésébe.
Részletek: [cache-vizsgálat](OSCILLATOR_FREQUENCY_CACHE_STUDY.md).

A következő izolált hipotézis a mintán belül ismételt frekvenciaszámítás
megosztása. Minden setter továbbra is fut; nincs cache-állapot, feltételes
frekvenciafrissítés vagy új pitch-simítás. Az új jelölt külön variánsjelzést,
bitazonossági kontrollt és statikus/audio-rate pitch-modulált páros
Windows/macOS mérést kap. Ha ez nem ad következetes nyereséget, nem
integráljuk a teljes motorba. Kedvező izolált eredmény után is külön
numerikus és modulált teljesmotor-deadline kapu, célgépes ismétlés,
minőség/rátapolicy és natív REAPER-elfogadás következik.
Részletek: [frekvenciaszámítás-megosztás](OSCILLATOR_SHARED_FREQUENCY_STUDY.md).

## A #88 platformos döntése és a következő mérés

A #88 már mainben van (`cbffa34`); az `eae61b7` pontos PR-head mind a 47
ellenőrzése sikeres. A 37829386024 futam nyers CPU-adatainak és compiler
fájljainak byte-hash ellenőrzése sikeres. A Windows grid 24 cellájából 21
mediánban lassabb, 13 cella mind a négy párban lassabb. A modulált SAW
mediánja 9,05–10,00% lassulást mutat. A macOS grid 13 mediánlassulást és
nagy páronkénti szóródást mutat, egyetlen cella sem lassabb mind a négy párban.
Ez nem következetes platformos CPU-nyereség: nincs teljesmotoros aktiválás.
A beolvasztás a külön kutatási eszközökre vonatkozik, a shipping DSP változatlan.

A következő lépés a mérési megbízhatóság ellenőrzése: 131 072 frame/pár,
nyolc váltakozó pár, külön változatlan SevenSaw/SevenSaw kontroll és jelölt
futás. A két összehasonlítás ugyanazon executable-t használja, külön
azonosítóval és teljes nyers adattal. Nincs outlier-kizárás, kontrollból
levont gyorsulás vagy CPU-küszöb lazítása. A hosszabb timer nem kontrollálja
a runner háttérterhelését. Ha az új mérés sem támasztja alá a nyereséget,
a jelölt kísérleti marad; a komponensprofil alapján más célpontot választunk.
A 4–8. lépés és a natív/minőségi/CPU-kapuk változatlanul nyitottak.

A hosszabb helyi M1-próba elkészült: 118/118 teszt sikeres; külön-külön
384 soros referencia- és jelöltgrid, egyező páros energia. Square/Triangle/Sine
mediánban kedvező, SAW vegyes (0,994258–1,009570). A kontroll mediánja
−0,38% és +0,67% között, egyes párok jóval szélesebben szóródnak. A régi
Windows-lassulás nem törlődik; az új platformos futam a következő döntési
kapu. A teljes eredmény és korlátai a frekvenciamegosztási jegyzetben vannak.

## A #89 hosszabb platformmérése — 2026-10-09

A #89 beolvadt (`30f1cc5`); a `c097858` pontos PR-head 21/21 ellenőrzése
és a main 10/10 ellenőrzése sikeres. A hosszabb platformkampány elkészült,
nem várakozó feladat. Windows alatt a jelölt 12/24 cellában medián szerint,
6/24 cellában mind a nyolc párban lassabb; a SAW/Square minden cellája
mediánban lassabb. Triangle/Sine kedvezőbb, de ez nem a fő SAW út javítása.
macOS-en 5/24 jelöltcella mediánban lassabb, a referenciaismétlés mediánjai
is szélesebben szóródnak. A korábbi negatív eredmények érvényesek maradnak.

A következő lépést a [célgépes gyűjtő](OSCILLATOR_TARGET_MEASUREMENT.md)
támogatja: friss, azonos forrású Release build, numerikus kapu, két fordított
kampánysorrendű kör és teljes eredetjegyzék. A mérések elkülönítettek maradnak;
nincs automatikus zajlevonás vagy CPU-elfogadás. A megosztott jelölt kísérleti.
Az izolált gyűjtő regressziója szimulált parancsfuttatással ellenőrzi a
sorrendet, hibás grid/teszt és változó executable elutasítását; ez nem
Windows/M1 célgépes teljesítménymérés. A 4–8. lépés nyitott marad.

## Workflow-indítási javítás — 2026-10-09

A #90 beolvadt (`ec2d463`), de a megosztott oszcillátor workflow-ja
konfigurációs hiba miatt nem indult el: `runner.temp` job-szintű `env`
mezőben nem használható. A 45 sikeres check-run ezt nem mutatta meg;
a failed workflow-futam nulla jobot tartalmazott. Az új négyszeres
Windows/macOS mérés ezért még nem tekinthető elvégzettnek.

A javítás a futás közbeni `RUNNER_TEMP` környezeti változóból képezi az
eredménymappát; az artifact lépés saját, megengedett `runner` kontextust
használ. Külön, minden PR-en futó, verzióra és SHA-256-ra rögzített
actionlint ellenőrzi az összes workflow szintaxisát és kontextusát, ismert
hibás job-env negatív kontrollal. A következő kapu a validálás és a valóban
elindult, sikeres Windows/macOS mérés. A CPU/native elfogadás nyitott marad.

Beolvasztás előtt a pontos forráscommit check-runjai mellett a workflow-
futamok állapotát is külön ellenőrizzük: a job nélkül elbukott workflow
nem zöld kapu. A javítás megelőzi az új DSP-kísérletet.

## A #91 lezárt mérése és az új SAW-hipotézis — 2026-10-09

A fenti workflow-indítási hiba már javítva: a #91 beolvadt (`7983958`),
a `02dca9d` pontos head 20 check-runja és mind a hét workflow-futama sikeres.
A valódi Windows/macOS gyűjtő platformonként két kör / négy teljes kampányt
futtatott, 1536 nyers sorral. Ez a mérési feladat elkészült; nem várakozó kapu.

Windows alatt a frekvenciamegosztási jelölt 12/24, majd 14/24 cellában
medián szerint lassabb; 7/24, majd 6/24 mind a nyolc párban lassabb.
Minden SAW-cella mediánja lassabb mindkét körben: 1,028877–1,084684 és
1,029480–1,077297 study/reference arány. macOS-en mindkét kör 6/24
mediánlassulást és széles referencia-szóródást mutat. Nincs általános
production aktiválás. A külön körök és a korábbi negatív adatok megmaradnak.
[Nyolc riport és eredet](../experiments/oscillator/measurements/2026-10-09-shared-frequency-target-ci-summary/provenance.json).

A következő izolált jelölt a tisztán SAW feldolgozás bankonkénti dispatch-e:
az inaktív alternatívák feltételeit egyszer vizsgálja a hét oszcillátor
belső ciklusa helyett. Nem a korábbi cache vagy frekvenciamegosztás
kombinációja. A SAW setterei, frekvenciaszámítása, súlya és összegzési
sorrendje megmarad; átmenet alatt az eredeti általános út dolgozik.
Külön variáns, hosszú átmeneti bitazonossági regresszió és két körös
Windows/macOS referencia/jelölt mérés következik.
[SAW-dispatch vizsgálat](OSCILLATOR_SAW_DISPATCH_STUDY.md).

A 3. lépés CPU-elfogadása és a 4–8. lépés továbbra is nyitott.
Kedvező izolált mérés után is teljesmotoros modulált deadline-próba,
azonos forrású célgépes ismétlés, minőség/rátapolicy és natív REAPER kell.

## A #92 lezárt izolált mérföldköve - 2026-10-09

A #92 beolvadt (`9cf0ba0`); a `f0a06cd` pontos PR-head mind az 50
check-runja és mind a 14 workflow-futama sikeres. A SAW-dispatch külön
Windows/macOS, két körös mérése elkészült. Minden SAW-cella mediánja
kedvezőbb: Windows körülbelül 12-23%, macOS 7-20% kisebb bankidő.

| Platform / kör | Jelölt mediánban lassabb | Jelölt mind a nyolc párban lassabb | SAW mediáni study/reference tartomány |
| --- | ---: | ---: | ---: |
| windows / 1 | 8 / 24 | 2 / 24 | 0.771983-0.880968 |
| windows / 2 | 9 / 24 | 4 / 24 | 0.780816-0.878055 |
| macos / 1 | 15 / 24 | 2 / 24 | 0.844562-0.927996 |
| macos / 2 | 12 / 24 | 0 / 24 | 0.804728-0.925081 |

Az alternatív hullámformáknál lassulások is vannak; a macOS referencia-
ismétlés mediánjai is szélesen szóródnak. A körök és hullámformák külön
maradnak, nincs kontrollból levonás, outlier-kizárás vagy CPU-elfogadás.
[Nyolc riport és eredet](../experiments/oscillator/measurements/2026-10-09-saw-dispatch-ci-summary/provenance.json).
A fájlhash-ek a joblogból kinyert JSON-archívumot ellenőrzik; az eredeti
nyers CSV/compiler artifactokat ebben a lépésben nem töltöttük újra le.

A következő kapu a külön [teljesmotoros dispatch-próba](OSCILLATOR_DISPATCH_ENGINE_STUDY.md):
azonos lookup filter és Drive mellett csak OSC1/OSC2 változik. A négy
hullámforma külön statikus/modulált p99 és deadline-túllépés gridet kap,
bitazonos hanggal és hullámformaváltási regresszióval. A kedvezőtlen
alternatív ágakat ugyanúgy értékeljük. Az új platformos mérés még nyitott.
A production DSP nem változik. A 3. lépés CPU-elfogadása és a 4-8. lépés
továbbra is nyitott; kódazonos célgépes ismétlés és natív REAPER szükséges.


