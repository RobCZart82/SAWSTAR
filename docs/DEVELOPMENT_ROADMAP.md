# Elfogadott fejlesztési sorrend — 2026-10-08

Ez az aktuális sorrend; a DEVELOPMENT_PLAN_POST_1.0.4.md korábbi szakaszai
fejlesztési naplóként és mérési bizonyítékként maradnak meg. Kritikus bug
vagy piros funkcionális CI megelőzi a következő fejlesztési lépést.

Aktuális audit, 2026-10-09: a #92 izolált SAW-dispatch mérföldköve már
mainben van (`9cf0ba0`). A #93 `a351db4` forrásának négy Windows/macOS
jobja a négy hullámforma külön statikus/modulált teljesmotoros gridjét
elkészítette: összesen 16 grid, sikeres numerikus kontrollokkal. Ez a mérés
elkészült, a 3. lépés CPU-elfogadása azonban nem teljesült. A jobb SAW-medián
mellett alternatív lassulások és vegyes p99/túllépések maradtak; Windows
192 kHz-en mindkét út minden mért blokkja túllépte az offline határidőt.
A #93 beolvadt (`ec3c637`), majd a #94 Classic-denormál javítása is
mainbe került (`504f070`). A részletes
[repository-audit](REPOSITORY_REVIEW_2026_10_09.md) az audit idején rögzített
forrásokat és korlátokat őrzi. A következő nyitott kapu a kódazonos,
ellenőrzött célgépes CPU-ismétlés. Ezt új
[teljesmotoros gyűjtő](DISPATCH_ENGINE_TARGET_MEASUREMENT.md) támogatja:
friss Release build, előzetes numerikus kapuk, két fordított sorrendű kör,
külön referencia–referencia és jelöltgridek mind a négy hullámformán,
statikus/modulált terheléssel. A gyűjtő elkészítése nem maga a Windows/M1
célgépes CPU-elfogadás. A 3. CPU-kapu és a 4–8. lépés nyitott marad.

A #95 gyűjtője már mainben van (`cf1fae0`); a main Windows/macOS,
kódminőségi és workflow-ellenőrzései sikeresek. A következő adatértékelést
read-only archívumellenőrző támogatja: teljes kampány/hash/azonosító-kontroll,
majd külön körönkénti p50/p99 és túllépések. Ez sem célgépes mérés vagy
CPU-elfogadás; a következő érdemi kapu a tényleges Windows/M1 gyűjtés marad.

Az archívumellenőrző `--verify-raw` módja már a teljes nyers blokkgrideket és
a mentett páros eredményeket is összeveti, a meglévő numerikus riportkontraktussal.
Ez további adatellenőrzés; nem új CPU-mérés, és nem zárja le a 3. lépést.

| Lépés | Feladat | Befejezési feltétel |
| --- | --- | --- |
| 1 | #82 modulált deadline-fixture stabilizálása, mérések értékelése | A pontos PR-head minden ellenőrzése sikeres; beolvasztás; statikus/modulált workload külön értelmezése. |
| 2 | Lookup motor komponensenkénti CPU-profil | Ellenőrzött Windows/macOS grid, compiler/forrás/nyers eredet, domináns költségre adat alapú hipotézis. A profil nem additív CPU-felosztás. |
| 3 | Domináns költség célzott optimalizálása | Változatlan minőségi korlátok, teljesmotor-regresszió, páros kis-bufferes p99/túllépés és kódazonos célgépes ismétlés. |
| 4 | Filter/minőség/magasráta-policy; korai natív REAPER-próba | Mac mini M1-en telepített REAPER, tesztplugin és saját audioeszköz: hang/minőség/terhelés ellenőrzése; Windows célgépes kontroll. Dokumentált policy-döntés. |
| 5 | Prémium LP12/LP24/HP12/BP12 production integráció | Meglévő GUI/ID-k megőrzése, Classic-csere, latency/state/automation és régi projektkompatibilitás ellenőrzése. |
| 6 | Presetkoherencia és plugin-életciklus | Import lejátszás alatt, editor bezárás/újranyitás, unload, több példány, külső meghajtó és régi projekt/preset tesztek. |
| 7 | Végleges factory presetek | Az elfogadott hangúton kiegyensúlyozott, ellenőrzött presetkészlet és mentés/visszatöltés. |
| 8 | Végső natív Windows/macOS REAPER és kiadási kapuk | Production build célgépes elfogadása, csomagok/kézikönyvek/verzió/RC; kiadás külön döntés. |

A 4. lépéshez a felhasználó gépén REAPER szükséges, a fejlesztés és offline
CI 1–3. lépéséhez nem. A korai hostpróba és a végső production-elfogadás
külön feladat. Egy GitHub Actions runner zöld tesztje vagy jobb mediánideje
nem bizonyít natív realtime működést. Új policy/integráció csak az addig
nyitott CPU- és minőségkapuk teljesülése után következhet.

Linux plugin és 32 voice továbbra is halasztott; a lezárt click/pop kutatást
nem nyitjuk újra. A következő mérföldkő állapotát a PR/CI és a mérési
forrásazonosító rögzíti, nem a tervezett feladat készre jelölése.

A 2. lépés módszere: [komponensprofil](PREMIUM_COMPONENT_PROFILE.md).

## A 2–3. lépés aktuális állapota

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
