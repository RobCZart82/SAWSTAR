# Elfogadott fejlesztési sorrend — állapot: 2026-10-16

## Aktuális állapot és publikálási cél

Robert 2026-10-15-én elfogadta az alábbi fejlesztési tervet; a cél a
Premium filteres SAWSTAR következő nyilvános kiadása. A verziószám még
nincs kijelölve; a jelenlegi nyilvános kiadás 1.0.4. A publikálás külön döntés.

Ellenőrzött main: `2f401939db95c4da51e80d041e8911d0258f3b39` (#105).
A Windows/macOS build, Code quality és workflow-validálás sikeres.
**#102, #103, #104, a javító #106 és #105 beolvadt.** A korábbi merge-sor lezárult.
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
  A [Windows célgépes kontroll](WINDOWS_TARGET_CONTROL.md) pontosan a Macen mért
  commitból indul, forrásbyte-előellenőrzéssel; a
  [natív REAPER-jegyzőkönyv](NATIVE_REAPER_CPU_ACCEPTANCE.md) előkészítve.
  Ez még nem Windows-/natív mérési eredmény vagy elfogadás.
- A #106 buildpolicy-/nyersbizonyíték-javítása és a #105 azonos binárisos CI-időzítési
  kontrollja beolvadt. A korábbi piros mérések történeti bizonyítékai megmaradnak.

## Elfogadott hátralévő munkasorrend

| Sorrend | Feladat | Befejezési feltétel |
| --- | --- | --- |
| 1 | Soros beolvasztás — kész | #102/#103/#104/#106/#105 beolvadt, a végső main négy fő workflow-ja sikeres. #104 dokumentáció, nem GUI-fix. |
| 2 | Célgépes CPU-elfogadás — nyitott | M1 gyűjtés/nyers ellenőrzés kész, p99/túllépés-elfogadás nincs. Következő az azonos forrású Windows célgépes mérés és natív REAPER-kontroll; 16 hang, statikus/modulált terhelés, ráták és kis pufferek; p99, időkeret-túllépés és dropout értékelve. Dokumentált elfogadási döntés, nem végtelen optimalizálás. |
| 3 | Filterminőség és magasráta-policy | LP12/LP24/HP12/BP12, rezonancia, Drive, moduláció és módváltás; a minőség/költség és támogatott működési tartomány dokumentált elfogadása. |
| 4 | Premium production-integráció | Elfogadott motor és hangerőkalibráció beépítése; meglévő GUI/ID-k, latency, bypass, state, automatizálás és régi projektbetöltés ellenőrzött. A Classic teljes cseréje a korábbi tulajdonosi döntés szerint. |
| 5 | GUI-méret host-state javítása | 75/100/125% példányonként mentődik és visszaáll; presetváltás nem írja át, régi state betölthető; macOS/Windows REAPER-próba. |
| 6 | Presetkoherencia és plugin-életciklus | Mentés/import lejátszás alatt, editor bezárás/újranyitás, unload, több példány, külső meghajtó és projekt-visszatöltés rendben. |
| 7 | Végleges factory presetkészlet | A production motoron minden preset zenei és szintellenőrzése, megfelelő headroom, mentés/visszatöltés; az elfogadott általános korrekció megtartása. |
| 8 | Végső natív Windows/macOS REAPER-elfogadás | Pontos production RC telepítése, GUI, MIDI, automatizálás, lejátszás/render, ráták/pufferek, state és lifecycle dokumentált ellenőrzése. |
| 9 | Dokumentáció és kiadási csomag | Roadmap/kézikönyv/install/release notes friss, verzió/RC kijelölt; a hallható filtercsere leírva; végleges csomagok, hashok és VST3-validator sikeresek. |
| 10 | Publikálás | Pontos végleges commit minden kiadási kapuja teljesült, RC elfogadott, külön publikálási jóváhagyás. |

Kritikus bug és piros funkcionális CI megelőzi a következő lépést.
Linux plugin, 32 voice és függőségfrissítés halasztott; a lezárt click/pop
kutatást nem nyitjuk újra.

## GUI/state feladat

A DAW-projekt újranyitásakor a 75%-os GUI alapméretre áll vissza.
A javítás még nincs implementálva.
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


