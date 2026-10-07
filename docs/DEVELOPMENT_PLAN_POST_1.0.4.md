# SAWSTAR — fejlesztés az 1.0.4 kiadás után

Frissítve: 2026-10-07. A kiadási összegzés történeti alapja: `2d3fd73527ecdad27028792d898b37b2d4bf28de`.
Az akkori main Windows, macOS és Code quality workflow-ja sikeres volt. A #36 PR lezárta
az ARP transport-stop utáni editor-tulajdonlás, a VST3 host MIDI-kontrollerpontok
és a jövőbeli editor-offsetek overflow-helyreállításának javítását.

## Rövid állapotkép — 2026-10-07

| Terület | Jelenlegi állapot | Következő kapu |
|---|---|---|
| Nyilvános kiadás | 1.0.4; a kiadott csomag forrása továbbra is `1a6a5f2` | A következő verzió külön rögzítendő |
| Kiadási ellenőrzés | #69: pontos platformú installer, duplikációvédelem, végleges assetlista | Következő kiadás csomagellenőrzése |
| Prémium filter | Négy módos kutatási prototípus; production hangút még változatlan | CPU/minőségpolicy és natív kis-bufferes mérés |
| Filter integráció | Még nem kész | Latency/state/automation, factory presetek, natív elfogadás |
| Presetátnevezés | #70 beolvadt; 17/17 ellenőrzés sikeres | exFAT és natív GUI elfogadás |
| Gyors presetlista | #71 beolvadt; ABC-sorrend, kategóriacímkék és azonos sorrendű nyilak; 17/17 ellenőrzés sikeres | Natív GUI elfogadás |
| Preset archiválása | #72 beolvadt; 17/17 ellenőrzés sikeres | Natív törlés/mentés és külső meghajtós elfogadás |
| Presetkezelés | #73 importprofil és #74 háttérimport beolvadt; a #74 friss headjének mind a 17 különböző CI-ellenőrzése sikeres | Natív import/editor/unload elfogadás; koherencia-reprodukció nyitott |
| 32 voice / Linux plugin | Halasztott | Következő release után |

Az átnevezés működése és külső meghajtós tesztje:
[preset rename validation](PRESET_RENAME_VALIDATION.md).
A gyors lista működése: [quick preset list](QUICK_PRESET_LIST.md).
A törléskor megtartott mentések védelme: [preset archive validation](PRESET_ARCHIVE_VALIDATION.md).
Az importmérés nyers eredményei és korlátai: [preset import profile](PRESET_IMPORT_PROFILE.md).
A háttérimport élettartama, működése és elfogadási kapui: [background preset import](PRESET_IMPORT_BACKGROUND.md).
Az alábbi történeti mérési eredmények nem helyettesítik a nyitott kapukat.

## Aktív prioritás 2026-10-04

A tulajdonos új sorrendje szerint a következő release fő fejlesztési iránya
**a prémium filter**. A többi felsorolt megbízhatósági és használhatósági téma
az aktív terv része marad. Csak a 32 voice vizsgálata és a Linux plugin kerül
biztosan a soron következő release utánra.

1. Filterreferencia és külön kutatási prototípus: cutoff/rezonancia mérés,
   hosszabb bevezetőjű hangminták, stabilitás több mintavételi frekvencián.
2. A jelölt hallásos értékelése, rezonancia és telítés tervezése; a nemlineáris
   részekhez anti-aliasing/túlmintavételezés és CPU-költség külön ellenőrzése.
3. Teljes filtercsere a tulajdonos október 4-i döntése szerint: prémium LP12,
   LP24, HP12 és BP12 a meglévő GUI-val. Nincs Classic/Premium választó.
   Régi preset/projekt értékei megmaradnak, de az új karakterrel szólnak.
   Parameter ID-k, automation és módértékek stabilak; a hallható változás
   dokumentált. A prototípus önmagában még nem kerül a plugin jelútjába.
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
funkcionális tesztbukás nélkül. A friss `0c3f4e2` head összes ellenőrzése
sikeres lett, a #47 beolvadt (`29a25ac`); a küszöb nem módosult.
A szimmetrikus FIR következő optimalizálása helyben kb. 17/34/66%-ra
csökkenti a 16 voice Drive-költséget 48/96/192 kHz-en. A 192 kHz-es
32 mintás próba még túllépte az időkeretet; teljes engine-profil és
magas rátás minőség/költség döntés szükséges az integráció előtt.
A #48 szimmetrikus FIR minden CI után beolvadt (`20fe01f`).
A külön kutatási teljes motor próba elkészült: 16 voice, 20 dB Drive és
FX mellett helyben kb. 30/60/120% medián audioidő 48/96/192 kHz-en.
A rezonancia ismételt együttható-számítását cache kiváltja, a korábban
elfogadott hangminta változatlan. 192 kHz-en így is túl nagy a költség.
Következő aktív kapu a magas rátás Drive minőség/költség megoldása,
abszolút hallható frekvenciákon a 4x referenciával összevetve; utána natív
host CPU-próba és a négy mód teljes cseréje következik a meglévő vezérlőkkel.
A #49 teljes motorpróba minden CI után beolvadt (`6eb4fc2`). A tulajdonos
az új filter teljes cseréjét választotta; a Classic/Premium külön modell
korábbi terve megszűnt. A mostani kutatási LP24 mellé LP12/HP12/BP12 kell,
és minden factory preset újrahallgatandó az integráció után.
A #50 teljes filtercsere-terv beolvadt (`65e3f4d`). Magas rátás kutatási
jelölt készült: 176,4 kHz alatt 4x, attól felfelé 2x Drive, azonos 32 mintás
késéssel. A 88,2 kHz-től 2x korábbi jelöltet a mérés elutasította.
A szűkített jelölt 192 kHz-en helyben kb. 78% teljes motor-mediánt mért,
de erős magas hangon több spektrális maradékot ad, mint a 4x referencia.
Ez nem elfogadott végleges policy: összetett gerjesztés, aliasing/moduláció
és natív stresszpróba kell. Utána a hiányzó LP12/HP12/BP12 fejlesztése
és a teljes csere integrációja következik; új GUI kezelőszerv nélkül.
A #51 magas rátás kutatás minden CI után beolvadt (`769993c`). A 24 rövid,
OSC1/OSC2/SUB unison és Drive-lépcsős kontroll egy 8x offline referenciával
is összeveti a jelöltet. Legnagyobb jelölt/8x eltérés kb. 0,39%, a vizsgált
LP24 után 0,16%; ez nem hallhatatlansági vagy teljes aliasing-bizonyíték.
A #52 mind a 17 ellenőrzése sikeres lett és beolvadt (`f0b2851`).
A korábbi magas szinuszos stresszteszt kompromisszuma megmarad. A négy mód
kutatási munkacsomagja elkészült: az offline filter és motoradapter már LP12,
LP24, HP12 és BP12 módot kezel. 360 komplex frekvenciaválasz-kontroll,
folyamatosan futó ágak közötti simított módváltás és minden móddal
Poly/Mono/Legato lifecycle regresszió készült. A célzott Release és ASan/UBSan
próbák sikeresek; a már elfogadott LP24 hangminta változatlan.
A #53 minden ellenőrzése sikeres lett, beolvadt (`8056431`). A tulajdonos
a három új mód 48 kHz-es, 50% rezonancia/20 dB Drive mintáját szépen szólónak
hallotta. A 96 összetett forráskontroll minden móddal teljesült, a korábbi
korlátok változtatása nélkül; HP12-ben a jelölt/8x eltérés közel 0,49%.
A teljes offline motor két változatán új envelope/LFO/wheel/pitch/sustain és
mód/Drive/mix-váltási kontroll sikeres, Poly/Mono/Legato módban.
A friss négy módos profil 16 voice/20 dB/FX mellett helyben kb. 30–33% mediánt
ad 48 kHz-en. 192 kHz-en a 4x út kb. 120–131%, a rátafüggő jelölt kb. 81–100%;
jelentős legrosszabb blokkkiugrások is voltak, a régi motoron is.
Ez rövid offline mérés, nem natív stresszteszt. A realtime CPU-kapu továbbra
is nyitott; a magas rátás minőségpolitika sem végleges.
Következő aktív feladat a CPU-költség és minőségpolitika megoldása; ezt követi
a latency/state/automation integráció, factory presetek és natív elfogadás.
A production engine hangútja egyelőre változatlan.

A #54 mind a 17 ellenőrzése sikeres lett és beolvadt (`b5e3625`). A következő
kutatási lépés változatlan hang mellett újrahasználja a beállt szűrőegyütthatókat:
3 923 984 sztereó frame és a 12 korábbi négy módos WAV bitazonos a referenciával.
Nyolc célzott Release és öt ASan/UBSan ellenőrzés sikeres. A külön szűrőprofil
állandó beállításnál kb. 11–13% kisebb időt, gyors modulációnál kb. 4% többletet
mutat helyben; ez nem teljes motoros gyorsulás. A CPU-kapu továbbra is nyitott.
A következő munka a domináns Drive/FIR-költség és a magas rátás minőségpolitika
vizsgálata, majd teljes motoros és natív terhelésmérés. Az integráció csak ezek
után következik; az elfogadott karaktert és a meglévő GUI-t megtartjuk.

A következő Drive/FIR kutatási optimalizálás az interpolációban csak a host
nemnulla mintáit tárolja, és folytonos fázistáblákból olvassa a változatlan
együtthatókat. A `715cd47` befagyasztott sparse FIR-éhez 998 400 sztereó frame
bitazonos 2x/4x, hat ráta, Drive-váltás, ring-wrap, clear, snap, másolás,
új Init és hibás bemenet mellett. A 12 scene/mód preview mind a 36 WAV-ja
bájtról bájtra változatlan. Kilenc célzott helyi Release teszt sikeres;
a szándékosan hibás fázisindexet az új regresszió elutasítja.
A külön páros Drive-mérés 16 voice/20 dB mellett kb. 7–8% kisebb időt mutat
2x, illetve 13% körüli csökkenést 4x esetén. Ez nem teljesmotor-gyorsulás
vagy natív teljesítményígéret. A CPU-kapu és a magas rátás minőségpolitika
továbbra is nyitott; teljes motoros és natív mérés következik az integráció előtt.

A #56 kompakt Drive-interpoláció minden ellenőrzés után main-ba került
(`c36f743`). Új offline kis-pufferes időeloszlás-mérő mód készült: három
motorút, 16 voice/20 dB/FX, négy mód, 48/96/192 kHz, 32/64/128 mintás
csoportok; 108 CSV-sor és 110 592 blokk. A helyi MSVC Release mérésben a
prémium utak 96/192 kHz-en továbbra is túllépik az audioidőt. A production
kontroll is túl lassú 192 kHz-en ezen a gépen; ez nem natív hostbizonyíték.
Négy lifecycle/modulációs regresszió sikeres. A nyers percentilisek és a mérés
korlátai a filterdokumentumban szerepelnek. Következő lépés költségbontás,
kontrollált célgépes ismétlés és natív próba; a CPU-kapu továbbra is nyitott.

A #58 teljesmotor-időeloszlás minden friss ellenőrzés után main-ba került
(`54ca373`). Új komponensprofil külön időzíti a Drive-ot, prémium szűrőt,
filteradaptert és a kutatási/production motorokat FX nélkül és FX-szel.
Két út, három ráta, négy mód és három ismétlés: 504 ellenőrzött sor.
A helyi 48 kHz-es 16 példányos 4x Drive kb. 39,8% audioidőt, a lineáris
szűrő kb. 3,4%-ot mért. Ezek külön kernelpróbák, nem összeadható motoros
CPU-részarányok. A költségbontás a Drive/FIR/nemlinearitás további vizsgálatát
indokolja; a teljes szintézisköltség, magas rátás minőségpolitika és natív
elfogadás is nyitott. Négy lifecycle/modulációs regresszió sikeres.
Részletek és minden nyers ismétlés a filterdokumentumban; a production DSP
és a GUI ebben a mérési munkacsomagban változatlan.

A #59 komponensprofil 17/17 sikeres ellenőrzés után beolvadt (`d4c7822`).
A következő Drive-optimalizálás fordításkor rögzíti a 2x/4x interpolációs
fázisokat és a decimátor kimenetágát, változatlan mintasorrenddel és tárolókkal.
1 996 800 sztereó frame bitazonos a sparse és a befagyasztott kompakt
referenciával; 36 preview WAV változatlan, kilenc helyi Release regresszió sikeres.
A hibás fázissorrendet negatív kontroll elutasítja. A 432 váltakozó páros
mérésben 16 voice/20 dB mellett helyben kb. 2–3% kisebb izolált Drive-idő;
ez nem teljesmotor-gyorsulás és nem zárja le a CPU-kaput. A nyers párok és
korlátok a filterdokumentumban. Következik a FIR és nemlinearitás szélesebb
profilozása, teljes motoros és kontrollált natív célgépes próba. Production
DSP, GUI, minőségpolitika és állapotformátum ebben a kutatási körben változatlan.

A #60 main (`3179223`) minden workflow-ja sikeres. A következő offline
Drive-költségkontroll telítés nélkül ugyanazokat a FIR-eket futtatja.
72 váltakozó sorrendű páros mérésben helyben a FIR-only út a teljes Drive
idejének kb. 52–56%-át használja; ez nem pontos, összeadható CPU-részarány
vagy elfogadható hangmód. Öt célzott Release és két ASan/UBSan teszt sikeres,
a normál út két referenciához továbbra is bitazonos. Következik a nemlinearitás
gyorsabb számításának külön numerikus/spektrális és CPU-kontrollja, majd
teljes motoros és natív próba. A production DSP továbbra is változatlan.

A #61 17/17 sikeres ellenőrzés után beolvadt (`262177f`). Külön skaláris
telítésjelölt készült, a normál Drive használata nélkül. A std::tanh
referenciától helyben legfeljebb 1,11e-16 abszolút eltérést mért; Release
és ASan/UBSan regresszió sikeres. A skaláris mikromérésben 20/24 dB
bemenetskálán kb. 28% kisebb idő, ami nem teljes Drive/motor gyorsulás.
Következik a jelölt külön túlmintavételezett Drive-, spektrális és CPU-próbája;
a normál hangút, CPU-kapu és minőségpolitika egyelőre változatlan.

A #62 beolvadt (`c53d025`). A telítésjelölt külön Drive- és teljes 4x
motor-probe mérföldköve elkészült: 72 Drive-eset és 12 Poly/FX motor-fixture
a rögzített eltérési korlátokon belül, helyben nulla float kimeneti eltérés.
Három célzott Release és két új ASan/UBSan regresszió sikeres. Páros helyi
mérésben kb. 7–8% Drive és 5–6% teljesmotor-időcsökkenés; ez nem natív
realtime elfogadás. A normál Drive nem vált át. Következő kapu a Windows/macOS
ismétlés, szélesebb moduláció/spektrum és kis-bufferes deadline-próba; csak
utána dönthető el a jelölt átvétele és a végleges engine-integráció.

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
  presetek hangja az elfogadott teljes filtercsere miatt változhat; a betöltési
  és automatizálási szerződés megmarad, a hangváltozás kiadási jegyzetbe kerül.
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

## Ellenőrzött audit-találatok — 2026-10-05

Forrás: a `SAWSTAR_FRESH_ANALYSIS_2026_10_05_Version2.md` jelentés
állításainak ellenőrzése a `715cd47f5e3d869bc55974450d8156bbbca7c9fd`
revision alapján. Az alábbiak nyitott védelmi, dokumentációs és későbbi
platformfeladatok; a jelentés nem adott reprodukált, a támogatott
Windows/macOS plugin működését érintő hibát. Ez nem hibamentességi bizonyíték.
A prémium filter aktív prioritása és a már rögzített natív QA-kapuk megmaradnak.

| Feladat | Ellenőrzött tény és besorolás | Következő lépés / elfogadási feltétel |
|---|---|---|
| Presetútvonal környezeti bemenetének ellenőrzése | `src/presets/UserPresets.h`: a `UserPresetFolder()` nem ellenőrzi külön a kapott szöveg ürességét. Üres HOME relatív macOS útvonalat képezhet; az üres APPDATA Windows API-viselkedése külön reprodukciót igényel. Védekezési hiány, nem igazolt általános presetmentési regresszió. | Hiányzó és üres HOME/APPDATA célzott vizsgálata; biztonságos, egyértelmű hibajelzés vagy dokumentált fallback. Ne jöjjön létre presetkönyvtár véletlenül a munkakönyvtárban. Normál és Unicode útvonal regressziója Windows/macOS alatt. |
| Kiadási ellenőrzések függetlenítése a Python assert-től | `scripts/publish-release.py`: hét assert maradt (11, 13, 30, 40, 49, 51, 54. sor a vizsgált revisionben), amelyeket a `python -O` elhagy. A `workflows_ready()` explicit kivételes CI-védelme ettől megmarad; nem igazolt teljes CI-megkerülés vagy hibás publikálás. | A szükséges ellenőrzések explicit kivételt kapjanak. Normál és optimalizált Python-futtatásban ugyanazok a hibás dátum/notes, lejárt artifact, installer-darabszám/név/verzió és hiányos platformkészlet esetek bukjanak; az explicit CI-védelem maradjon meg. |
| CMake-követelmények egyeztetése | `CMakeLists.txt`: minimum 3.21; `docs/BUILDING.md`: 3.25+. Dokumentációs eltérés, nem reprodukált buildhiba. A string(JSON) 3.19 óta létezik, ezért önmagában nem indokol 3.21-et. | Rögzíteni a tényleges minimum és az ajánlott/tesztelt verzió különbségét. Minimum ígérete esetén konfigurációs/build ellenőrzés azon a verzión; a dokumentáció és CMake követelménye legyen összhangban. |
| Linux presetkönyvtár | A nem Windows ág Linuxon is macOS szerkezetet választ. A Save létrehozza a könyvtárat, ezért annak kezdeti hiánya nem bizonyít mentési hibát. A Linux plugin továbbra is halasztott. | A Linux munkacsomag részeként XDG-konform útvonal és HOME fallback, hiányzó/üres/érvénytelen bemenetek és mentés/visszatöltés tesztje. Nem előfeltétele a következő Windows/macOS release-nek. |

Karbantarthatósági megjegyzés: az enum/FindParameter használata mellett a wrapperben
és az EngineControls leképezésében numerikus ID-k is maradtak (például
`GetParam(59)`, `71+row*3`). Ez önmagában nem bug; későbbi tisztítás csak
változatlan host ID-k, állapotkompatibilitás és meglévő leképezési regresszió mellett.

A TSan-bővítés a már felsorolt valódi editor/audio életciklus-esetekhez kapcsolódjon.
Az egyszálú overlapping-notes, mono/legato és voice-transition tesztek puszta
TSan-futtatása nem igazol szálak közötti állapotbiztonságot. Az os.name Windows-ág
és a dátum végi pont nem került új hibafeladatként a tervbe.

## Friss repository-felülvizsgálat — 2026-10-06

Vizsgált main: `c53d025a6f9ef215b0d11a47771c0fc98717de1c`.
Részletek: [REPOSITORY_REVIEW_2026_10_06.md](REPOSITORY_REVIEW_2026_10_06.md).

A korábbi audit két védelmi feladatának javítása elkészült: az üres környezeti
presetútvonal elutasítása és a kiadási ellenőrzések explicit kivételekre cserélése.
Új, konkrét kiadási hiba: két macOS `.dmg` teljesítette a darabszámfeltételt
hiányzó `.pkg` mellett. Most minden kötelező telepítőtípusból pontosan egy kell,
és a névnek a megfelelő verzióprefixszel kell kezdődnie. Normál és `-O`
Python-folyamatban futó regressziók készültek; a HOME eseteket macOS/Linux
lifecycle teszt ellenőrzi. A javítások elfogadását az új PR CI-je igazolja;
helyi futás a Windows parancsfuttató környezeti hibája miatt nem történt.

A kutatási filtermunka elkészült részei nem jelentik a production filtercsere
lezárását. A CPU/minőségpolicy, integráció, natív hostelfogadás, presetek,
kézikönyvek és következő kiadás továbbra is nyitott. A #63 kutatási PR a
vizsgált main revisionön még nyitott; munkáját ez a hibajavítás nem írja felül.


## Telítésjelölt szélesebb regressziója — 2026-10-06

A #63 külön Drive-jelöltjének próbája további 288 gerjesztési esetet kap:
2x/4x, hat ráta, három amplitúdó, magas koherens hang, chirp, determinisztikus
zaj és bipoláris impulzussor; állandó vagy mintánként változó Drive.
Mindkét csatorna teljes egyoldalas eltérésspektrumát vizsgálja, DC/Nyquist
végpontokkal és negatív kontrollokkal. Élő FIR-állapot másolása, hibás
csatorna izolációja, clear-csend és 32 mintás impulzuscsúcs is ellenőrzött.
Az elfogadást a friss összevont PR CI-je igazolja; új helyi CPU-mérés nem
készült. A normál út átváltása, kis-bufferes CPU-kapu és natív QA nyitott.


## Telítésjelölt páros kis-pufferes CPU-próbája — 2026-10-06

A #63 beolvadt (`8c0bb23`), mind a 17 PR-ellenőrzés és a main tíz
ellenőrzése sikeres. Új külön mérőprogram hasonlítja össze a std::tanh és
ResearchTanh 4x teljes motorját: 16 Poly voice, 20 dB, négy mód, FX,
48/96/192 kHz, 32/64/128 mintás csoportok és négy váltakozó pár.
Platformonként 288 összesítő sor és 73 728 nyers blokkidő, p50/p95/p99,
maximum és szigorúan 100% fölötti audioidőszámláló készül.

Külön Windows/macOS Release CI gyűjti a nyers adatot, compiler- és
forrásazonosítót, ellenőrzi a CSV teljességét, és 90 napos artifactot ment.
Az időzítés nem CI-küszöb, nem host-wrapper vagy natív realtime elfogadás.
A kis-pufferes jelöltadatok alapján kell értékelni a további CPU-munkát;
a normál kutatási/production hangút átváltása és natív QA továbbra is nyitott.


Első eredmény a `b0fbebf` mért forrásból:
Windows x64 medián blokkidő kb. 1–2%-kal nagyobb, macOS ARM64 kb. 4–5%-kal
kisebb a jelölttel. A p99 nem mindenütt javul; 4x/192 kHz-en mindkét
platformon minden mért blokk túllépte az audioidőt. A két összesítő CSV és
eredetjegyzék tartósan rögzített; a nyers blokkok CI-artifactokban vannak.
Ez egy megosztott runneres kör, nem natív vagy platformfüggetlen CPU-bizonyíték.
A normál út átváltása nem indokolt. Következik a nagyobb FIR/teljesmotor-költség
optimalizálása és célgépes ismétlés; részletek a filterdokumentum új táblájában.


### 2026-10-06: SIMD FIR külön minősítése

- [x] Opt-in SSE2/NEON kutatási jelölt, változatlan együtthatók/telítés/állapot, explicit skaláris fallback.
- [x] 2x/4x bitazonossági kontroll és külön teljesmotor-fixture: Windows/macOS Release alatt 998 400 sztereó frame bitazonos, a 12 motorfixture eltérése nulla.
- [x] Páros Windows/macOS CPU-adatsor és tartós összesítő/provenance. Az első p50-arány Windows 0,932–0,938, macOS 0,886–0,933; néhány tail/túllépési eset romlott.
- [ ] Célgépes deadline/minőség QA és magasrátás policy lezárása. Fix 4x magas rátákon a kutatási motor többnyire továbbra is túllépi az audioidőt.
- [ ] Éles integráció/preset elfogadás. A SIMD opció alapértéke false; production rate-policy változatlan. Részletek és korlátok a `PREMIUM_FILTER_DEVELOPMENT.md` utolsó szakaszában.


### 2026-10-06: rátafüggő SIMD továbblépés

- A meglévő, Init-on választó kutatási 4x/2x út külön SIMD-jelöltet kap; scalar alapérték és 176,4 kHz-es normalizált határ megmarad.
- Új határ/hibásráta/állapot kontroll: 66 447 frame scalar és SIMD egyezése explicit fix-faktor oracle-lel.
- Új páros motoradatok 48/96 kHz 4x és 192 kHz 2x mellett; a fordított faktorok külön ellenőrzött metadata-ban szerepelnek.
- Az első Windows/macOS próba sikeres: a 66 447 routing frame bitazonos, a 12 motorfixture eltérése nulla. A faktorrekord ellenőrzött.
- 48/96 kHz-en a p50 csökken, de a 192 kHz/2x Windows-nyereség csak 0,1–0,3%, macOS 1–4% körüli; tail/túllépési nyereség nem általános. CSV/provenance és korlátok tartósan feljegyezve.
- A végleges PR CI-je rögzíti az elfogadást. Shipping rate-policy, célgépes teljesmotor-CPU/minőség QA és production filtercsere nyitott.

- Kódazonos ismétlés (`cd4c591`): Windows 192 kHz/2x p50 arány 1,028–1,029, kb. 3%-os lassulási jel; macOS 0,949–0,956. Mindkét bitazonossági fixture ismét sikeres. Az ismétlés CSV/provenance és a negatív CPU-találat a filterdokumentumban is szerepel; default aktiválás nincs.

### 2026-10-06: külön Drive CPU-diagnosztika

- Külön 24 páros, fix 2x/4x Drive-mérés a rátafüggő Windows/macOS CI-ben; ellenőrzött CSV és forrásazonosítós artifact.
- A SIMD tesztforrások hiányzó workflow path-triggerének javítása.
- A mérés a teljes Drive költségét izolálja; a Windows teljesmotoros 2x lassulás gyökérokának megállapítása, célgépes ismétlés és shipping policy továbbra is nyitott.

- Első külön Drive-kör: Windows 2x arány 0,935–0,943, 4x 0,906–0,907; macOS 2x 0,919–0,937, 4x 0,787–0,844. CSV/provenance tartósan rögzítve.
- Ugyanazon Windows job 192 kHz-es motorideje semleges (~1,000); a korábbi 3%-os lassulás most nem ismétlődött. macOS motor p50 javul, de néhány tail/túllépési eset romlik. Gyökérok vagy shipping CPU-elfogadás ebből nem állapítható meg.

- Kódazonos ismétlés: macOS külön 192 kHz/2x Drive arány 1,072197 (~7% lassulás), szemben az első 0,919041 értékkel. A negatív eredmény CSV/provenance-nal dokumentálva. Windows külön Drive továbbra gyorsabb, teljesmotoros 192 kHz nyereség csekély (~0,997); stabil/célgépes CPU-garancia nincs.


### 2026-10-07: a 2× Drive decimátorpufferének csökkentése

- A kutatási 2× ág 65 FIR-együtthatójához 128 helyes, tükrözött gyűrű elegendő; a korábbi 256 hely helyett pontosan 4096 bájttal kisebb a sztereó Drive-objektum. A 4× ág 256 helyes gyűrűje változatlan.
- A korábbi, befagyasztott scalar referenciákhoz 1 996 800 sztereó frame bitazonos a helyi MSVC x64 Release-próbában. A méret és a 32 mintás késés fordítási regresszióval védett.
- A #75 beolvadt (`1e99508`), mind a 17 különböző PR-ellenőrzés sikeres. A memóriaelőny nem CPU- vagy natív realtime-elfogadás; a shipping ráta-policy és production filtercsere továbbra is nyitott.
- Részletek: [2× decimátorpuffer](PREMIUM_DRIVE_RING_STORAGE.md).

### 2026-10-07: pufferkapacitásra elkülönített CPU-mérés

- Külön, közvetlenül a #75 előtti forrásból befagyasztott referencia; scalar/scalar és SIMD/SIMD összevetés, változatlan 4× kontrollal. Csak a pufferkapacitás változik.
- A helyi MSVC x64 Release-próbában 513 048 sztereó frame bitazonos; 96 páros mérési sor és 18 elutasító riportkontroll sikeres. Az új célpont DaisySP nélkül is fordítható.
- Az első helyi CPU-arányok nem mutatnak következetes gyorsulást; a lassabb scalar 96 kHz és SSE2 48 kHz eset is dokumentált. CSV és byte-hash eredetjegyzék tartósan rögzített.
- Külön Windows/macOS Release CI-mérés és ismétlés következik. A memóriaelőnyből nem következik CPU-kapu, végleges ráta-policy vagy production integráció elfogadása.
- Részletek: [pufferkapacitás CPU-vizsgálata](PREMIUM_DRIVE_RING_CPU_STUDY.md).

### 2026-10-07: következő filtermérföldkövek

| Mérföldkő | Állapot és bizonyíték | Következő kapu |
| --- | --- | --- |
| Pufferkapacitás CPU-vizsgálata | #76 beolvadt (`bfd011f`); 25/25 ellenőrzés sikeres. A Windows/macOS párok és provenance tartósan rögzítettek; a macOS kontroll erősen szór. | Nincs általános CPU-nyereségből következő aktiválás. |
| Drive-normalizálási jelölt | Elkészült az alapból kikapcsolt reciprok opció. Helyben 1 831 104 sztereó frame egyezett, per-ráta/per-scene numerikus kapukkal; 11 célzott teszt sikeres. | #77 CI sikeres; a friss teljesmotor-eredmények nem zárják a CPU-kaput. |
| Jelölt teljesmotor-CPU-ja | Külön SIMD/4× motoradapter és 16 hangos, négy módos, FX-es kis-bufferes fixture/mérőcélpont kész. A helyi külön Drive SIMD-ideje kb. 4–10%-kal kisebb; ez nem motoreredmény. | Új CI-adatok, ismétlés és célgépes/natív REAPER deadline-próba. |
| Végleges minőség/rátapolicy | Továbbra nyitott. A reciprok opció alapértéke false; a szállított hangút változatlan. | Magas rátás minőség, teljesmotor-CPU és natív elfogadás együtt. |
| Production filtercsere | A policy után következik a meglévő négy móddal és GUI-val. | Latency/state/automation, factory presetek és kompatibilitás. |
| Következő kiadás | A production integráció és natív elfogadás után. | Kézikönyvek, végleges verzió/RC, Windows/macOS csomagkapuk. |

A normalizálás numerikus határa kutatási minősítés, nem általános bitazonossági,
hallhatatlansági vagy realtime garancia. Részletek és a megtartott negatív/korlátozó
eredmények: [normalizálási jelölt](PREMIUM_GAIN_NORMALIZATION.md).


### 2026-10-07: a #77 utáni munkacsomag előzménye

A #77 Windows/macOS és sanitizer ellenőrzései sikeresek, a main `1c3fefb`
mind a tíz ellenőrzése zöld. Helyben 98/98 Release teszt és az öt új
preset/DSP-terület ASan/UBSan tesztje sikeres. A friss fix-4x motorpróba
kb. 1–2% medián időnyereséget ad, de 192 kHz-en mindkét platformon minden
mért blokk túllépi az audioidőt. Windows 96 kHz-en több a deadline-túllépés;
a kisebb medián nem elegendő elfogadási feltétel.

1. A normalizálási jelölt külön rátafüggő SIMD/4x–2x próbája, explicit
   routing/numerikus regresszió és Windows/macOS páros teljesmotor-mérés.
   A meglévő 176,4 kHz-es határ és 32 mintás késés megmarad; default aktiválás nincs.
2. Az eredmények alapján célgépes ismétlés és a magasrátás minőség/költségpolicy
   lezárása. A CI mérési idők leíró adatok; a funkcionális tesztek sikerét
   nem tekintjük realtime CPU-elfogadásnak.
3. Natív REAPER-elfogadás az új háttérimporttal: lejátszás/import, editor
   bezárás/újranyitás, plugineltávolítás, több példány és külső meghajtó.
4. Elfogadott policy után production filtercsere, latency/state/automation
   és factory presetek. Kézikönyvek, RC és kiadás csak ezután.

A teljesmotor- és külön Drive-eredmények, a negatív kontrollok és a mérési
forrásazonosítók a [normalizálási dokumentumban](PREMIUM_GAIN_NORMALIZATION.md)
szerepelnek. A presetkoherencia és a hagyományos automatizálás blokkfüggése
nyitott; Linux/32 voice halasztott, a lezárt click/pop kutatás nem újranyitott.


Az első új rátafüggő normalizálási jelölt helyi ellenőrzése: 101/101 Release,
3/3 új ASan/UBSan kontroll sikeres. A Mac mini M1 célgépes offline próba
kb. 1,2% medián javulást ad; 192 kHz-en 81,85% helyett 80,86% audioidő-medián,
de 175/12 288 blokk még túllépi az időkeretet. A 48 kHz-es túllépésszám
2-ről 4-re nőtt. Ez nem natív hostelfogadás; a friss Windows/macOS CI-mérés,
kódazonos ismétlés és realtime/minőség kapu továbbra is nyitott.

### 2026-10-07: #78 után, aktuális sorrend

A #78 mind a 31 ellenőrzése sikeres; beolvadt a mainbe (`cafae21`), ahol
a Code quality és a Windows/macOS build is zöld. A rátafüggő SIMD/reciprok
jelölt Windows/macOS mérése elkészült és tartósan rögzített. A mediánidő
kb. 0,9–2,1%-kal kisebb, de Windows 192 kHz-en továbbra is 12 288/12 288
blokk túllépi az időkeretet. macOS 192 kHz-en a jelölttel több lett a
túllépés (631 helyett 798). A CPU-kapu továbbra is nyitott.

Az új kombinált minőségi regresszió 176,4/192/384 kHz-en, 144 OSC1/OSC2/SUB
kontrollban ellenőrzi mind a négy szűrőmódot, 20/24 dB és változó Drive mellett.
A változatlan 8x referenciakorlátok teljesülnek; helyben 102/102 Release és
2/2 célzott ASan/UBSan teszt sikeres. A legnagyobb szűrt referenciaeltérés
0,490212% (192 kHz, high-lead, HP12, 24 dB), közel a 0,5%-os diagnosztikai
korláthoz. Ez nem hallásos vagy aliasing-elfogadás, és nem production aktiválás.

1. Az új minőségi regresszió Windows/macOS és sanitizer CI-ellenőrzése;
   beolvasztás csak a teljes, pontos headhez tartozó zöld ellenőrzéssor után.
2. Kódazonos célgépes ismétlés, kis-bufferes teljesmotor-profil és a magasrátás
   költség/minőségpolicy lezárása. A normalizálási gyorsítás önmagában kevés;
   a rosszabb deadline-eredményeket és a minőségi kompromisszumot megtartjuk.
3. Natív Windows/macOS REAPER-elfogadás az elfogadott filterpolicy és a
   háttérimport mellett: lejátszás/import, editor bezárás/újranyitás,
   plugineltávolítás, több példány, külső meghajtó.
4. A kapuk után a prémium filter váltsa le teljesen a Classicot a meglévő
   GUI-val és négy móddal; latency/state/automation, presetkoherencia,
   factory presetek és régi projektek ellenőrzése külön feladat.
5. Kézikönyvek, verzió/RC és Windows/macOS csomagellenőrzés, majd kiadás.

A részletes mérési eredmények és forrásazonosítók:
[Drive-normalizálás és kombinált policy](PREMIUM_GAIN_NORMALIZATION.md).
Linux/32 voice továbbra is halasztott; a lezárt click/pop kutatás nem nyílik újra.

### 2026-10-07: #79 után, telítési CPU-jelölt

A #79 mind a 31 PR-ellenőrzése sikeres és mainbe került (`8b03970`).
A kombinált rátafüggő jelölt komplexforrás-kontrollja így Windows/macOS
és sanitizer CI-n is megfelelt. A main új Windows/macOS buildje és
Code quality futása is sikeres.

A következő, alapból kikapcsolt kutatási jelölt a tanh kiértékelését gyorsítja
közös, változatlan Hermite-együttható-táblával. Nem csökkenti a FIR hosszát,
nem változtat faktort vagy gain-simítást. A skaláris próba és a 360-case Drive
waveform/spektrum-kontroll sikeres, a korábbi Drive-korlátok változatlanok.
Helyben 105/105 Release és 3/3 új ASan/UBSan kontroll sikeres. Az első M1
páros mérésben kb. 10% (4x) / 18% (2x) az izolált Drive időnyeresége;
ez még nem a teljes motor vagy a natív host eredménye.

1. A lookup-jelölt pontos headjének Windows/macOS, ASan/UBSan és TSan kapui;
   a közös tábla párhuzamos első inicializálása is tesztelt. Beolvasztás csak
   minden aktuális ellenőrzés befejezése és sikere után.
2. Külön rátafüggő, SIMD/reciprok lookup motorpróba a std::tanh változathoz
   képest; szóló/modulációs/numerikus és 8x referencia-kontroll, majd
   kis-bufferes páros blokkidő és célgépes ismétlés. A rosszabb eredmény is maradjon.
3. A teljesmotor-adatok alapján a magasrátás költség/minőségpolicy lezárása,
   majd natív Windows/macOS REAPER-elfogadás és háttérimport-lifecycle próba.
4. Elfogadás után production filtercsere a meglévő GUI-val és négy móddal;
   latency/state/automation, presetkoherencia, factory presetek, régi projektek.
5. Kézikönyvek, verzió/RC, csomagkapuk és kiadás.

Részletek, határok és megőrzött mérési adatok:
[lookup telítési jelölt](PREMIUM_LOOKUP_SATURATION.md).
Ez a munkacsomag nem zárja le a CPU/minőség kaput; Linux/32 voice halasztott,
a click/pop kutatás lezárt marad.
