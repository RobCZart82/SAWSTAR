# SAWSTAR prémium filter fejlesztése

Frissítve: 2026-10-03. Alap: a kiadott 1.0.4 filter és a `71a4855` main.
A cél határozottabb cutoff-karakter, zeneileg használható rezonancia, tiszta
moduláció és jó minőségű Drive. A „prémium” hallásos elfogadási cél, nem a
szűrő neve alapján bizonyított minőség vagy más hangszer hangjának ígérete.

## A jelenlegi filter

A `LowPass` sztereó TPT SVF-et használ. LP24 módban a második kétpólusú
fokozat Q=0,5, az első Q=0,5–10. Nulla rezonanciánál mindkét fokozat a cutoffnál
−6,02 dB-t ad: együtt −12,04 dB. A negyedrendű Butterworth referencia ugyanitt
−3,01 dB, míg a távoli vágási meredekség mindkét szűrőnél 24 dB/oktáv.
Ez mérhető karakterkülönbség, nem bizonyítja, hogy a mostani implementáció hibás.
A Drive host mintavételi frekvencián fut; az erős nemlineáris gerjesztés aliasingja
külön későbbi minőségi feladat. A korábbi dokumentumok történeti validációt tartalmaznak.

## Első kutatási jelölt

Az `experiments/premium_filter/PremiumLowPass.h` önálló lineáris négy pólusú
prototípus. Két sztereó TPT fokozat, saját állapotok és 10 ms vezérlősimítás.
Nulla rezonancián a csillapítási tényezők 1,847759 és 0,765367; a szorzat
negyedrendű Butterworth frekvenciamenetet ad. A rezonancia csak az első fokozat
csillapítását csökkenti; ennek skálája kutatási választás, nem azonos a régi
filter százalékskálájával. Nincs új Drive, túlmintavételezés vagy önrezgési ígéret.

A jelölt kizárólag teszt/preview targetben szerepel. A kiadott `LowPass`, Synth,
GUI, parameter ID-k és state formátum változatlanok. A plugin nem választja ki
az új szűrőt; a régi presetek hangja ebben a munkacsomagban nem módosul.

Az SVF integráció matematikai háttere:
[Andrew Simper, Linear Trapezoidal Integrated State Variable Filter](https://www.cytomic.com/files/dsp/SvfLinearTrapOptimised.pdf).
A jelölt saját implementáció a SAWSTAR meglévő TPT szerkezetére építve;
nem került át külső szintetizátor vagy könyvtár forráskódja, nincs új függőség.

## Mérés és hallásos próba

A `premium_filter_response` teszt független analitikus átviteli függvényhez
hasonlítja a mért szinuszválaszt. 8 / 44,1 / 48 / 96 / 192 / 384 kHz-en
ellenőrzi a cutoffot és a vágást, sztereó izolációt, szélső cutoff/rezonancia
váltásokat, resetet és hibás bemenet utáni csatornánkénti helyreállást.
Ez funkcionális teszt, nem minden moduláció/gerjesztés stabilitási bizonyítása.

A cutoff-mérés minden tesztelt frekvencián:

| Filter | Szint 1000 Hz bemenettel, 1000 Hz cutoff, nulla rezonancia |
|---|---:|
| Régi LP24 | −12,0412 dB |
| Jelölt LP24 | −3,0103 dB |

A `sawstar_premium_filter_preview OUTPUT_PREFIX` két 12 másodperces,
48 kHz sztereó float WAV-ot ír. Ugyanaz az egyetlen generált SevenSaw forrásjel
kerül mindkét filterbe, azonos bemeneti gainnel és közös kezdő/záró rámpával.
Három másodperc nyitott bevezető után öt másodperces 8000→180 Hz sweep,
majd négy másodperc tartás. Drive és további FX nincs; nincs kimeneti normalizálás.

A fájlok A=régi LP24, B=jelölt LP24. A 30% rezonancia eltérő belső csillapítást
jelent a két modellben. Ez feltáró karakterpróba: a különbséget nem szabad
vakon jobb minőségként értékelni. A későbbi döntési A/B külön szintillesztett,
összevethető rezonanciájú változatokat is igényel.

## Első ellenőrzési eredmények 2026-10-03

Helyi Release CTest: 71/71 sikeres, benne az új választeszt és a meglévő
filter/moduláció regressziók. A külön ASan/UBSan prototípusteszt sikeres.
A két WAV fejlécét, hosszát, csatornaszámát, véges mintáit és clippingmentességét
programból ellenőriztük. Peak A: 0,052403; B: 0,074782; nincs normalizálás.
Windows/macOS CI és a hallásos minősítés külön következő ellenőrzés.

## Hallásos visszajelzés és hangerőkontroll

A tulajdonos az első, normalizálás nélküli B mintát jobb minőségűnek hallotta.
Ez a kutatási irány pozitív visszajelzése, nem végleges plugin-elfogadás.
A két minta eltérő szintje és rezonanciaskálája miatt külön kontroll következik.

A preview eszköz az eredeti A/B mellé `-B-RMS-matched-LP24.wav` fájlt is ír.
A B egyetlen állandó gainnel kerül az A 0,5–11,5 másodperces ablakának sztereó
RMS-szintjére. Nincs kompresszor vagy pillanatról pillanatra változó szintkorrekció:
a sweep dinamikája és filterkaraktere megmarad. A globális RMS-egyezés nem
jelent azonos érzékelt hangerőt minden pillanatban; a rezonanciaskálák is eltérnek.
A mért korrekció 0,644361 (−3,81741 dB). A WAV-formátumot, RMS-egyezést és
az állandó korrekciót a `premium_filter_preview_contract` teszt ellenőrzi.

A tulajdonos a hangerőillesztett B változatot is jobb minőségűnek hallotta
(2026-10-03, `02-level-control-B-RMS-matched-LP24.wav`). Az eredeti B és a
konstans RMS-illesztett B egyaránt pozitív visszajelzést kapott. Ez csökkenti
az összesített szintkülönbség magyarázó szerepét, de nem bizonyít azonos érzékelt
hangerőt vagy általános minőségi fölényt minden presetre és rezonanciára.

A tiszta LP24 jelöltet e tesztesetben elfogadott fejlesztési iránynak tekintjük.
Következő kapu: alacsony/közepes/magas rezonancia, lead/pluck/pad gerjesztés,
rezonáns csúcs és basszusátvitel mérése, majd megfelelő szintű hallásos kontroll.
Csak ezután következzen a nemlineáris Drive és anti-aliasing minőségi prototípusa.
A pluginba történő beépítés és a régi presetek kompatibilitása továbbra is külön feladat.

A forrás 130,8128 Hz, 20 cent detune, teljes unison mix és teljes width.
A korábbi fixture a belső 0–1 tartományban clampelt 60/75 értéket adta át;
az explicit 1/1 ugyanazt az eredményt adja. Az eredeti A és B változatlan PCM-jét
külön ellenőrizzük, hogy a kontroll ne változtassa meg a már meghallgatott forrást.

## Rezonancia vizsgálat 2026-10-03

A prototípus nem változott; a vizsgálati eszköz és a kontrollok bővültek.
A választeszt most 25/50/75/100% rezonancián is független analitikus átviteli
függvényhez hasonlítja a mért szinuszválaszt, hat mintavételi frekvencián.
A százalék továbbra is a jelölt saját csillapítási skálája, nem a régi filterrel
azonos Q. A szintek 48 kHz-en, 1000 Hz cutoffnál, beállt állapotban:

| Rezonancia | 100 Hz bemenet gain | 1000 Hz bemenet gain |
|---|---:|---:|
| 0% | kb. 0,00 dB | −3,01 dB |
| 10% | +0,06 dB | −0,48 dB |
| 30% | +0,12 dB | +4,59 dB |
| 50% | +0,14 dB | +9,66 dB |
| 70% | +0,15 dB | +14,72 dB |
| 90% | +0,15 dB | +19,79 dB |
| 100% | +0,15 dB | +22,32 dB |

A basszus közel egységnyi átvitellel megmarad; a rezonáns sávban jelentős
kiemelés lehetséges. Ez nem torzítás vagy hiba bizonyítéka, de a headroom,
rezonanciaskála, telítés és a későbbi kimeneti szintkezelés tervezési tényezője.
A táblázat két kiválasztott frekvencia erősítése, nem teljes peak-keresés.
Automatikus basszusvesztést vagy dinamikus gain-kompressziót nem vezettünk be.

A preview CLI új opcionális argumentumai:
`OUTPUT_PREFIX [RESONANCE_PERCENT [sustain|lead|pluck|pad]]`.
Argumentum nélkül a korábbi 30%-os sustain A/B fájl byte-azonos marad.
A lead egy oktávval feljebb szól; a pluck 0,5 s periódusú, rövid támadású és
lecsengésű forrás; a pad három azonos súlyú hang és lassabb felfutás.
Mindegyik közös forrásgerjesztést használ az A és B számára, három másodperces
nyitott bevezetővel, öt másodperces cutoff sweeppel, 12 s teljes hosszal.
Ezek külön DSP-gerjesztési minták, nem pluginba integrált factory presetek.

Mindhárom jelenet 10/50/90% rezonanciával készült el: 9 A/B pár, mindhez
konstans RMS-illesztett B kontroll. A fájlkontraktus-teszt mind a 9 kombinációt
ellenőrzi: hossz, formátum, véges/clippingmentes jel, állandó gain és RMS-egyezés.
Hibás/NaN/végtelen/tartományon kívüli rezonancia és ismeretlen jelenet nem ír fájlt.
A szélső rezonanciák analitikus választesztje és az ASan/UBSan kontroll is
külön fut; hallásos elfogadás és natív engine-integráció továbbra is következő kapu.

## Következő megvalósítási lépések

A tulajdonos a hangerőillesztett lead minták közül az 50%-os rezonanciájú
jelöltet preferálta (2026-10-03, `lead-res50-B-RMS-matched-LP24.wav`).
Ez a közepes rezonanciakarakter választása a három vizsgált lead mintából,
nem minden preset vagy a teljes rezonanciatartomány elfogadása. A rezonancia
továbbra is állítható marad; nem rögzítjük a pluginban 50%-ra.

A következő hallásos kapu a már elkészült `pluck-res50` és `pad-res50`
konstans RMS-illesztett jelöltje. Ezeken a lecsengés, a rezonáns csengés,
a cutoff karaktere és a pad teltsége vizsgálandó. A tulajdonos mindkét mintán kellemesnek hallotta a rezonanciát (2026-10-03).
A közepes rezonanciakarakter így lead, pluck és pad mintán is pozitív
visszajelzést kapott; a Drive külön minőségi prototípusa megkezdődött.
Az eddigi lineáris jelölt és a kiadott plugin jelútja változatlan.

1. A tiszta jelölt hallásos és szélesebb gerjesztési kontrollja: lead, pad, pluck,
   basszus; mérhető frekvenciamenet és rezonáns csúcs. Legyen elegendő bevezető.
2. Rezonancia/gain viselkedés és Drive helye: a telítés nem fedheti el egyszerű
   hangerőemeléssel a karaktert. Automatikus basszuscsökkentés külön döntés.
3. Nemlineáris fokozat minőségi jelöltje: túlmintavételezett változat, megfelelő
   fel/le mintavételi szűrés, aliasing-mérés nagy hangokon és erős Drive-on.
   Az oversampling önmagában nem garancia; latency, sztereófázis és száraz út is tesztelendő.
4. 16 voice, kis buffer és több mintavételi frekvencia CPU-profilja. Fix realtime
   költség, allokálás/zárolás nélkül. 32 voice és Linux biztosan későbbi release.
5. Kompatibilitási szerződés, majd engine-integráció: régi state/preset régi
   modellre tér vissza, új modell választása megkülönböztethető. Ne cseréljük le
   rejtetten a meglévő módokat; parameter/state migrációhoz külön regresszió szükséges.
6. Factory presetek, dokumentáció, Windows/macOS CI és natív hallásos elfogadás.

A lezárt click/pop kutatás nem indul újra. A prémium filter új hangminőségi
fejlesztés; az elfogadott GUI/kompatibilitási követelmények megmaradnak.


## Első Drive prototípus 2026-10-03

A `PremiumDrive.h` saját MIT kutatási megvalósítás, a lineáris jelölt előtt.
A nonlinearitás `tanh(g*x)/g`, ahol g a 0–24 dB bemeneti Drive erősítése.
A kisjelű erősítés közel egy marad; a telített csúcs szintje csökkenhet.
Ez nem automatikus hangerőillesztés. Nulla Drive-nál a waveshaper identitás,
de a fel/le mintavételi FIR-ek továbbra is szűrnek és késleltetnek.
A tiszta pluginút bitazonosságát ezért ez az osztály önmagában nem biztosítja.

A telítés négyszeres belső mintavételen fut. Mindkét mintavételváltáshoz
129 tagú, Blackman-ablakos sinc FIR tartozik, 0,1125 ciklus/belső minta
határfrekvenciával és egységnyi DC-erősítéssel. Az interpolátor négyszeres
skálázást kap; a decimálás csak a kimeneti szűrés után történik.
Az eredeti referencia direkt konvolúciót használ, fix tömbökkel.
Az első optimalizálás és a helyi 16 voice profil eredménye alább szerepel;
a magas mintavételi ráták és a teljes engine költsége még nyitott. A lineáris FIR-lánc késleltetése
32 hostminta (48 kHz-en kb. 0,667 ms). A rezonáns filter host rátán marad;
ez nem a teljes szűrő vagy a rezonáns visszacsatolás túlmintavételezése.

A mintavételváltás matematikai háttere:
[Julius O. Smith, Windowed Sinc Interpolation](https://www.dsprelated.com/freebooks/pasp/Windowed_Sinc_Interpolation.html).
Nincs átvett külső forráskód vagy új könyvtárfüggőség.

A célzott teszt 44,1/48/96/192 kHz-en 3/16 hostfrekvenciájú,
0,75 amplitúdójú szinuszt használ (48 kHz-en 9000 Hz). A 7/16 frekvencián
mért, visszahajló harmadik harmonikus változása a normál rátájú azonos
waveshaperhez képest: 12 dB Drive-nál −77,24 dB, 24 dB-nél −26,53 dB.
Ez egy koherens tónus egy kiválasztott spektrális komponense; nem a teljes
aliasenergia vagy tetszőleges zenei gerjesztés minőségi bizonyítása.
Erős Drive-nál marad mellékhang; a további frekvencia/amplitúdó-sweep és
magasabb rátájú referencia külön következő mérési kapu.

A regresszió az impulzus késleltetését, a nulla Drive késleltetett
passband-pontosságát, a törlést, a sztereó izolációt, a hibás bemenet utáni
helyreállást és a gyors Drive-moduláció véges kimenetét is ellenőrzi.
ASan/UBSan ellenőrzés is tartozik hozzá. A nyolc Drive-preview (0/12/20/24 dB,
`host`/`4x`) WAV-kontraktusa, clippingmentessége, konstans RMS-illesztése és
azonos A referenciája külön tesztelt.

A preview opcionális kiegészítése: `[DRIVE_DB [host|4x]]` a jelenet után.
Drive-próbában a közös bemenet tízszeres, a közös kimeneti gain 0,15,
hogy hallható telítés mellett megmaradjon a headroom. Mindegyik A referencia
Drive nélküli régi LP24; B az új filterrel feldolgozott jel. A normál rátájú
ágak 32 mintát késnek; a túlmintavételezett B cutoff-idővonala ugyanennyivel
eltolódik. Az RMS-illesztett B fájlok ugyanahhoz az A referenciához igazodnak,
0,5–11,5 s között, egyetlen állandó gainnel. Az eltérő Drive-k nélküli korábbi
preview A/B fájlok byte-azonosak maradtak. Az új Drive-minták még hallásos
értékelésre várnak; nem jelentik a végleges filtermodell elfogadását.

Következő kapuk: Drive hallásos karakter, szélesebb aliasing-mérés, FIR
passband/stopband teljes karakterizálás, CPU-optimalizáció és profilozás,
majd a száraz/wet út, moduláció és kompatibilis engine-integráció terve.


## Drive visszajelzés és 20 dB kontroll

A tulajdonos pontosítása szerint a 24 dB-os mintának tiszta, de szélsőséges:
feszesebb, pulzáló, érdesebb karaktere van. Potmétermaximumként elfogadható,
de a szokásos használatot körülbelül 20 dB-ig képzeli el. A 24 dB nem
elutasított vagy bizonyítottan hibás hang. A 0–24 dB tervezett tartomány
megmarad, 20 dB alatt jól adagolható szabályozással; ezt a GUI-integrációban
külön kell ellenőrizni. A 20 dB-os lead kontroll elkészült, hallásos értékelése
még nyitott. Nem állítunk előre elfogadást a 12 vagy 20 dB-os mintára.

A korábbi koherens tónuskontroll 20 dB Drive-nál −34,98 dB csökkenést mért
a kiválasztott visszahajló komponensben. A teszt mind a négy mintavételi rátán
és a preview-kontraktus mindkét feldolgozási módban erre az értékre is bővült.
Ez továbbra sem általános aliasmentességi állítás.

## Első CPU optimalizálás

A `ReferencePremiumDrive.h` megőrzi az eredeti teljes konvolúciós megvalósítást.
A kutatási `PremiumDrive` az interpoláció ismert nulla mintáit kihagyja, a
kimeneti konvolúciót pedig csak a decimáláskor megtartott fázisban számolja.
A kihagyott fázisok előzményeit továbbra is beírja. A szűrőegyütthatók,
a telítés, a vezérlősimítás és a 32 mintás késleltetés változatlan.

Az első optimalizálás regressziója 10 000 determinisztikus sztereó mintán,
rátánként, változó Drive, Snap, Clear és hibás bemenet mellett pontos
mintánkénti egyezést követelt az eredeti referenciával. A második optimalizálás
numerikus kontraktusa a következő szakaszban szerepel. A 20 dB-os teljes raw és RMS-illesztett WAV is
byte-azonos az optimalizálás előtti kontrollal.

A `sawstar_premium_drive_benchmark [--reference]` külön Release profilozó
eszköz; nem időzítésfüggő CI pass/fail teszt. Egy mérés 8192 sztereó mintát
feldolgozva három ismétlés mediánját jelenti; előtte 256 minta bemelegítés.
Csak a Drive fut, a source-fixture előre generált; nincs benne oszcillátor,
rezonáns filter, FX, wrapper vagy valódi host-audio callback.
A százalék a feldolgozási idő és a modellezett audioidő aránya egy szálon,
nem a plugin CPU-kijelzője vagy a teljes gép kihasználtsága.

A helyi macOS `clang++ -O3 -DNDEBUG` futás 16 hang és 20 dB Drive mellett:

| Host ráta | Eredeti referencia | Első optimalizált jelölt |
|---|---:|---:|
| 48 kHz | 88,27% | 38,20% |
| 96 kHz | 174,33% | 76,75% |
| 192 kHz | 351,04% | 152,24% |

Ezek két helyi futás adatai, környezetfüggőek; nem platformfüggetlen
sebességígéretek. Az első optimalizálás jelentős, de a 192 kHz-es 16 voice
Drive önmagában is több időt igényel, mint amennyi rendelkezésre állna.
A 96 kHz-es tartalék szintén kevés a teljes motorhoz. Emiatt további
hatékonysági/minőségi terv és kis-bufferes teljes engine profil szükséges
az integráció előtt. A 32 voice továbbra is halasztott.


## Második CPU optimalizálás és elfogadott 20 dB minta

A tulajdonos a `04-drive20-B-RMS-matched-LP24.wav` lead mintát szépen
szólónak hallotta (2026-10-03, 50% rezonancia). Ez e teszteset pozitív
visszajelzése, nem a teljes Drive-tartomány vagy az integrált plugin elfogadása.

A második optimalizálás tükrözött FIR előzményt használ: a belső tároló
minden mintát két helyre ír, így az együtthatókhoz tartozó minták folytonos
memóriából olvashatók. A szorzatösszeg négy független double akkumulátorral
számolható, ami fast-math nélkül is segíti a vektorizálást. Az együtthatók,
fel/le mintavételi faktor, nonlinearitás, simítás és késleltetés változatlan.
A tároló nő, de továbbra is fix méretű; feldolgozás közben nincs új allokáció.

Az összegzés sorrendje eltérhet, ezért a referencia-kontraktus most numerikus
hibahatárt rögzít: maximum abszolút mintahiba <1e−7 (−140 dBFS), RMS-hiba
<1e−8 (−160 dBFS) a 10 000 mintás moduláció/reset/hibás bemeneti kontrollban.
Ez nem bitazonossági ígéret minden platformra vagy tetszőleges gerjesztésre.
A helyi négy rátás kontrollban a tényleges eltérés nulla volt; a teljes 20 dB-os
raw és RMS-illesztett WAV mintái is pontosan egyeztek az eredeti referenciával.
A független aliasing-, passband- és késleltetési tesztek megmaradtak.

Azonos profilozási módszerrel, helyi Release futásban, 16 hang és 20 dB mellett:

| Host ráta | Első optimalizálás | Második optimalizálás |
|---|---:|---:|
| 48 kHz | 38,20% | 20,75% |
| 96 kHz | 76,75% | 41,34% |
| 192 kHz | 152,24% | 82,60% |

A futások környezetfüggőek. A 192 kHz-es Drive önmagában már a rendelkezésre
álló időn belül van, de ez nem a teljes engine, kis buffer vagy natív host
elfogadása. Az oszcillátorok, rezonáns filter, FX és wrapper ezen felül dolgoznak.
Következő kapu a teljes frekvenciamenet és aliasenergia szélesebb mérése,
a kis-bufferes CPU-próba, majd a magas ráták minőség/költség politikája.
A production engine továbbra sem használja a kutatási osztályokat.


## Szélesebb spektrális és kis pufferes kontroll

A `premium_drive_spectral_grid` 160 tónuskontrollt futtat: 44,1/48/96/192 kHz,
3/5/7/11/13 osztva 32-vel hostfrekvencia, 6/12/20/24 dB Drive, 0,1/0,75
bemeneti amplitúdó. A 32 mintás koherens periódus beállás után 64 periódus
átlagából készül. A Nyquist alatt szabályos harmonikusok frekvenciabinjei
kimaradnak a residual-mérésből. Az ezekkel egybeeső aliasok nem különíthetők
el; ez nem teljes aliasenergia-mérés vagy általános aliasmentességi bizonyítás.
A magas rátákon több vizsgált frekvencia az emberi hallási tartomány fölött
van, ezért ezek numerikus stresszkontrollok, nem hallásos minősítések.

A normál rátájú azonos nonlinearitáshoz viszonyított 148 érdemben mérhető
kontroll mind javult. A residual energia változása −99,36 és −12,94 dB között;
a fundamentális energiájához viszonyított változás −99,36 és −12,38 dB között
volt. Utóbbi külön ellenőrzése kizárja, hogy csak általános kimeneti halkítás
okozza a javulási mérőszámot, de az interpolátor előtti/utáni gerjesztésváltozás
és az egybeeső aliaskomponensek elkülönítését nem oldja meg. A regresszió
mindkét mérőszámban legalább 6 dB javulást kér, ha a régi residual >1e−12.
A zajpadló közeli többi eset véges kimenete ellenőrzött; ott nem értékelünk
félrevezető nagy százalékot vagy dB-arányt minőségi javulásként.

A nulla Drive lineáris válasz külön karakterizált. 48 kHz-en: 18 kHz −0,031 dB,
19,5 kHz −1,13 dB, 21 kHz −7,12 dB, 22,5 kHz −23,24 dB. A magas sáv
csillapítása a mintavételváltás véges FIR-jének része. A passband/aliasing
tradeoff az integráció előtt kifejezetten értékelendő.

A `sawstar_premium_drive_benchmark --small-blocks` 16 Drive-példányt,
20 dB-os beállítással, 32/64/128/256 mintás darabokban mér. A medián összidő
mellett a három ismétlés leglassabb megfigyelt blokkját is jelenti a blokk
rendelkezésére álló audioidő százalékában. Ez tartalmazza a mérési overheadet
és az OS ütemezésének hatását; továbbra sem valódi host/audio-thread teszt.

| Host ráta | Medián tartomány, pufferméretek között | Legrosszabb megfigyelt 32 mintás blokk |
|---|---:|---:|
| 48 kHz | 20,34–22,76% | 63,27% |
| 96 kHz | 40,67–40,88% | 47,26% |
| 192 kHz | 81,30–82,28% | 154,95% |

192 kHz-en a 64 mintás maximum is 107,75% volt. A helyi próbában tehát
néhány rövid blokk túllépte a rendelkezésre álló időt, miközben még csak a
Drive dolgozott. A magas rátás minőség/költség policy és további optimalizálás
beépítési előfeltétel; ezt nem szabad a medián 83%-os értékkel lezárni.

## Korábbi CI teljesítményellenőrzés – beolvasztási kapu lezárva

A #47 `59db7de` headjén két macOS Release job CPU-guardja elbukott.
A 16 hangos, száraz production-engine összevetés 34,23%, illetve 35,36%
többletet mért a rögzített történeti benchmark-baseline-hoz képest.
A funkcionális tesztek mindkét jobban sikeresek voltak. A kutatási Drive
nem része ennek a benchmarknak; a PR nem módosít production engine-forrást.
Ezért a két mérés önmagában nem bizonyítja, hogy a Drive-kutatás okozta
az eltérést. A korábbi sikertelen futások megmaradnak auditnyomként.

A küszöb változatlan. A friss `0c3f4e2` head minden GitHub-ellenőrzése
sikeres lett; a #47 beolvadt a mainbe (`29a25ac`). A kutatási
Drive-profilozás sikerét nem tekintjük a sikertelen production CPU-guard
helyettesítő igazolásának.


A helyi production-kontroll ugyanazzal a rögzített baseline-nal, nyolc
váltakozó sorrendű mérési párral és az eredeti guard-küszöbökkel sikeres lett.
A jelenlegi helyi Release buildet használtuk (`build-tail-contract`); a script
forrása és statisztikája nem módosult. A száraz 16 voice többlete +7,57%,
a többi CPU-különbség +6,01–11,13%. Az audio-szint és DC guardok is sikeresek.
Ez fontos ellenkontroll, nem a CI-eltérés okának bizonyítása. A friss head
GitHub CPU-guardja is sikeres lett; az eltérés pontos oka ettől még nem bizonyított.
A szélesebb munkacsomag teljes helyi CTest eredménye 74/74; az új spektrális
teszt külön ASan/UBSan alatt is sikeres.

## Harmadik optimalizálás – szimmetrikus FIR

A matematikailag szimmetrikus 129 tap együtthatópárjait átlagoljuk az
inicializáláskor, így a libm kerekítési eltérése nem akadályozza a párosított
konvolúciót. A decimátor és a páros interpolációs fázisok tükrözött bemeneti
mintáit összeadjuk, majd közös együtthatóval szorozzuk. A páratlan fázisok
megtartják a teljes összeget. A FIR hossza, cutoffja, négyszeres mintavétel,
32 mintás késés és a Drive görbéje változatlan.

A korábbi numerikus toleranciák változatlanok. A determinisztikus random
kontroll helyben minden rátán nulla float kimeneti eltérést mért; ez nem
általános bitazonossági ígéret. Új impulzuskontroll ellenőrzi a ring-buffer
határait, két csatornát és 0/20/24 dB Drive-ot az eredeti teljes FIR ellen.

Helyi Release mérés, csak a Drive, 16 voice és 20 dB:

| Ráta | Korábbi négy részösszeg | Szimmetrikus FIR |
| --- | ---: | ---: |
| 48 kHz | 20,75% | 16,89% |
| 96 kHz | 41,34% | 33,87% |
| 192 kHz | 82,60% | 66,40% |

A 32/64/128/256 mintás próba mediánja rendre kb. 17/34/67% a három rátán.
192 kHz-en a 32 mintás legrosszabb mért blokk még 130,58%; a teljes motor
és natív host költségét ez nem méri. Az integrációhoz szükséges magas rátás
CPU-tartalék továbbra sem igazolt. Az adatok egy helyi futás eredményei,
nem garantált platformsebességek. A korábbi spektrális és preview-kontrollok
sikeresek; a gyártási engine jelútját nem módosítottuk.

A 20 dB-os lead teljes raw és RMS-illesztett renderje az előző optimalizált
mintával helyben mintánként azonos lett. A négy célzott CTest és a kibővített
Drive-regresszió külön ASan/UBSan futása sikeres. A platform-CI az új PR
külön beolvasztási kapuja.
