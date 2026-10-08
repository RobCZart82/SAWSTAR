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
