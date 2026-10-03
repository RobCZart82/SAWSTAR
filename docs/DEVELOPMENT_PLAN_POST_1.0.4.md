# SAWSTAR — fejlesztés az 1.0.4 kiadás után

Frissítve: 2026-10-03. A kiadási összegzés történeti alapja: `2d3fd73527ecdad27028792d898b37b2d4bf28de`.
Az akkori main Windows, macOS és Code quality workflow-ja sikeres volt. A #36 PR lezárta
az ARP transport-stop utáni editor-tulajdonlás, a VST3 host MIDI-kontrollerpontok
és a jövőbeli editor-offsetek overflow-helyreállításának javítását.

## Aktív prioritás 2026-10-03

A tulajdonos új sorrendje szerint a következő release fő fejlesztési iránya
**a prémium filter**. A többi felsorolt megbízhatósági és használhatósági téma
az aktív terv része marad. Csak a 32 voice vizsgálata és a Linux plugin kerül
biztosan a soron következő release utánra.

1. Filterreferencia és külön kutatási prototípus: cutoff/rezonancia mérés,
   hosszabb bevezetőjű hangminták, stabilitás több mintavételi frekvencián.
2. A jelölt hallásos értékelése, rezonancia és telítés tervezése; a nemlineáris
   részekhez anti-aliasing/túlmintavételezés és CPU-költség külön ellenőrzése.
3. Kompatibilis engine-integráció: régi preset/projekt a régi karaktert használja;
   az új karakter kiválasztása és alapértelmezése kifejezett döntés. A prototípus
   önmagában még nem kerül a plugin jelútjába.
4. Presetkoherencia reprodukció, import/mentés késésének profilozása,
   fájlrendszer-megbízhatóság és hagyományos automatizálás blokkfüggésének mérése.
   A reset/kerék és natív lifecycle kombinációk ugyanitt kapnak célzott tesztet.
5. Gyors presetlista ABC-sorrendje és kategóriái, majd az elfogadott filterhez
   cutoff-központú factory presetek. Az új termékfunkciók scope-ja követhető PR-ekben
   legyen rögzítve; bizonyított kritikus hiba mindig megelőzi a hangminőségi munkát.
6. Végleges CPU- és kompatibilitási próba, Windows/macOS CI, natív elfogadás,
   kézikönyvek és új kiadási csomagok. A következő verziószám külön rögzítendő.

A rezonancia-vizsgálat #45 alatt beolvadt (`6c48ac0`): minden CI sikeres.
A tulajdonos az 50%-os karaktert lead mintán preferálta, pluck és pad mintán
is kellemesnek hallotta. A külön, négyszeres mintavételű Drive kutatási
prototípus elkészült; hallásos értékelése, szélesebb aliasing-mérése és
CPU-profilozása következik. A #46 Drive referencia minden CI után beolvadt
(`cc41769`). A 24 dB tiszta, szélsőséges maximumként elfogadható; a tulajdonos
szokásos használatra kb. 20 dB-ig tekerne. A 20 dB kontroll és az első,
referenciával mintánként egyező optimalizálás elkészült. A 16 voice költség
különösen 96/192 kHz-en még integrációt akadályozó tervezési feladat.
A 20 dB-os lead mintát a tulajdonos szépen szólónak hallotta.
A második optimalizálás helyi, 16 hangos Drive-költsége kb. 21/41/83%
48/96/192 kHz-en; ez nem a teljes engine CPU-terhelése. A numerikus
referencia-kontraktus és a hangminta kontrollja sikeres, a teljes motor,
kis buffer és magas-rátás minőségpolitika még következő kapu.
A szélesebb spektrális grid 148 érdemben mérhető kontrollja javult.
A kis-pufferes Drive-próbában 192 kHz-en néhány 32/64 mintás blokk túllépte
az audioidőt; a magas rátás költségpolicy ezért integrációs előfeltétel.
A #47 két macOS Release CPU-guardja a korábbi headen sikertelen volt,
funkcionális tesztbukás nélkül. Friss CI és kontrollált összevetés szükséges;
a küszöb nem módosul, piros guarddal nincs beolvasztás.
A production engine hangútja változatlan.

Részletes filterkövetelmények és első eredmények:
[prémium filter fejlesztése](PREMIUM_FILTER_DEVELOPMENT.md).
Az alábbi korábbi sorrend a feladatok technikai háttere; az aktív prioritást
most ez a szakasz határozza meg. A 32 voice nem kerül be a következő release
előfeltételei közé, a Linux továbbra is halasztott.

## Aktuális kiadási státusz

Az 1.0.4 2026-10-02-án megjelent a tulajdonos kifejezett publikálási döntése
alapján, és a GitHub Latest kiadása. A csomagok változatlan forrása `1a6a5f2`.
A publikálási ellenőrzések, a builddátum eltérése és a nem dokumentált teljes
friss natív mátrix korlátja a [kiadási jegyzőkönyvben](RELEASE_1.0.4_CHECKLIST.md)
szerepel. A további aktív munkát a kiadás utáni sorrend vezeti; új reprodukált
kritikus hiba ezt a sorrendet megelőzi.

Az alábbi kiadás előtti összegzés és mérföldkövek történeti tervként maradnak
meg; nem jelentik, hogy az 1.0.4 továbbra is draft.

## Kiadás előtti munkacsomag történeti állapota

Az 1.0.4 draft a #38 után ténylegesen frissült: forrása
`1a6a5f2fc13d9b85a4f0ad4afcb191e4f84ddf38`, mind a 8 asset ellenőrzött.
A PR 17/17 és a main 11/11 ellenőrzése sikeres. Nyilvános Latest továbbra is 1.0.3.

A publikálásig hátralévő lépések, a natív REAPER-mátrix, a blokkoló hibák,
a végleges csomagkör és a dátum/Latest egyeztetése egy helyen szerepel:
[terv a drafttól a publikálásig](RELEASE_1.0.4_CHECKLIST.md).
Ez a kiadási végrehajtás elsődleges terve; az alábbi további fejlesztési témák
csak igazolt kiadást blokkoló regresszió esetén válnak az 1.0.4 előfeltételévé.

## A publikálásig következő mérföldkövek

A 2026-10-02-i ellenőrzésben a helyi tesztsor 70/70, a main ASan/UBSan és
coverage tesztsora 69/69, a TSan tesztsora 5/5 sikeres. A Windows és macOS
build is zöld. Az átnézett új változtatásokban nem igazoltunk új kiadást blokkoló
regressziót. A vizsgálat hatóköre és korlátai a
[repository-ellenőrzési jelentésben](REPOSITORY_REVIEW_2026_10_02.md) szerepelnek.

| Mérföldkő | Feladat | Teljesülési feltétel |
|---|---|---|
| 1. Friss csomag elfogadása | A `1a6a5f2` draft Windows és macOS natív próbája, különösen az új MIDI/ARP és import útvonalakon. | Buildazonosítóval rögzített eredmény; a kiadási checklist N01–N16 eseteinek státusza ismert. |
| 2. Esetleges regresszió javítása | Csak reprodukált kiadási hiba javítása, célzott regresszióval. | Javított eset és meglévő tesztek sikeresek; GUI, DSP-karakter és kompatibilitás megmarad. Ha nincs hiba, ez a lépés nem igényel új kódot. |
| 3. Végleges kiadási tartalom | Tényleges publikálási dátum, changelog, release notes, README és kézikönyvek egyeztetése. | A végleges tag dokumentációja nem állítja tartósan, hogy az 1.0.4 még draft; verzió és dátum minden csomagban egyezik. |
| 4. Végleges build és ellenőrzés | Pontos PR-head és merge utáni main CI; draft újraépítése ugyanabból a végleges commitból. | Windows, macOS és Quality sikeres; 8 asset, checksumok, manifestek, licenszek és kézikönyvek ellenőrzöttek. |
| 5. Publikálás | Végleges csomag About/build és telepítési próba, majd a draft publikálása. | Tag, forrás és csomag eredete egyezik; nyilvános Latest és letöltések ellenőrizve. |
| 6. Kiadás utáni fejlesztés | A lent felsorolt P2 témák reprodukciója és profilozása, külön PR-ekben. | Minden változásnak saját követelménye és bizonyító tesztje van; új hangkarakter külön kiadási döntés. |

A korábbi RC felhasználói próbája elfogadott, de az új `1a6a5f2` csomag
elfogadása külön rögzítendő. A main utolsó #39 változása csak dokumentáció;
a jelenlegi draft és main közötti eltérés ezért nem új DSP-kódeltérés.
A végleges kiadási körben ettől függetlenül egyező commitból készüljön a tag
és minden csomag. A jelenlegi `2026.10.01.` előkészítési dátumot nem szabad
ellenőrzés nélkül a tényleges publikálás dátumaként kezelni.

## Elkészült munkacsomag: presetimport

Az importot a GUI közvetlenül hívja. A korábbi implementáció minden új fájlnál
újra listázta a könyvtárat, a duplikációkereséshez újraolvasta a preseteket,
és a mentés előtt ismét elvégezte a névütközés-ellenőrzést.

A javítás a közös `PresetMutationLock` alatt, egyetlen importhívás idejére
név- és tartalomindexet készít. A tartalomindex fokozatosan épül fel, és korai
egyezésnél megáll; a következő bemenet a rendezett keresést onnan folytatja.
Minden meglévő preset legfeljebb egyszer kerül beolvasásra egy batchben.
Hibás vagy üres bemenet önmagában nem olvastatja be a könyvtárat.
Sikeres import után az új név és hang is bekerül az indexbe. Sikertelen mentés
esetén a foglalás visszavonódik. A közönséges Save útvonal névellenőrzése megmarad.

Megőrzött követelmények:

- Unicode/case-insensitive névütközés; meglévő fájl nem íródik felül.
- Pontos, kanonikus Snapshot-egyezés; közel azonos hangok nem olvadnak össze.
- A korábbi könyvtárban az első rendezett egyező név kerül a jelentésbe.
- Hibás meglévő preset nem akadályozhatja a jó fájlok importját, de a neve foglalt.
- Azonos hangok külön néven csak a kifejezett másolatimporttal kerülnek be.
- A pluginpéldányok és folyamatok műveleteit a meglévő közös zár sorosítja.
- A fájl létrehozása továbbra is kizáró: külsőleg létrehozott cél sem írható felül.

Az index az import idején készített könyvtárpillanatképre épül. A zárat nem
használó külső program ne módosítsa közben a könyvtárat; egy következő importhívás
már új indexet készít. A GUI-művelet továbbra is szinkron, és a zárra várakozás
sem szűnik meg. A háttérmunka külön következő feladat.

### Helyi mérés

Windows, MSVC 19.44, Release; ugyanaz a mérőprogram és adatkészlet. A mérés az
`ImportPresets` hívást tartalmazza, a fixture-előkészítést nem. A mentés/flush
ideje benne van. Egy-egy helyi futás, nem platformfüggetlen teljesítményígéret.

| Meglévő + új preset | Korábbi main | Indexelt import |
|---|---:|---:|
| 100 + 100 | 2634,85 ms | 185,77 ms |
| 200 + 200 | 9859,56 ms | 377,46 ms |
| 400 + 400 | 37825,70 ms | 788,17 ms |

A manuális mérőprogram: `sawstar_preset_import_benchmark`. Egy parancssori
argumentuma a generált mérési alkönyvtárak szülőkönyvtára. A cél nem CTest:
közös CI-gépek sebességére nem állítunk merev időkorlátot. A funkcionális
`preset_import_index` teszt vegyes batch, pontos duplikáció, eltérő kis/nagybetűs
nevek, sérült fájlok, másolatimport és sikertelen mentés utáni folytatás esetét fedi.

## Presetbetöltési koherencia — követelmény a javítás előtt

A GUI presetalkalmazás jelenleg 93 külön paramétergesztust küld. A forrásból
látható a mezőnkénti átadás; ez önmagában nem bizonyít új hallható regressziót.
Teljes hangállapot átadását csak a következő szerződéssel szabad bevezetni:

1. Rögzíteni kell az editor presetváltás és közben érkező host-automatizálás
   sorrendjét: melyik esemény melyik blokkban érvényesül, és ki nyer ütközéskor.
2. A teljes motorparaméterkészlet és az ARP/editor reset ugyanahhoz az elfogadott
   állapothoz tartozzon. A host paraméterértesítése, Undo és GUI-visszajelzés megmarad.
3. A host-state visszatöltés, SerializeState, gyors egymás utáni presetek, zero-frame
   hívások és reset közbeni átadás is kapjon külön tesztet.
4. Az audio callback nem várhat GUI-mutexre, nem allokálhat és nem használhat
   korlátlan újrapróbálkozást. Egy audio-oldali snapshot önmagában nem rendezi
   a többi paraméteríróval való együttélést.
5. Determinisztikus interleaving-harness igazolja a részleges állapotot, majd a
   kiválasztott javítást. A meglévő állapotformátum és parameter ID-k megmaradnak.

## Kiadás utáni sorrend

1. Az importindex funkcionális regressziói és platform-CI elkészültek; a további
   teljesítményvizsgálat külön, kiadás utáni profilozás.
2. Import/mentés GUI-késésének profilozása; háttérmunka csak megmaradó késés esetén,
   megszakítással és az editor élettartamát tiszteletben tartó eredményátadással.
3. Presetkoherencia reprodukció és a fenti sorrendi szerződés.
4. Hard link nélküli átnevezés, kizáró célfoglalással és hibainjektált helyreállítással.
5. Backup/kedvencek helyreállítása, írási és zárolási hibák, többpéldányos próbák.
6. A hagyományos paraméterautomatizálás blokkfüggésének mérése. A #36 MIDI-pontjavítása
   nem vezette be a hagyományos paraméterek sample-accurate automatizálását.
7. Wrapper/motor profilozás; hangkarakter-változtatás csak külön mérés és döntés után.

### További nyitott kérdések és termékfejlesztés

- A wheel Publish/Clear reset-határhoz előbb egyértelmű eseménysorrendi szabály
  és reprodukció kell. A jelenlegi atomi átadás TSan-safe; az epoch hiánya
  önmagában nem bizonyított adatverseny.
- A további tesztmunka a valódi VST3/editor életciklus állapotkombinációira
  összpontosítson. A coverage százalék növelése önmagában nem elfogadási cél.
- A queue legrosszabb rendezési esete és a blokkonkénti teljes vezérlőátadás
  mérendő; audio-oldali optimalizálás csak igazolt költség alapján indokolt.
- A stabilitási feladatok után a prémium filter legyen az első hangminőségi
  munkacsomag: jelenlegi filter referencia, cutoff/rezonancia sweep, több
  mintavételi frekvencia, szintillesztett hallásos A/B és CPU-mérés. A régi
  presetek hangját nem szabad észrevétlenül megváltoztatni; kompatibilitási
  döntés szükséges az új karakter bevezetése előtt.
- A gyors presetlista ABC-sorrendje és kategóriák szerinti böngészése külön
  használhatósági feladat. Új cutoff-központú factory presetek csak a filter
  végleges viselkedésére készüljenek.
- A 32 voice előbb CPU- és voice-stealing mérés legyen, ne automatikus minőségi
  ígéret: a nagyobb polifónia önmagában nem vastagít egyetlen hangot.
- Linux plugin halasztott. A lezárt click/pop kutatás nem kerül vissza az aktív
  hibajavítási listára új bizonyíték és külön döntés nélkül.

Natív REAPER-mátrix: host/editor azonos hang, overflow, késői release, ARP HOLD,
sustain, stop/start, editor close, bypass, deaktiválás, reset, presetváltás és
offline tail. A korábbi 1.0.4 általános felhasználói elfogadás nem azonos az új
build minden kombinációjának külön dokumentált elfogadásával.

A következő release verzióját és RC-jét a tényleges kiadási kör rögzítse.
Minden PR pontos headje legyen zöld; publikálás előtt a végleges commit,
csomagok, natív elfogadás és dokumentáció kapui is teljesüljenek.
Filterkarakter, 32 voice és a lezárt click/pop kutatás külön munkacsomag marad.

