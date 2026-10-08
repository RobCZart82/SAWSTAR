# Elfogadott fejlesztési sorrend — 2026-10-08

Ez az aktuális sorrend; a DEVELOPMENT_PLAN_POST_1.0.4.md korábbi szakaszai
fejlesztési naplóként és mérési bizonyítékként maradnak meg. Kritikus bug
vagy piros funkcionális CI megelőzi a következő fejlesztési lépést.

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
