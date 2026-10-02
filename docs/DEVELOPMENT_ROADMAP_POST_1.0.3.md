# SAWSTAR — összevont fejlesztési terv a v1.0.3 után

**Aktuális terv:** [Fejlesztés az 1.0.4 után](DEVELOPMENT_PLAN_POST_1.0.4.md),
2026-10-02; #36 mainben. Az alábbi dátumozott összegzések a korábbi vizsgálatok
és az 1.0.4 előkészítésének történeti állapotát őrzik.

**Frissítve:** 2026-10-01
**Kiinduló kiadás:** v1.0.3, tag `57dbee9`  
**Auditált főág:** `4d80ba948415327f18c6a138524e649dea509816`  
**Legutóbb ellenőrzött main:** `45f11dc` (#32; macOS, Windows és Code quality sikeres)
**Következő kiadás:** v1.0.4; a tulajdonos 2026-10-01-én engedélyezte az előkészítést és publikálást a kiadási kapuk teljesítése után. Részletes sorrend: [1.0.4 checklist](RELEASE_1.0.4_CHECKLIST.md).
**Cél:** a megbízhatósági hibák rendezett javítása, majd mérésalapú motor- és termékfejlesztés; a következő kiadás előtt teljes ellenőrzéssel.

Ez a dokumentum egyesíti a v1.0.3 utáni fejlesztési irányt és a 2026-09-26-i mélyaudit hasznos, aktuális megállapításait. A vizsgált audit főága hat commitnyira van a kiadott tagtől; a PR #22 már beolvadt. A 57/57 CTest és a coverage-adatok az audit jelentésében szereplő főági eredmények, nem e dokumentum önálló újrafuttatásai.

## 2026-10-01 — aktuális összegzés és következő feladat

- #27–#30 mainben: reset előtti upstream editor FIFO ürítés; saját editorhang nélküli késői felengedés elnyelése; keyboard és aktív PITCH-gesztus befejezése editorbezáráskor; zero-frame overflow-jelzés megőrzése; bypass alatti MIDI/DSP továbbfuttatása némított hostkimenettel. MOD továbbra is latching.
- A #30 Windows-tesztjének túl nagy veremfoglalását külön javítottuk; a végső PR és a `0bc9799` main minden CI-workflow-ja sikeres. A natív REAPER close/reset/bypass/deactivation elfogadás továbbra is kiadási kapu.
- #31 mainben (`2f724c0`): a korábbi nulla VST3 tail-bejelentés helyett 380 másodperces, véges maximum jelenik meg a konstruktorban és minden mintavételifrekvencia-resetnél. A korábbi FX-előzmény miatt nem rövidül le rögtön presetváltáskor. A PR 17/17 ellenőrzése és a main három workflow-ja sikeres; natív offline render elfogadás még szükséges. Részletek és korlátok: [tail-szerződés](VST3_TAIL_CONTRACT.md).
- A tail regresszió a valódi envelope/chorus/delay/reverb láncot és a teljes 16-voice motort ellenőrzi. A teljes lecsengés Release, sanitizer és coverage alatt fut; Debugban célzott, rövid motorpróba és teljes burkolólecsengéses FX-próba marad. A hangjel feldolgozása nem változott.
- #32 mainben: a törtszámos enum/egész presetértékek egységesítése a tényleges iPlug2 regisztrációval. A régi kód `VoiceMode=1.5` értéket mentett, a plugin 2-t alkalmazott, ezért a frissen betöltött preset azonnal módosítottnak látszhatott. A javítás csak a valóban diszkrét értékeket kerekíti, negatív félértéknél is a framework szabályával; a folytonos vezérlők pontossága megmarad. Azonosítók és wire format nem változnak. A main három workflow-ja sikeres.
- #33 mainben (`645a994`): a strict presetfájl-dekóder elutasítja a szöveg/headerless, üres, unknown-only és host-traileres fájlokat. A régi verziózott részleges presetek és ismert mellett jövőbeli rekordok megmaradnak; a legacy host-state dekóder változatlan. Régi betöltőn bukó regresszió, helyi Release 67/67, célzott ASan/UBSan 7/7 és PR CI 17/17 sikeres.
- Az 1.0.4-rc1 metaadatok, kiadási jegyzetek és a 14 oldalas EN/HU kézikönyvek elkészültek a külön kiadási ágon; a natív hostelfogadás és publikálás még hátravan.
- A wheel mailbox Publish/Clear átfedése memória-szinten biztonságos; az epoch bevezetése külön reset-policy döntést és reprodukciót igényel. Ez jelenleg specifikációs nyitott kérdés, nem igazolt új adatverseny. A strict presetimport már elkülönült a kompatibilis host-state dekódertől külön; presetváltási koherencia és automatizálás blokkfüggése következik.
- Profilozás, további fájlrendszer-policy, filterminőség, bővíthető presetbrowser és 32 voice későbbi, külön munkacsomag. Az 1.0.4 stabilitási kiadás; Linux plugin továbbra is elhalasztva.

## 2026-09-29 — fejlesztési állapot

- PR #23 beolvadt: `6ec866f`. A MIDI recovery utáni késői editor Note Off és a nagy mintavételi frekvencián megakadó ADSR javítása a main része. A merge commit macOS, Windows és Code quality Actions futása sikeres.
- A következő ág a fejlesztői build egyértelmű azonosítását javítja: `release.json` candidate=`dev`, üres release_date. Az About és a fejléc `1.0.3 dev` / `Build` jelölést kap; az installer fájlnév `-dev` végződést. A numerikus hostverzió 1.0.3 marad, a következő kiadás száma nincs eldöntve.
- A CMake buildmeta hét fejlesztői/RC/kiadási és hibás-metaadat esetben ellenőrzött; a release guard külön teszteli a `dev` publikálás kihagyását. A platformos plugin buildet az új PR Actions ellenőrzése végzi.
- A vezérlő-overflow és close/reset lifecycle nyitott: a keretsorban elveszett CC1/pitch bend utolsó értékét a jelenlegi note-only overflow callback nem állítja helyre. Olyan átadási megoldás kell, amely a későbbi hostüzenetet sem írja felül egy régi editorértékkel. A close/reset esetekhez valós hostreprodukció szükséges.
- A VST3 tail jelentés és a további P2 feladatok nyitva maradnak. A lezárt click/pop kutatást ez a munka nem nyitja újra.

## 2026-09-30 — controller overflow részjavítás

- PR #24 beolvadt (`1be4986`): a fejlesztői verziójelölés a main része.
- A SAWSTAR `BlockMidiQueue` saját audiószálas sorában reprodukáltuk az elvesző végső pitch bend/CC1-visszaállítást. A régi kód a regresszión elbukik; a javítás megtartja a csatornánkénti utolsó bend, CC1 és CC121 eseményt, időponttal és forrásjelöléssel.
- Túlcsorduláskor a bizonytalan hangüzenetekre megmarad a meglévő panic. Legfeljebb 48 vezérlőesemény kerül vissza a sorba, eredeti időzítéssel. Azonos MIDI-időpontnál az érkezési sorrend dönt, különböző időpontnál a későbbi esemény; így régebbi editorérték nem írja felül a későbbi hostértéket.
- Ez kivételes helyreállítás: a köztes vezérlőmozgásokat összevonja. Normál, nem túlcsorduló feldolgozás változatlan. A tesztek a visszaállítást, a 16 csatornát, CC121 sorrendet, jövőbeli offsetet, forrásjelölést és explicit Clear-t lefedik.
- Az iPlug2 upstream editor FIFO külön hibaforrás; ennek GUI-kerék javítását a következő szakaszban leírt PR #26 kezeli. REAPER hostelfogadás hátravan.

## 2026-09-30 — editor kerékátadás

- PR #24 már mainben (`1be4986`); PR #25 beolvadt (`8e7562d`): a saját audiószálas MIDI-sor controller-helyreállítása a main része.
- Az új editor-kerék javítás a GUI offset=0 pitch bend/CC1 eseményeit az iPlug2 sor előtt egy fix méretű atomi átadóba irányítja. A legutolsó érték a következő nem üres blokk elején egyszer kerül alkalmazásra. A sor telítődése így nem dobhatja el a GUI kerék végső visszaállítását.
- Dokumentált együttélési szabály: a blokk host MIDI-eseményei ezután, saját offsetjüknél futnak, tehát sample=0 ütközésnél a host nyer. A több GUI-mozdulat blokkhatárok között összevonódik; a host automatizálása nem.
- Az editor note/sustain és más, illetve nem nulla offsetű üzenetek továbbra is a FIFO-n mennek. Ez a GUI-kerekek konkrét hibájának megoldása, nem minden upstream MIDI-vesztés általános kezelése.
- Célzott teszt ellenőrzi a 100000-es burstöt, 16 csatornát, közép/nulla visszaállítást, host-sorrendet, resetet és párhuzamos producer/consumer működést; a ThreadSanitizer CI-be is bekerül. Platform build és kézi REAPER-elfogadás szükséges.
- Következő nyitott tételek: close/deactivation/reset hostreprodukció; VST3 tail; preset/fájlrendszer és automatizálás P2 audit. A kattogáskutatás lezárt marad.

## 2026-09-30 — reset előtti editor-események

- PR #25 és #26 mainben (`dce55cc`), mindhárom főági workflow sikeres.
- A reset eddig a saját blokk-MIDI-sort ürítette, az iPlug2 upstream editor FIFO-ját nem. Így a reset előtt várakozó editor-esemény újra bekerülhetett a frissen visszaállított motorba.
- A javítás a reset elején a consumer oldalon eldobja az upstream sor pillanatnyi elemszámának megfelelő régi üzeneteket. Nincs korlátlan drain; a snapshot után érkező üzenetek megmaradnak. A resetet a hostnak az audio-feldolgozással sorosítva kell hívnia, ahogyan a meglévő motorreset is megköveteli.
- A production patchből kinyert metódus regressziója üres/teli sort, új bemenetet és drain közbeni producer-hozzáadást ellenőriz; a no-op negatív kontroll elbukik. Az elérhető iPlug2 valódi SPSC sorával is külön teszt fut. A korábbi framework patch helyben frissíthető.
- Ez nem teljes lifecycle-javítás: GUI close/deactivation, resetet követő késői fizikai Note Off és valós REAPER-elfogadás továbbra is külön ellenőrzendő. A VST3 tail feladat szintén nyitott.

## Fejlesztési alapelvek

- A kiadott v1.0.3 viselkedése az alap. Paraméterazonosítók, régi presetek, projekt-visszatöltés és hangkarakter csak célzott teszttel és indokolt változtatással módosulhat.
- A korábbi kattogás/pukkanás vizsgálat lezárt kutatási szál. Nem folytatunk belőle találomra filter-, crossfade-, hangindítási vagy burkolóváltoztatást. A vizsgálat alatt talált, külön igazolt hibákat viszont a saját tesztjeikkel kezeljük.
- Előbb reprodukció és regressziós teszt, aztán a lehető legkisebb javítás. Minden változtatás után az érintett host- és motorviselkedés újraellenőrzendő.
- A GUI elrendezése és méretei maradjanak változatlanok. A fejlesztés elsődlegesen stabilitási, DSP- és könyvtárkezelési munka.
- A szűrő fejlesztése kiemelt hangminőségi cél, de külön munkacsomag: előbb mérés és szintillesztett A/B, csak utána karaktert módosító implementáció.
- Linux plugin kiadása elhalasztva; nem része a következő release kapuinak.

## Már lezárt vagy külön kezelendő témák

- **v1.0.3 kiadás:** megjelent; újabb kiadás csak a lenti kapuk teljesítése után.
- **Editor MIDI túlcsordulás alapjavítása:** a korábbi recovery- és sorrendjavítások a főágba kerültek. Az új audit egy maradék forrástulajdonlási kombinációt és további kontroll-esemény réseket talált; ezeket kell vizsgálni, nem a régi megoldást általánosan lecserélni.
- **Kattogás/pukkanás kutatás:** lezárva mint külön hibakeresési program. A jelenlegi roadmap nem állítja, hogy a jelenség megszűnt, és az abból származó, hangkaraktert módosító kísérleteket nem emeli át automatikusan.
- **Mono glide, LFO eseménysorrend és filterállapot hipotézisek:** csak akkor nyitandók újra, ha új, ismételhető bizonyíték és külön követelmény áll rendelkezésre.
- **Külső audit állításai:** az alábbi fázisokban P1-ként szereplő itemspecifikus regresszióval ellenőrzendő; a teljesítmény-, hangminőség- és fájlrendszerészrevételek nem mind igazolt hibák.

## Mérföldkövek és sorrend

### 0. Baseline és munkafolyamat — kész / minden PR előtt ellenőrizendő

Rögzítsük a cél commitot és a kiadott v1.0.3 tagot; minden javítás külön PR-ben, változatlan release assetek mellett történjen. A CI (macOS/Windows/Linux minőségellenőrzés, sanitizer, VST3 validator, csomagoló és installer próbák) kötelező kapu marad. A CI zöld állapota nem helyettesíti a valós hostban végzett ellenőrzést.

**Elfogadás:** pontos commitra zöld ellenőrzések; nincs kiadási vagy plugin-telepítési változtatás felhasználói jóváhagyás nélkül.

### 1. P1 — MIDI-forrástulajdonlás és túlcsordulási helyreállítás

**Státusz:** célzott kódjavítások mainben (#23, #28, #30); kombinált natív hostelfogadás még hátravan. Az alábbi leírás a korábbi audit indoklását és az elfogadási követelményeket őrzi.

**Indok:** a motor hangszámlálója jelenleg csatorna+hang szerint egyesítheti a host és editor lenyomásait. Recovery után egy késői valódi editor Note Off olyan számlálót csökkenthet, amelyben host által tartott hang is szerepel.

**Munka:**

1. Készítsünk determinisztikus tesztet: host és GUI ugyanazt a csatorna/hang kombinációt tartja; editor overflow; szintetikus felengedés; ezután fizikai editor Note Off. Ellenőrizzük, hogy a host hangja tovább szól.
2. Fedjük le az ellenkező sorrendet (valódi Note Off recovery előtt), ismételt hangokat, új leütést recovery közben, sustain, csatornaváltást, Poly/Mono/Legato módokat és 16 hangon túli helyzeteket.
3. A javítást tulajdonosi állapotra építsük: az editor és host hang-életciklusa ne ugyanazt a számlálót kezelje úgy, hogy egymás felengedését kiválthassák. Ne használjunk globális panicet normál helyreállításként.
4. Tartsuk meg a meglévő overflow recovery korlátait és az audiószálon végzett, előre lefoglalt feldolgozást.

**Elfogadás:** a hosthang egyetlen sorrendben sem némul el az editor recovery miatt; a késői/dupla Note Off nem hagy beragadt hangot; meglévő MIDI-regressziók és sanitizer futások zöldek.

### 2. P1 — Editor vezérlőesemények és ablak/reset életciklusa

**Státusz:** #25–#30 célzott kódjavításai mainben; valós close/reset/bypass/deactivation próbák szükségesek a teljes lezáráshoz.

**Indok:** az editor MIDI-sor túlcsordulása CC1/pitch bend eseményeket is elveszíthet; emellett a bezárás, reset vagy deaktiválás idején függő hang- és eseményállapotokra nincs minden útvonalban bizonyított cleanup.

**Munka:**

1. Teszteljük a CC1 és pitch bend overflowját, különösen az utolsó érték vagy visszaállító esemény elvesztését.
2. Határozzuk meg a host és editor azonos MIDI-csatornáján használt vezérlők tulajdonosi szabályát. Ne írjuk vissza vakon a GUI kerekeinek értékét úgy, hogy az felülírja a host automatizálását.
3. Vizsgáljuk meg valós hostban a GUI ablak bezárását, host deaktiválást, audio resetet, projektváltást, valamint az editor sorában már várakozó eseményeket. Teszteljük az iPlug2 keretsor és a SAWSTAR saját sorának külön ürítését.
4. Ha reprodukálható, adjunk explicit, forráshelyes editor cleanupot és definiáljuk a reset/transport/panic szemantikát. Ne használjunk hangmódosító, azonnali globális némítást pusztán a takarítás helyett.

**Elfogadás:** nincs beragadt editorhang; a host által tartott hang és automatizálás érintetlen; CC/bend állapot helyreállítása dokumentált és tesztelt; REAPER macOS-en és Windowson ellenőrizve.

### 3. P1 — ADSR pontosság nagy mintavételi frekvencián

**Státusz:** #23-ban javítva és célzott regresszióval lefedve; új bizonyíték nélkül nem nyitjuk újra.

**Indok:** audit és célzott próba szerint az 1 másodperces határ eltérő float/double számítási ágat választ, és a hosszabb mintavételi frekvencián mért burkoló nem éri el megfelelően a célértéket.

**Munka:**

1. Tegyünk regressziós tesztet az attack/decay/release időkre a 0.1–1.0 s tartományban, különösen pontosan 1.0 s-nál; 44.1/48/96/192/384 kHz-en.
2. Rögzítsük a matematikai elvárást: célértékhez való közelítés, véges kimenet, monoton szakaszok, release végi csend, valamint az 1.0 s körüli folytonosság.
3. Javítsuk a számítási pontosságot a lehető legkisebb viselkedésváltozással. Rövid, tipikus presetburkolókra bit-/tűréshatár alapú kompatibilitási teszt szükséges.

**Elfogadás:** az összes frekvencián stabil és véges burkoló; nincs határugrás 1.0 s-nál; a rövid burkolók regressziói és zenei meghallgatási ellenőrzése rendben.

### 4. P1 — VST3 tail jelentés és offline render

**Státusz:** #31-ben mainbe került; platform-CI sikeres, natív hostelfogadás hátravan. A véges maximum és mérési feltételek a [tail-szerződésben](VST3_TAIL_CONTRACT.md) szerepelnek.

**Indok:** a plugin nulla tailt jelenthet a hostnak, miközben a burkoló, delay és reverb hangot adhat a MIDI leállása után.

**Munka:** számítsuk ki vagy definiáljuk a hanghatás-lánc alapján a host felé jelentett tailt. Vegyük figyelembe a legnagyobb delay feedback/idő, reverb lecsengés és amp release értéket; ne adjunk tetszőleges vagy félrevezető végtelen értéket. VST3 hostban ellenőrizzük az offline render befejeződését.

**Elfogadás:** a REAPER export nem vágja le a hallható tailt, a render nem vár korlátlan ideig, a VST3 validator sikeres.

### 5. P1 — Verzió, kiadási metaadat és dokumentációs konzisztencia

**Státusz:** fejlesztői buildazonosítás #24-ben mainbe került. A következő kiadás dokumentációja és csomagjai a tényleges release-folyamat részei.

**Indok:** a kiadott v1.0.3 után a főágon további commitok vannak, de a fejlesztői build továbbra is kiadásként jelenhet meg. A fejlesztési/merge-review dokumentumok között régebbi `release.json` státusz maradt.

**Munka:** hozzunk létre egységes verzióforrást a fejlesztői build és a kiadás számára. Következő tervezett kiadásként 1.0.4 csak akkor rögzíthető, ha a kiadási döntés megszületett; addig egyértelmű `dev`/RC buildjelölés szükséges. Frissítsük a README, changelog, release notes, csomagoló és About/build-metadata leírásokat, a korábbi tagek és assetek változatlanul hagyásával.

**Elfogadás:** a főági build nem téveszthető össze publikált v1.0.3 binárissal; a dokumentáció és buildmeta ugyanazt a státuszt írja; a tag és release kizárólag tényleges release folyamatban készül.

### 6. P2 — Paraméterautomatizálás és presetbetöltés koherenciája

**Indok:** a jelenlegi paraméterkezelés blokkszintű értéket alkalmaz; presetbetöltéskor a sok paraméter nem egyetlen atomi hangállapotként érkezik. Ez ismert korlátozás, nem adatverseny.

**Munka:** külön specifikáljuk a sample-accurate automatizálás és az atomi presetváltás követelményeit. Teszteljük gyors cutoff/volume/waveform automatizálást, host visszatöltést és presetcsere közbeni audio blokkot. Csak mérhető vagy hallható hibára vezessünk be változtatást; parameter ID/state kompatibilitás maradjon.

### 7. P2 — Presetkezelési és fájlrendszer-megbízhatóság

**Vizsgálandó tételek:** szigorú fájlimport-dekóder a kompatibilis host-state dekódertől elkülönítve; enum/egész paraméterek kanonikus kerekítési szerződése; symlink policy; hard link nélküli átnevezés; atomic save és könyvtár tartóssága; importkor a közel azonos lebegőpontos értékek összehasonlítása; mentési backupok felhasználói helyreállítási politikája.

**Sorrend:** előbb formátum- és kompatibilitási teszt, aztán dokumentált policy, majd külön javítások. Automatikus backup-törlés nem vezethető be, amíg nincs kifejezett megőrzési és helyreállítási szabály.

### 8. P2 — Teljesítmény- és tesztlefedettség célzott javítása

Mérjük az audio wrapper valós költségét (paraméterleképezés, telemetria, MIDI-rendezés, sűrű ARP) és a 16 voice + teljes FX szélső esetet. A már gyorsítótárazott settereket ne kezeljük igazolatlanul drága műveletként. Profil alapján tervezzünk dirty-state/caching módosítást; minden esetben ellenőrizzük, hogy nem változik a kimeneti hang.

A 62.3%-os auditált branch coverage kombinációs teszthiányokra figyelmeztet, de önmagában nem bizonyít hibát. A fenti MIDI-, reset-, ADSR- és tail-esetek kapjanak elsőbbséget; ezután vizsgáljuk a branch coverage-ben kimaradó, kockázatos állapotátmeneteket.

### 9. P2/P3 — Szűrő és hangminőség külön programban

Ez az eredeti fejlesztési irány kiemelt hangminőségi része. A cél a határozottabb, zeneileg kontrollálható trance-szűrés, de a „prémiumabb” hangot nem szabad pusztán kódolvasásból kijelenteni.

1. Készítsünk szintillesztett tesztkészletet cutoff-, rezonancia-, Drive- és envelope-söprésekkel, több hangmagasságon és mintavételi frekvencián.
2. Mérjük a slope-ot, rezonancia csúcsát, aliasingot, DC-t, tranziens választ és CPU-t; hasonlítsuk össze rögzített kontrollhangokkal.
3. A Drive túlmintavételezése, a rezonáns filter karaktere és a SUB triangle felharmonikus-határa külön hipotézis legyen. Egyik sem bizonyított regressziós hiba az audit alapján.
4. Vak, szintillesztett A/B meghallgatás és kompatibilitási vizsgálat után válasszunk hangkarakter-változtatást. Gyári presetek alapértéke csak külön hangtervezési jóváhagyással változzon.

### 10. P2/P3 — Factory presetek és presetbrowser bővíthetősége

Az eredeti kérés szerinti cutoff-központú hangok (például Crystal Pad CutOff és SuperSaw CutOff jellegű hangok) összehasonlító presetként készülhetnek, ha a filtertesztek stabilak. A presetben az ADVANCED modulációs kerék és cutoff-moduláció legyen dokumentált, szándékos és visszahallgatható. Ezek kezdetben teszt/user presetek; csak validálás és jóváhagyás után kerüljenek factory készletbe.

A felső gyors presetlista ABC sorrendje, majd sok hang esetén kategória/mappa → jobbra nyíló ABC presetlista jó későbbi UX irány. Előbb mérjük fel a valós factory/user presetmennyiséget és billentyűzetes/akadálymentes navigációt; a GUI méretezését és jelenlegi elrendezését őrizzük meg.

### 11. P3 — 32 voice és további motorfunkciók megvalósíthatósága

16-ról 32 voice-ra váltás nem automatikus minőségi javulás, ha az oszcillátorszám változatlan. Előbb mérjük a 16 és 32 voice legrosszabb életszerű terhelését (unison, filter Drive, moduláció és FX), hanglopási/artikulációs viselkedését és több platform CPU-ját. Csak akkor valósítsuk meg, ha a 32 voice kívánt zenei előnyt ad és a CPU/memória keret teljesül; GUI- vagy presetparaméter-változás nélkül.

Waveform aliasing, inactive oscillator/voice optimalizálás és más motorfejlesztés szintén méréshez kötött. GUI-layout változtatás nincs napirenden.

## Következő release kapui

A következő nyilvános kiadás verziószámát külön release döntés rögzítse. Addig a munka fejlesztői/RC buildként azonosítható. Kiadás csak akkor következik, ha:

- az 1–5. mérföldkő elfogadási feltételei teljesülnek, vagy bármely nyitva hagyott tételről dokumentált, indokolt döntés születik;
- minden új regressziós és teljes repository teszt zöld macOS/Windows/Linux CI-n, megfelelő sanitizer futásokkal;
- VST3 validator, platformcsomagok, telepítés/eltávolítás és fájlellenőrzések sikeresek;
- REAPER-ben ellenőriztük a MIDI-ownership, overflow, reset/close és offline tail eseteket;
- a GUI és hangkarakter változásait a felhasználó elfogadta; a filter- és presetkísérlet nincs észrevétlenül a stabilitási javításokba keverve;
- kézikönyvek, changelog, kiadási jegyzetek, verziómetaadatok, licenc-megjegyzések és csomaghash-ek egyeznek a tényleges csomaggal.

## Javasolt megvalósítási egységek

1. PR: MIDI forrástulajdonlás regresszió és javítás.
2. PR: editor CC/bend overflow és close/reset lifecycle.
3. PR: ADSR hosszú idő/nagy mintavétel pontosság.
4. PR: VST3 tail + offline render validáció.
5. PR: verziómetaadat és fejlesztői dokumentáció konzisztenciája.
6. Külön tervezési/teszt PR-ek: preset atomicitás, fájlrendszer, performance, filterminőség, presetek és 32 voice döntés.

Egy PR egy jól körülhatárolt viselkedést változtasson, és tartalmazzon regressziós tesztet, ha a hiba automatizálható. A sikeres build nem helyettesíti a hallásos vagy valós hosttesztet; az eredményeket konkrét commitra kell rögzíteni.
