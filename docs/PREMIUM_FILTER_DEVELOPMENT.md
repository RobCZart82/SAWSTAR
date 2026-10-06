# SAWSTAR prémium filter fejlesztése

Frissítve: 2026-10-04. Kutatási kiindulás: a kiadott 1.0.4 filter és a `71a4855` main.
A cél határozottabb cutoff-karakter, zeneileg használható rezonancia, tiszta
moduláció és jó minőségű Drive. A „prémium” hallásos elfogadási cél, nem a
szűrő neve alapján bizonyított minőség vagy más hangszer hangjának ígérete.

## Elfogadott teljes filtercsere

A tulajdonos 2026. október 4-i döntése szerint a prémium filter **teljesen
leváltja** a jelenlegi szűrőt. Nem lesz Classic/Premium modellválasztó,
és nem tartunk fenn felhasználó által választható régi filtermodellt.
Ez felülírja a korábbi, régi hangkaraktert megtartó opt-in integrációs tervet.

A MAIN oldal meglévő FILTER kezelőszervei maradnak: Cutoff, Resonance,
Drive (0–24 dB), Filter Mix, Key Track és a négy filtermód lenyíló listája.
A FILTER ENV A/D/S/R és Amount feladata változatlan. Nincs új GUI kezelőszerv,
és a meglévő parameter ID-k, filtermód-számértékek és automation kapcsolatok
megmaradnak: 0=LP12, 1=LP24, 2=HP12, 3=BP12.

A régi presetek/projektek mentett értékei továbbra is betölthetők, de ezután
is az új motor dolgozza fel őket. A filter frekvenciamenete, rezonanciája és
Drive karaktere szándékosan változhat. A korábbi renderrel való bitazonosság
nem elfogadási feltétel a végleges filtercserére. A kiadási jegyzet és a
kézikönyv ezt a hallható változást kifejezetten jelzi. A régi implementáció
kutatási/tesztreferenciaként használható; a production jelútban nem marad
kompatibilitási alternatíva.

Teljes csere csak az alábbi kapuk lezárása után:

1. Magas rátás Drive minőség/költség megoldása, a 4x referenciával mérve;
   a jelenlegi 192 kHz-es teljes motor költsége még túl nagy.
2. A prémium LP24 mellett LP12, HP12 és BP12 kidolgozása. Minden módhoz saját
   cutoff/center, meredekség, rezonancia/gain és szélső gerjesztési kontroll kell.
   Egyetlen LP24 algoritmus nem helyettesítheti némán a többi módot.
3. Filtermód- és Drive-váltás, cutoff-envelope, LFO/mod wheel, key tracking,
   dry/wet keverés, sztereó izoláció és visszaállítás ellenőrzése.
   A késés és a host felé jelentett latency kezelése külön tervezési feladat;
   a kutatási 32 mintás adapter nem kész plugin-latency megoldás.
4. Engine-integráció a meglévő vezérlőkkel; régi state/preset betöltési és
   automatizálási regresszió, minden factory preset újrahallgatása.
5. 16 voice CPU-próba és natív Windows/macOS elfogadás, majd dokumentált kiadás.
   Linux és 32 voice továbbra is halasztott.

A #49 alatt beolvadt kezdeti teljes motorpróba (`6eb4fc2`) csak LP24-et támogatott.
A mostani kutatási prototípus és az offline motoradapter már mind a négy módot
kezeli; a részletek és a fennmaradó elfogadási kapuk a dokumentum végén vannak.
A kiadott plugin filtere még nem cserélődött le.

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
5. A teljes filtercsere fenti szerződése szerint mind a négy mód befejezése,
   majd engine-integráció a meglévő vezérlőkkel. Régi preset/state értékei
   betöltődnek az új motorba; a hangkarakter változása dokumentált. Nincs
   Classic/Premium választó; parameter/state/automation regresszió szükséges.
6. Factory presetek, dokumentáció, Windows/macOS CI és natív hallásos elfogadás.

A lezárt click/pop kutatás nem indul újra. A prémium filter új hangminőségi
fejlesztés; a GUI és a mentett vezérlőértékek szerződése megmarad, a filter
hangkarakterét az új tulajdonosi döntés szerint lecseréljük.


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

## Teljes motoros kutatási próba 2026. október 4.

A #48 minden GitHub-ellenőrzése sikeres lett és beolvadt (`20fe01f`).
A következő mérés már az aktuális Synth oszcillátorait, burkolóit,
modulációját, hangkezelését, effekteit és kimeneti védelmét is futtatja.
A build egy külön névtérben fordítja a Synth aktuális forrását: csak a
hangonkénti filtert cseréli `EnginePremiumFilter` kutatási adapterre.
A forrásmásolat automatikusan újragenerálódik Synth-módosításkor.
Ez elkerüli egy kézzel karbantartott, később elavuló motorfork használatát.
A gyártási `sawstar_engine` és plugin változatlan.

Az adapter kizárólag LP24 kutatási módot enged; más módra hibát jelez.
A jelút PremiumDrive → PremiumLowPass, a dry mix ág 32 mintával késleltetett,
a mix 10 ms alatt simított. A jelenlegi kutatási Drive saját telítési
karakterét használja; nem emulálja a régi filter Drive/DC-kezelését.
A burkolók a meglévő Synth idővonalán futnak. Ez CPU- és működési próba,
nem a végleges preset/state/módváltási vagy latency-kompatibilitási megoldás,
és nem natív VST3/host mérés.

A motor minden mintán közli a cutoff/rezonancia értékét a filterrel.
A rezonancia együtthatóját most csak a kanonizált érték változásakor
számoljuk újra. Az Init érvényteleníti a cache-t; külön teszt ellenőrzi
az újrainicializálást és a változatlan kontrollok közlésének azonosságát.
A korábban elfogadott 20 dB lead raw és RMS-illesztett WAV helyben
mintánként azonos maradt.

A `sawstar_premium_engine_benchmark` 216 konfigurációt mér modellként:
48/96/192 kHz, 1/8/16 voice, 0/20/24 dB Drive, effektek ki/be,
32/64/128/256 mintás feldolgozási szakaszok. Három futás teljes idejének
mediánját és a legrosszabb megfigyelt szakaszt rögzíti. 2048 minta bemelegítés,
4096 minta időzített render; ellenőrzés és peak/RMS mérés is az időzített
ciklus része. Ez rövid ablak, nem teljes lecsengés vagy steady-state FX próba.
A szakaszolás nem a valódi plugin ProcessBlock és host/framework overheadje.

Helyi macOS arm64, Apple Clang 21, Release build, egymás után futtatott
premium és legacy mérés, 16 voice, 20 dB Drive, effektek bekapcsolva:

| Ráta | Legacy medián tartomány | Premium medián tartomány | Premium legrosszabb szakasz |
| --- | ---: | ---: | ---: |
| 48 kHz | 14,39–14,62% | 29,67–30,19% | 41,50% |
| 96 kHz | 28,89–29,32% | 59,24–60,08% | 88,91% |
| 192 kHz | 57,72–58,48% | 118,48–121,09% | 166,84% |

A tartomány a négy szakaszhosszt fedi le. Az eredmények gép- és ütemezésfüggők;
nem kiegyensúlyozott, több gépes sebességígéret, és a két modell más karakterű.
A 192 kHz-es kutatási megoldás átlagosan is túllépi az audioidőt: a 4x Drive
változatlan használatával még nem integrálható megfelelő CPU-tartalékkal.
96 kHz-en a legrosszabb mért szakasz közelít a kerethez; itt sincs általános
natív host tartalék igazolva.

Nyers mérési adatok:
[Premium motor](../experiments/premium_filter/measurements/2026-10-04-premium-engine.csv),
[Legacy kontroll](../experiments/premium_filter/measurements/2026-10-04-legacy-engine.csv).
Újrafuttatás Release buildben: `sawstar_premium_engine_benchmark`, illetve
ugyanez `--legacy` kapcsolóval. A `--smoke` CTest nem időkorlátot ellenőriz:
a késleltetett dry utat, az elfogadott wet láncot, Poly/Mono/Legato
note-release-t és reset utáni csendet vizsgálja négy mintavételi rátán.

Következő döntési kapu: magas hostrátán kisebb belső túlmintavételezés vagy
más, méréssel igazolt gyorsítás. A kisebb faktor még nem elfogadott megoldás.
Először abszolút hallható frekvenciákon kell összehasonlítani az aliasingot,
a teljes spektrumot, a 0/20/24 dB karaktert, késést és modulációs viselkedést
a mostani 4x referenciával. Ezután teljes motoros és natív host CPU-próba kell.
A CPU-guard küszöbei és a hangminőségi követelmények nem lazulnak.

Ellenőrzés: a teljes helyi munkacsomag 75/75 CTestje sikeres volt; a
cache- és adapter-kontroll bővítése után a két érintett teszt újrafuttatva
is sikeres, külön ASan/UBSan buildben is. Az öt premium célteszt és az
elfogadott WAV mintánkénti összevetése szintén sikeres. Az új PR teljes
platform-CI eredménye külön beolvasztási feltétel.

## Magas rátás Drive kutatás 2026. október 4.

A #50 teljes filtercsere-terv minden CI után beolvadt (`65e3f4d`). A 4x
Drive most fix faktorú sablon, a meglévő `PremiumDrive` neve továbbra is a
négyszeres változatot jelenti. A 4x referencia, a korábbi toleranciák és
spektrális tesztek megmaradnak. A 2x kutatási ág 65, a 4x ág 129 tapot
használ: mindkettő .45 hostrátájú cutoffot, Blackman-ablakot és 32 hostminta
késést ad. A telítési görbe és a 10 ms-os Drive-simítás közös.

A `RateScaledPremiumDrive` jelölt 176,4 kHz alatt 4x, attól felfelé 2x.
A faktor csak Init/reset során változik; élő streamben nincs faktorváltás.
Ez **nem elfogadott shipping policy**. A kiadott plugin nem használja,
és a standard offline motorpróba is megőrzi a 4x referenciaútját.
A második, külön fordított motorpróba teszi mérhetővé a jelöltet.

Az első, 88,2 kHz-től 2x faktorú jelölt megbukott a változatlan 10%-os
kutatási különbségkorláton: 88,2 kHz, 18 001,8 Hz bemenet, 24 dB Drive,
0,75 bemeneti amplitúdó esetén 11,05% relatív RMS-eltérés lett a 4x
referenciához képest. A követelményt nem lazítottuk: a kisebb faktort
csak a magasabb rátákra korlátoztuk. A különbség nem tisztán aliasingmérés.
[Elutasított kontroll](../experiments/premium_filter/measurements/2026-10-04-rejected-2x-88200.csv).

A jelenlegi grid 192 kontroll: 88,2/96/176,4/192 kHz; névlegesen
1/3/8/12/18/20 kHz; 0/12/20/24 dB; 0,1/0,75 amplitúdó. Koherens 1024-mintás
periódushoz igazítjuk a frekvenciát, ezért az adatfájl a tényleges Hz-et is
rögzíti; például 192 kHz-en a 20 kHz cél ténylegesen 20 062,5 Hz.
88,2 és 96 kHz-en a jelölt a referencia 4x útját használja és az eltérés nulla.
176,4/192 kHz-en a legnagyobb relatív RMS-különbség 5,14%/4,67%, 24 dB,
0,75 amplitúdó és kb. 20 kHz mellett. Ez az egész Nyquist-sáv kimeneti
eltérése: harmonikus amplitúdó/fázis és aliasing egyaránt része lehet.
Nem 5%-os hallható minőségromlást jelent, és nem bizonyít hallhatatlanságot.

Külön kiválasztott, 20 kHz alatti spektrális maradékot is mérünk: a legfeljebb
63. rendű páratlan harmonikusok behajlított binjeit, a legitim harmonikusokkal
azonos binbe eső komponensek nélkül. Ez nem teljes alias-energia. Legrosszabb
kontroll: 176,4 kHz-en a jelölt −55,86 dBFS, a 4x referencia −76,60 dBFS;
192 kHz-en −58,20 és −81,18 dBFS. A kisebb faktor mérhetően rosszabb ezekben
az erős gerjesztésekben. A teszt −50 dBFS kiválasztott maradék- és 10% teljes
relatív eltéréskorlátja diagnosztikai védőkorlát, **nem végleges hangminőségi
elfogadás**. Nulla Drive mellett külön 0,1%-os lineáris eltéréskorlát van.

Helyi Release teljes motorprofil, az előzővel azonos rövid ablakokkal,
16 voice, 20 dB, FX bekapcsolva, négy szakaszhossz:

| Ráta | 4x referencia korábbi mediánja | Rátafüggő jelölt mediánja | Jelölt legrosszabb szakasza |
| --- | ---: | ---: | ---: |
| 48 kHz | 29,67–30,19% | 29,77–29,90% | 34,06% |
| 96 kHz | 59,24–60,08% | 59,41–59,54% | 70,31% |
| 192 kHz | 118,48–121,09% | 77,91–78,05% | 95,15% |

Külön helyi futások, nem kiegyensúlyozott statisztikai sebességígéret.
192 kHz-en érdemi tartalék keletkezett a mediánban, de a legrosszabb szakasz
közel van az időkerethez. Natív Windows/macOS host vagy hosszú stresszfutás
nem történt; nem jelentjük ki a realtime integráció készségét.
[Nyers motorprofil](../experiments/premium_filter/measurements/2026-10-04-rate-scaled-engine.csv),
[spektrális kontroll](../experiments/premium_filter/measurements/2026-10-04-rate-scaled-controls.csv).

A következő kapu: összetett/unison gerjesztés, Drive-moduláció és magasabb
pontosságú aliasing-kontroll; szükség esetén a jelölt finomítása. A CPU-előnyt
nem váltjuk automatikusan hangminőségi engedményre. Ezután LP12/HP12/BP12
kidolgozása, minden mód stabilitási/hallásos tesztje, és a teljes filtercsere
integrációs próbája következik. Nincs új GUI minőségkapcsoló.

Ellenőrzés: 77/77 teljes helyi CTest sikeres. A végső csatorna/reset kontrollok
és a két külön névtérben fordított motorpróba célzott újrafuttatása sikeres;
a Drive- és rátafüggő motorregressziók ASan/UBSan alatt is sikeresek.
A korábban elfogadott 48 kHz-es, 20 dB-os raw és RMS-illesztett WAV
mintánként azonos maradt. Ezek nem helyettesítik a magas rátás jelölt
hátralévő hangminőségi és natív elfogadását.

## Összetett forrás és Drive moduláció kontrollja

A #51 minden ellenőrzése sikeres lett és beolvadt (`769993c`). A következő
kontroll a korábbi 2x/4x jelöltet ugyanazzal a forrással egy 8x offline
referenciához is méri. A `ReferencePremiumDrive` név továbbra is a korábbi
4x közvetlen konvolúciót jelenti, változatlan numerikus kontraktussal.
A külön `ReferencePremiumDrive8x` 257 tapot és megfelelően hosszú, 512 elemű
ringet használ; a késés továbbra is 32 hostminta. Ez mérési referencia,
nem production- vagy realtime jelölt. Független impulzus-késés, reset,
csatornaizoláció és késleltetett szinuszos passband-kontroll ellenőrzi.
A 8x sem matematikailag aliasmentes ground truth.

24 rövid numerikus kontroll: 176,4/192 kHz, basszus/lead/pad-voice/magas lead,
20/24 dB és lépcsős Drive célérték. Mindegyik forrás egyetlen voice két
SevenSaw unison bankjából és egy SUB-ból áll: a gyártási motorban is voice-onként
van a telítés, ezért nem keverünk teljes akkordot egyetlen nonlinear filterbe.
A gyökérhangok kb. 65/262/131/2093 Hz, az OSC2 egy oktávval feljebb, a SUB
egy oktávval lejjebb; a waveformok és detune/width az ellenőrző forrásban
rögzítettek. Egyetlen közös konstans skálázás 0,75 csúcsra illeszti a forrást,
nincs útvonalankénti vagy dinamikus normalizálás.

1024 hostminta bemelegítés után 4096 mintát mérünk. Ez nagyon rövid ablak,
nem hallásos zenei fixture vagy hosszú stresszfutás. A Drive-célok lépcsői
512 mintánként 0/12/20/24 dB értékeket is érintenek, a meglévő 10 ms simítással.
A három külön LP24 példány ugyanazt a 12 kHz-ről 500 Hz-re gyorsan változó
cutoff célt és 50% rezonanciát kapja. A gyors kontroll numerikus állapotpróba,
nem a musical envelope végleges beállítása.

A relatív RMS-különbség legnagyobb értékei a 24 kontrollban:

| Összevetés | Legnagyobb relatív RMS-különbség |
| --- | ---: |
| Rátafüggő jelölt – 4x | 0,3954% |
| Rátafüggő jelölt – 8x | 0,3868% |
| 4x – 8x | 0,0191% |
| LP24 után rátafüggő jelölt – 8x | 0,1557% |
| LP24 után 4x – 8x | 0,0076% |

A maximális szűrés előtti értékek a 192 kHz-es magas lead, 24 dB kontrollból
származnak; a legnagyobb szűrés utáni jelölt-eltérés a 176,4 kHz-es magas lead,
24 dB próba. Az RMS-különbség harmonikus/fázis/amplitúdó és aliasing hatását
is tartalmazhatja; nem izolált alias-energia, nem a hangminőség romlásának
százaléka és nem hallhatatlansági bizonyíték. A bemeneti oszcillátorok már
hostrátán előállított jelek: a kontroll nem méri külön a forrás saját aliasingját.

Diagnosztikai felső korlát: 1% nyers jelölt-eltérés a 4x/8x referenciától,
0,5% LP24 utáni jelölt-eltérés és 0,1% nyers 4x/8x eltérés. Mind teljesült.
Ezek a rögzített fixture regressziós korlátai, nem minden preset vagy
szűrőtípus minőségi elfogadása. A korábbi, kb. 20 kHz-es szinuszos stresszteszt
nagyobb maradéka továbbra is valós eredmény; nem írjuk felül az új kontrollal.
[Nyers eredmény](../experiments/premium_filter/measurements/2026-10-04-complex-source-controls.csv).

A hét célzott filter/Drive/motor teszt és az új kontroll, valamint a 4x
referencia-regresszió külön ASan/UBSan futása sikeres. Az új referencia nem
módosítja a production engine-t, a rátafüggő faktorpolitika továbbra is
kutatási jelölt. Ezt a kontrollt követően elkészültek az alábbi négy mód és
módváltási próbák. A natív host teszt és a végleges faktorpolitika döntése nyitott.

## Négy kutatási filtermód és módváltás

A #52 összetett forráskontroll mind a 17 CI-ellenőrzése sikeres lett;
beolvadt a mainbe (`f0b2851`). A négy mód munkacsomagja erre az alapra épül.

A kutatási `PremiumLowPass` és az offline `EnginePremiumFilter` már kezeli
a meglévő négy módértéket: 0=LP12, 1=LP24, 2=HP12, 3=BP12. Nincs új paraméter,
külső függőség vagy GUI-kapcsoló. A production `LowPass`, `Synth` és a plugin
változatlan; ez továbbra is kutatási implementáció.

Az elfogadott LP24 két fokozata megmarad. A három kétpólusú mód egy külön,
közös TPT SVF-ből kap low/high/band kimenetet. Nulla rezonancián sqrt(2)
csillapítás ad másodrendű Butterworth LP/HP választ. A rezonancia logaritmikusan
csökkenti a csillapítást 0,1-ig; ez kutatási karakterválasztás, külön hallásos
elfogadást igényel. A BP középfrekvenciás gainje egységre normalizált:
a rezonancia itt a sávot szűkíti, nem növeli a center gainjét.
Az LP12/HP12 távoli meredeksége 12 dB/oktáv; a kétpólusú BP alacsony és magas
oldala külön 6 dB/oktáv. A meglévő BP12 módnév/számérték megmarad.

Beállt válasz, 48 kHz, 1000 Hz cutoff és bemenet; nem maximális csúcskeresés:

| Mód | 0% rezonancia | 50% rezonancia | 100% rezonancia |
| --- | ---: | ---: | ---: |
| LP12 | −3,01 dB | +8,49 dB | +20,00 dB |
| LP24 | −3,01 dB | +9,66 dB | +22,32 dB |
| HP12 | −3,01 dB | +8,49 dB | +20,00 dB |
| BP12 | 0,00 dB | 0,00 dB | 0,00 dB |

Mindkét szűrőhálózat folyamatosan fut, hogy váltáskor ne régi, megállított
integrátorállapot jelenjen meg. A módok kimenete konvex súlyokkal vált át,
10 ms időállandóval: ez kb. 30 ms alatt éri el a változás 95%-át, nem 10 ms-os
véges fade. A jelentéktelen súlyfarok a denormál tartomány előtt megszűnik.
A súlyok egységnyi összege nem garantál állandó energiát/érzékelt hangerőt,
és önmagában nem bizonyít hallhatóan hibátlan módváltást. A harmadik TPT
fokozat CPU-költségét a következő teljes motorprofilban külön meg kell mérni;
a korábbi kétfokozatú CPU-számok nem írják le ezt a változatot.

A `premium_filter_modes` 360 szinuszos kontrollban ellenőrzi a komplex
frekvenciaátvitelt, tehát a fázist is: 8/44,1/48/96/192/384 kHz, négy mód,
0/50/100% rezonancia, 100/800/1000/1400/3000 Hz, 1000 Hz cutoff. Független
analitikus átviteli függvényhez mér; a legnagyobb komplex abszolút eltérés
helyben kb. 1,87e−7, a regressziós korlát 2e−4. Ez implementációs pontosság,
nem hangminőségi százalék.
[Nyers válaszkontroll](../experiments/premium_filter/measurements/2026-10-04-four-mode-response.csv).

További tesztek: gyors mód/cutoff/rezonancia-váltás a külön futó meleg ágakhoz
viszonyítva; sztereó szimmetria és izoláció; Init/Clear; invalid mód clamp;
NaN/Inf és szélsőséges véges bemenet utáni csatornánkénti helyreállás.
A nagy, véges bemenet rezonáns erősítése sem küldhet float-overflowt tovább.
Ez a védelmi ág nem szól bele a normál LP24 fixture-be. Az offline motorpróba
mind a négy filtermóddal ellenőrzi a wet jelutat és a Poly/Mono/Legato
hangelengedést/visszaállítást, négy rátán, bekapcsolt FX mellett.

A módfejlesztés előtti LP24-hez viszonyított külön helyi kontroll 600 000
sztereó mintája azonos; a korábban elfogadott 48 kHz-es 20 dB-os raw és
RMS-illesztett LP24 WAV is mintánként azonos. Ez a vizsgált fixture-ekre
érvényes, nem az új filter és a kiadott régi filter közötti kompatibilitási ígéret.
A hat célzott Release teszt és a három mód/motor ASan/UBSan teszt sikeres.

A preview eszköz utolsó opcionális argumentuma `LP12|LP24|HP12|BP12`;
elhagyva továbbra is az eredeti LP24 fájlokat készíti. Mindegyik módhoz
12 másodperces, három másodperc bevezetőjű lead minta készült, 50% rezonanciával
és 20 dB 4x Drive-val. Az A az adott mód régi, tiszta filtere, a B az új
Drive+filter. A B RMS-illesztése egyetlen konstans az adott A-hoz, nem a négy
mód egymáshoz igazított érzékelt hangereje; a rezonanciaskálák eltérnek.
A formátumot, véges/clippingmentes mintákat és állandó RMS-gain kontraktust
a preview-regresszió ellenőrzi. Az új módok tulajdonosi hallásos minősítése
pozitív: a három fenti RMS-illesztett mintát a tulajdonos szépen szólónak
hallotta. Ez az 50%-os rezonanciájú, 20 dB Drive-os, 48 kHz-es lead fixture-ek
visszajelzése, nem a magas rátás jelölt vagy a teljes VST3 elfogadása.

Következő kapu: minden móddal összetett gerjesztés, Drive/mód/moduláció,
új CPU-profil és magas rátás faktorpolitika. Ezután latency/state/automation
integráció, factory presetek, natív Windows/macOS elfogadás és kiadás.


## Négy mód összetett gerjesztése és modulációja

A #53 mind a 17 CI-ellenőrzése sikeres lett, beolvadt (`8056431`). A korábbi
összetett gerjesztési grid most mind a négy módra kiterjed: 96 kontroll,
ugyanazokkal a forrásokkal, rátákkal, 50% rezonanciával és 20/24 dB vagy
lépcsős Drive célértékekkel. A korábbi korlátok változatlanok: 1% nyers
jelölt/4x/8x eltérés, 0,5% szűrt jelölt/8x eltérés, 0,1% nyers 4x/8x eltérés.
Mind teljesült. A szűrt relatív RMS-különbség legnagyobb értékei:

| Mód | Rátafüggő jelölt – 8x | 4x – 8x |
| --- | ---: | ---: |
| LP12 | 0,1552% | 0,0076% |
| LP24 | 0,1557% | 0,0076% |
| HP12 | 0,4902% | 0,0243% |
| BP12 | 0,2250% | 0,0112% |

HP12-ben a jelölt közel kerül a diagnosztikai 0,5%-os korláthoz. Ez nem
izolált alias-energia vagy hallásos minőségi százalék; a korábbi magas
szinuszos stresszteszt kompromisszuma megmarad. A 176,4/192 kHz-es 2x
politika nem tekinthető véglegesen elfogadottnak.
[Nyers kontrollok](../experiments/premium_filter/measurements/2026-10-04-all-mode-complex-controls.csv).

A `premium_engine_modulation` és `premium_rate_engine_modulation` 36-36
jelenetet ellenőriz: 48/96/192 kHz, négy induló mód, Poly/Mono/Legato.
Cutoff, 0/50/100% rezonancia és mix, négy mód, 0/20/24 dB Drive váltása
mellett aktív filter envelope/key track, két LFO, wheel/velocity/pressure
routing, pitch bend és sustain működik az 1/16 MIDI-csatornán. Egy hang
pedál alatt enged fel; a végén minden voice befejeződik, majd a reset néma.
A Release paraméter időállandó, ezért 100 ms beállítás után egy teljes
másodpercet engedünk a voice-ok befejezésére.

Véges PreFX, védett végső kimenet, nem néma gerjesztés és release/reset
kontraktus mellett a két offline idővonal mintánként azonos. Az események
ugyanazon abszolút mintán érkeznek, akár egyesével, akár változó
16/32/64/256/2048 mintás csoportokban hívjuk a `ProcessStereo()`-t.
Ez nem VST3-wrapper vagy host automation blokkméret-függetlenségi bizonyítás.
Az öt célzott Release és a három új/kibővített ASan/UBSan teszt sikeres.

## Négy mód teljes motorprofilja

A benchmark `--all-modes` opciója módonként futtatja a korábbi rövid gridet:
864 eset/model, 48/96/192 kHz, 1/8/16 voice, 0/20/24 dB, FX ki/be,
32/64/128/256 mintás csoportok. Az opció nélkül a korábbi LP24 CSV-kontraktus
megmarad. 2048 minta bemelegítés után 4096 mintát mérünk, három ismétlés
mediánjával; ez nem hosszú zenei stresszfutás vagy natív pluginmérés.

16 voice, 20 dB Drive, FX bekapcsolva; a tartomány a négy mód és négy
csoportméret mediánjait fogja át, helyi Release build:

| Ráta | Régi motor | Új, mindig 4x Drive | Új, rátafüggő jelölt |
| --- | ---: | ---: | ---: |
| 48 kHz | 15,30–18,77% | 30,15–32,95% | 30,25–33,14% |
| 96 kHz | 30,64–37,90% | 60,34–66,23% | 60,87–66,62% |
| 192 kHz | 61,70–83,04% | 120,49–131,32% | 81,09–100,41% |

Az érték az audioidő-keret százaléka, nem az operációs rendszer teljes CPU-ja.
Külön, egymás utáni helyi futások; nem kiegyensúlyozott sebességígéret.
A legrosszabb blokkokban jelentős kiugrások voltak mind az új, mind a régi
motorral. Ez nem bizonyítja a kiugrások okát vagy ártalmatlanságát.
192 kHz-en a 4x út eleve túl költséges; a jelölt egyik mediánja is túllépi
az időkeretet. **A realtime CPU-kapu nincs lezárva**, még a 2x jelölttel sem.
Natív Windows/macOS és hosszabb, ellenőrzött terhelésű mérés szükséges.

Nyers profilok:
[régi](../experiments/premium_filter/measurements/2026-10-04-four-mode-legacy-engine.csv),
[4x](../experiments/premium_filter/measurements/2026-10-04-four-mode-4x-engine.csv),
[rátafüggő](../experiments/premium_filter/measurements/2026-10-04-four-mode-rate-engine.csv).
A production filter és az elfogadott hangminták e munkacsomagban nem módosultak.
Következő aktív feladat a CPU-költség és magas rátás minőségpolitika megoldása,
majd latency/state/automation integráció és natív elfogadás.

## Együtthatók újrafelhasználása változatlan vezérlésnél

A #54 mind a 17 ellenőrzése sikeres lett és beolvadt (`b5e3625`). A következő
kutatási optimalizálás a három SVF-együtthatót tárolja. `SnapToTargets()` után,
illetve amikor az eredeti double simítás már pontosan ugyanazokat az értékeket
adja, nem ismétli a három osztást és a változatlan simítást. Új cutoff vagy
rezonancia célérték újraindítja a számítást. Nincs toleranciás korai lezárás,
kvantálás, rövidebb simítás vagy kihagyott szűrőág; a módváltás meleg állapotai
megmaradnak. `Clear()` csak a jeltörténetet törli, az `Init()` és a snap a
megfelelő új együtthatókat is előállítja.

A `ReferencePremiumFilter` a #54 állapotának optimalizálatlan aritmetikája befagyasztott
offline kontrollja. A regresszió 3 923 984 sztereó frame kimenetét ellenőrzi
bájtról bájtra: 8/44,1/48/96/192/384 kHz, négy mód, hosszú beállás utáni új
vezérlés, gyors cutoff/rezonancia/módváltás, snap, clear, más rátán új Init,
másolt állapot és érvénytelen bemenet. A szándékosan elrontott cache-invalidation
kontroll megbukik. Mind a 12 korábbi négy módos preview WAV teljes fájlja
változatlan. Nyolc célzott Release és öt ASan/UBSan teszt sikeres; ez nem a
teljes helyi tesztsor újrafuttatása.

A külön, csak szűrőt mérő Release benchmark 16 sztereó példányt, négy módot,
48/96/192 kHz-et és módonként nyolc váltakozó sorrendű referencia/jelölt párt
használ. 4096 minta bemelegítés után 32768 mintát időzít. Állandó, snapelt
beállításnál a páronkénti időarányok rátánként összesített mediánja
0,878 / 0,872 / 0,888: kb. 11–13% kisebb idő. Cutoff és rezonancia 128 mintánkénti
váltásánál 1,038 / 1,038 / 1,037: kb. 4% többlet a cache állapotkezelése miatt.
A párok szórnak; ez helyi diagnosztika, nem CI-időzítési küszöb vagy natív ígéret.
[Nyers párok](../experiments/premium_filter/measurements/2026-10-04-coefficient-cache.csv).

Ez a lineáris szűrő részleges, változatlan hangú optimalizálása, nem a Drive,
a teljes motor vagy a VST3 költségének ugyanekkora csökkenése. A 192 kHz-es
realtime kapu és a magas rátás Drive minőségpolitikája továbbra is nyitott.
A production hangút, túlmintavételezési faktor, FIR, latency és GUI nem változik.

## Kompakt polyphase Drive-interpoláció — 2026-10-05

A `715cd47` sparse interpolátora már kihagyta a nullával szorzott FIR-tagokat,
de minden oversampled nullát továbbra is beírt a magas rátás ringbe. Az új
interpolátor 64 host-mintás, tükrözött ringet használ: minden bemeneti frame
egyszer kerül be, a négy vagy két fázis ugyanabból a host-történetből olvas.
A változatlan, szimmetrikus FIR-együtthatók fázisonként folytonos táblákba
kerülnek az Init során. A párosítás és a négy részösszeg összeadási sorrendje
megmarad. A decimátor aritmetikája és a gain-simítás nem változik.
Nincs rövidebb FIR, kisebb oversampling faktor, új karakter vagy fast-math.
A realtime út továbbra is fix tárolókat használ, allokáció és zár nélkül.

Helyi Windows x64 / MSVC 19.44 Release objektumméretek:

| Drive | Sparse referencia | Kompakt jelölt | Megtakarítás |
| --- | ---: | ---: | ---: |
| 2x | 16 960 bájt | 11 344 bájt | 5 616 bájt |
| 4x | 17 472 bájt | 12 384 bájt | 5 088 bájt |

A `premium_drive_polyphase_identity` a külön befagyasztott sparse referencia
998 400 sztereó frame-jével hasonlítja össze a 2x és 4x jelöltet bájtról bájtra.
8/44,1/48/96/192/384 kHz, 0..24 dB célváltások, snap, clear, mindkét ring
határain lévő sztereó impulzusok, másolás, más rátás új Init, NaN/Inf és
egyoldali history-reset szerepel benne. A téves nonzero-phase tükrözési index
helyi negatív kontrollja már a 17. összehasonlításnál elbukik.
Kilenc célzott Release teszt sikeres: identity, meglévő alias/latency és
spektrális grid, mindkét teljes motoradapter lifecycle/moduláció, valamint
magas rátás és összetett gerjesztési kontrollok. A korábbi minőségi küszöbök
nem változtak. Lead/pluck/pad, négy mód, 50% rezonancia és 20 dB 4x Drive
mellett a 12 fixture mind a 36 legacy/candidate/RMS-matched WAV-fájlja
SHA-256 szerint azonos az optimalizáció előtti Release renderrel.

Az identity program `--benchmark` módja csak a Drive-ot időzíti, nem CI-kapu.
1024 frame bemelegítés után 16 384 sztereó frame-et mér; 2x/4x,
48/96/192 kHz, 1/16 voice, 0/20/24 dB, 32/256-os offline csoportok és
esetenként hat, váltakozó végrehajtási sorrendű referencia/jelölt pár szerepel.
A bemenet előre elkészített szinusz/koszinusz, az Init és allokáció nincs a
mért szakaszban. A buffer oszlop offline csoportosítást jelent, nem natív
host-callback határidő- vagy legrosszabb blokkpróbát.

16 voice, 20 dB; a két csoportméret 12 páronkénti kompakt/sparse időarányának
mediánja, ugyanazon helyi MSVC Release binárisban:

| Faktor | 48 kHz | 96 kHz | 192 kHz |
| --- | ---: | ---: | ---: |
| 2x | 0,9233 | 0,9282 | 0,9234 |
| 4x | 0,8657 | 0,8715 | 0,8654 |

Ez kb. 7–8%, illetve 13% kisebb izolált Drive-idő. A párok szórnak;
ez helyi diagnosztika, nem teljesmotor- vagy natív sebességígéret.
[Mind a 432 nyers pár](../experiments/premium_filter/measurements/2026-10-05-drive-polyphase.csv).
A korábbi teljesmotor-profilok az optimalizáció előtti állapot mérései;
azokra ezt a százalékot nem alkalmazzuk automatikusan.
Teljes motoros és natív terhelésmérés, magas rátás minőségpolitika, majd
latency/state/automation integráció következik. A realtime CPU-kapu nyitott,
a production engine és a meglévő GUI ebben a kutatási lépésben változatlan.

## Kis pufferes teljesmotor-időeloszlás — 2026-10-05

A #56 kompakt interpoláció main-ba került (`c36f743`). A következő lépés
új mérési mód a meglévő offline motorpróbában; a DSP-kódot nem módosítja.
A `sawstar_premium_engine_benchmark --stress` a rögzített 4x utat,
a `sawstar_premium_rate_benchmark --stress` a rátafüggő kutatási jelöltet,
a `sawstar_premium_engine_benchmark --stress-legacy` a production kontrollt méri.

Mindhárom út: 48/96/192 kHz, 16 kitartott voice, négy filtermód,
20 dB Drive, chorus/delay/reverb, 32/64/128 mintás offline blokkok.
Esetenként 0,25 másodpercnyi audio bemelegítés után 1024 blokkot időzítünk:
medián, nearest-rank p95/p99, maximum és a 100%-os audioidőkeretet túllépő
blokkok száma kerül CSV-be. A fixture igazolja a 16 aktív voice-ot, a hangzó
kimenetet és a véges, védett mintákat; peak és RMS is szerepel.
A százalék falióra-idő / (buffer / sample rate), nem OS CPU-kihasználtság.
A mért idő a mintánkénti biztonsági ellenőrzést és energia/peak-gyűjtést is
tartalmazza; az Init és a bemelegítés nincs benne. Nincs automatikus időzítési kapu.

Windows x64, MSVC 19.44 Release `/O2 /Ob2 /DNDEBUG`, ugyanazon helyi gépen.
A processzor környezeti azonosítója: Intel64 Family 6 Model 158 Stepping 10.
Az útvonalakat egymás után mértük, külön motoros tesztfuttatás nélkül.
A négy mód és három buffer 12 esetének minimum–maximum medián blokkideje:

| Motor | 48 kHz | 96 kHz | 192 kHz |
| --- | ---: | ---: | ---: |
| Production kontroll | 41,7–44,5% | 80,9–85,7% | 161,9–169,2% |
| Prémium, rögzített 4x | 73,9–77,5% | 146,0–153,7% | 291,4–311,6% |
| Rátafüggő kutatási jelölt | 74,0–77,3% | 146,0–163,1% | 222,5–236,3% |

Összesen 108 ellenőrzött CSV-sor, 110 592 időzített blokk. A prémium utak
96/192 kHz-es összes mért blokkja túllépte az időkeretet. A production
kontroll 192 kHz-en szintén minden blokkban túllépett; 96 kHz-en 210/12 288
blokk lépte túl. A 48 kHz-es 4x út 28, a rátafüggő út 169 túllépést adott
12 288 blokkból; ezek az azonos 4x faktor mellett külön futások szórását is
mutatják, nem eltérő algoritmus bizonyítékai. A p99 és maximum adatok a
nyers fájlokban vannak, az összefoglaló táblázat nem rejti el őket.

- [Rögzített 4x](../experiments/premium_filter/measurements/2026-10-05-engine-stress-premium.csv)
- [Rátafüggő jelölt](../experiments/premium_filter/measurements/2026-10-05-engine-stress-rate-scaled.csv)
- [Production kontroll](../experiments/premium_filter/measurements/2026-10-05-engine-stress-legacy.csv)

Négy helyi Release lifecycle/modulációs regresszió sikeres mindkét adapterrel.
A CSV-k teljes esetkészlete, véges és rendezett percentilisei, számlálói és
hangzó/védett kimenete külön ellenőrzött. A production kontroll más filtert
használ, ezért ez nem hangazonos gyorsulásmérés. A korábbi rövid profilok más
mérési körét és platformját nem tekintjük közvetlen előtte/utána párnak.

Ez egy statikus beállítású, rövid offline diagnosztika. Nem VST3 callback,
nem natív REAPER-próba, nem általános Windows/macOS teljesítményígéret,
és nem hosszú modulációs vagy ütemezési stresszteszt. A magas költség és a
production kontroll túllépése miatt a CPU-kapu ezzel a méréssel nem zárható le.
Következő feladat a Drive/FIR és a teljes motor költségének bontása, ismételt
mérés kontrollált célgépen, a magas rátás minőségpolitika és natív hostpróba.
Latency/state/automation integráció csak a megfelelő kapuk után következhet.

## Izolált komponensek és teljesmotor-kontroll — 2026-10-05

A #58 időeloszlás-próba 17/17 sikeres ellenőrzés után main-ba került
(`54ca373`). A benchmark új `--components` módja külön méri a 16 példányos
Drive-ot, a PremiumLowPass-t, a teljes filteradaptert, valamint a kutatási
és production motorokat FX nélkül és FX-szel. Mindkét meglévő benchmark
target támogatja; a DSP-implementáció és a production jelút változatlan.

48/96/192 kHz, négy mód, 16 voice/példány, 20 dB Drive konfiguráció;
esetenként három külön inicializált ismétlés, 4096 frame bemelegítés és
16 384 időzített frame. A kernelbemenet előre számolt, 0,4 amplitúdójú
440 Hz szinusz / 660 Hz koszinusz. A motorbemenet a korábbi Setup szerinti
OSC1/OSC2/SUB/noise és 16 kitartott MIDI-hang; FX esetén chorus/delay/reverb.
Az izolált kernelek snapelt célértékekkel indulnak, a motor a megszokott
parameter/reset útját használja. A filter sorban a drive_db=20 csak a
vizsgált konfigurációt azonosítja: ez a kernel nem tartalmaz Drive-ot.
A Drive-nak nincs filtermódja; a négy mód sorai itt ismételt kontrollok.

A komponenseket külön futtatjuk, per-sample mérőóra nélkül. Az Init,
allokáció, bemenet-előkészítés és bemelegítés nincs az időzítésben; a kimenet
energiájának összegzése benne van, és véges, pozitív energia szükséges.
Az alábbi értékek a négy mód × három ismétlés 12 audioidő-százalékának mediánjai.
Ugyanaz a helyi Windows x64 / MSVC 19.44 `/O2 /Ob2 /DNDEBUG` gép,
mint az előző stresszpróbánál; a két út sorban futott, motoros tesztek nélkül.

| Kernel / motor | 4x: 48 kHz | 4x: 96 kHz | 4x: 192 kHz | Rátafüggő: 48 kHz | Rátafüggő: 96 kHz | Rátafüggő: 192 kHz |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Drive | 39,8% | 78,4% | 156,2% | 38,8% | 79,1% | 86,7% |
| Prémium szűrő | 3,4% | 6,7% | 13,2% | 3,4% | 6,9% | 14,0% |
| Teljes filteradapter | 44,1% | 86,3% | 172,4% | 43,2% | 86,8% | 108,8% |
| Kutatási motor, FX nélkül | 75,9% | 147,8% | 289,1% | 74,3% | 148,2% | 228,4% |
| Kutatási motor, FX-szel | 76,7% | 149,8% | 293,3% | 75,7% | 149,4% | 233,3% |
| Production kontroll, FX nélkül | 41,3% | 80,3% | 157,1% | 40,3% | 78,2% | 160,9% |
| Production kontroll, FX-szel | 43,0% | 84,1% | 161,8% | 41,8% | 86,0% | 169,9% |

- [4x nyers ismétlések](../experiments/premium_filter/measurements/2026-10-05-components-premium.csv)
- [Rátafüggő nyers ismétlések](../experiments/premium_filter/measurements/2026-10-05-components-rate-scaled.csv)

504 ellenőrzött sor, összesen 8 257 536 időzített host-frame. Mindkét fájl
teljes esetkészlete, ismétlésenkénti véges/pozitív idő és energia, valamint
az időből számolt audioidő-százalék ellenőrzött. Az ismétlések energiái
azonosak. A két út 48/96 kHz-en azonos energia-kontrollt ad minden stage-ben;
a szűrő és a production kontroll 192 kHz-en is azonos. Ez összesített energia,
nem új sample-by-sample azonossági vagy spektrális bizonyíték.
Négy helyi Release lifecycle/modulációs regresszió mindkét adapterrel sikeres.

A külön kernelidők nem összeadható motor-részarányok: más a bemenet, a
cache-környezet, a simítás indulása és az energia-gyűjtés költsége. A két
út időzítése nem váltakozó páros mérés; az alsó két rátán ugyanaz a 4x
algoritmus fut, eltérő időik futásszórást jeleznek. A 192 kHz-es Drive
különbsége 2x/4x minőség/költség összevetés, nem változatlan hangú optimalizálás.
Nincs natív host, automatizálási stressz vagy CI-időzítési küszöb ebben a módban.

A mért izolált Drive költsége lényegesen nagyobb a lineáris szűrőénél;
következő fókusz a FIR/interpoláció/decimáció és nemlinearitás elkülönített
vizsgálata. A teljes motor fennmaradó szintézisköltsége sem hagyható figyelmen
kívül. A CPU-kapu, a magas rátás minőségpolitika és a natív REAPER-próba
továbbra is nyitott; ezek lezárása előtt nincs production filtercsere.

## Fordításkori Drive-fázisválasztás — 2026-10-05

A #59 komponensprofil 17/17 sikeres ellenőrzés után beolvadt (`d4c7822`).
A következő kutatási optimalizálás a rögzített 2x/4x interpolációs fázisokat
template-paraméterként adja át. A szimmetrikus/általános interpolációs ág és
a decimátor kimenetszámításának választása `if constexpr` alapján történik.
Minden fázis az eredeti sorrendben lefut; a nulladik után is beírjuk az
összes többi shaped mintát a decimátorba. A ring, az együtthatók, a gain-simítás,
a tanh és a lebegőpontos összeadási sorrend változatlan. Nincs új állapottag,
allokáció, lock, rövidebb FIR, kisebb faktor, fast-math vagy új hangkarakter.
A 32 host-frame latency és a production jelút megmarad.

A `tests/fixtures/premium_drive_runtime_phase_reference.h` a `d4c7822`
kompakt, futásidős fázisválasztásának külön névtérben befagyasztott kontrollja.
A meglévő polyphase regresszió most ezzel és a korábbi `715cd47` sparse
referenciával is összeveti a jelöltet: összesen 1 996 800 sztereó frame
bitazonos. A két faktor, hat ráta, targetváltás, snap/clear, ring-határok,
állapotmásolás, reinit és hibás bemenet ellenőrzése megmarad. A szándékosan
megismételt nulladik fázist a negatív kontroll már a harmadik összehasonlításnál
elutasítja. Kilenc célzott helyi MSVC Release regresszió sikeres, és mind a
36 WAV a 12 eddigi lead/pluck/pad × filtermód fixture-ben bájtról bájtra azonos.

Új `sawstar_premium_polyphase_tests --phase-benchmark` mód a befagyasztott
kompakt runtime és a statikus jelölt ugyanabban a binárisban futó párait méri.
1024 frame bemelegítés, 16 384 frame időzítés; 2x/4x, 48/96/192 kHz,
1/16 voice, 0/20/24 dB, 32/256 mintás offline csoportok, hat pár
váltakozó futási sorrendben. A korábbi `--benchmark` továbbra is a sparse
kontrollt használja. Mind a 432 új sor egyedi esete és véges/pozitív ideje
ellenőrzött. Nincs CI-időzítési küszöb.

16 voice / 20 dB: a két csoportméret 12 páronkénti statikus/runtime
időarányának mediánja ezen a helyi Windows x64 / MSVC 19.44 Release gépen:

| Faktor | 48 kHz | 96 kHz | 192 kHz |
| --- | ---: | ---: | ---: |
| 2x | 0,9690 | 0,9766 | 0,9780 |
| 4x | 0,9833 | 0,9816 | 0,9769 |

Kb. 2–3% kisebb izolált Drive-idő. A 16 voice / 20 dB 2x 36 párja mind
kisebb jelöltidőt adott; a 4x 36 párjából 35. Az egyvoice-os és egyes más
beállítású párok jobban szórnak, és egyedi lassulások is vannak. Ez kis helyi
előny, nem teljesmotor-gyorsulás vagy platformfüggetlen teljesítményígéret.
[Mind a 432 nyers pár](../experiments/premium_filter/measurements/2026-10-05-static-phase.csv).

Új teljesmotoros és natív CPU-próba ebben a körben nem készült; az eddigi
határidő-túllépésekből ez a kis javulás nem csinál elfogadott realtime motort.
A következő Drive-feladat a FIR/interpoláció/decimáció és a nemlinearitás
szélesebb költségbontása, majd kontrollált célgépes ismétlés. A CPU-kapu,
a magas rátás minőségpolitika és a latency/state/automation integráció nyitott.


## Drive FIR és nemlinearitás költségkontroll — 2026-10-05

A #60 main (`3179223`) Windows, macOS és Code quality futása sikeres. Új
offline költségkontroll ugyanazt a 2x/4x interpolációt, FIR-decimációt,
tárolókat és 32 mintás késést futtatja a tanh/gain telítés nélkül. A külön
`Nonlinear=false` template csak a mérőtargetben szerepel: nem választható
filter vagy production policy. A normál alapértelmezett út változatlan.

Nulla Drive mellett a kontroll 2x/4x és öt rátán mintánként bitazonos a normál
úttal; clear után néma. 20 dB mellett szándékosan eltérő hangot ad, mindkét
út véges. A normál Drive két korábbi referenciával végzett 1 996 800 frame-es
azonossági regressziója megmarad. Öt célzott Release és két ASan/UBSan teszt
sikeres; a két motor modulációja és az összetett gerjesztési kontroll is teljesül.

A Release benchmark 16 sztereó példányt, 2x/4x, 48/96/192 kHz, 20/24 dB,
1024 mintás bemelegítést és 16 384 időzített mintát használ. Előre számolt
0,4 amplitúdójú 440 Hz szinusz / 660 Hz koszinusz; hat váltakozó sorrendű
pár konfigurációnként, összesen 72 ellenőrzött sor. Init és allokáció nincs
a mérésben, a kimeneti ellenőrző összeg benne van. Helyi macOS Release mérés:

| Faktor | 48 kHz FIR-only/full | 96 kHz | 192 kHz |
| --- | ---: | ---: | ---: |
| 2x | 0,5230 | 0,5244 | 0,5247 |
| 4x | 0,5560 | 0,5571 | 0,5612 |

Az érték a 20/24 dB tizenkét páronkénti időarányának mediánja. A telítés
kikapcsolása megváltoztatja a gerjesztést, a compiler optimalizációit és
a kód elrendezését is: a különbség nem izolált libm-tanh idő vagy összeadható
CPU-részarány. A kontroll nem javított hangút, nem teljes motoros gyorsulás
és nem minőségbizonyíték. A FIR-only mért költség is jelentős marad.
[Nyers párok](../experiments/premium_filter/measurements/2026-10-05-drive-cost-control.csv).

Következő kutatási lépés a nemlinearitás gyorsabb számítási lehetőségeinek
külön összevetése a változatlan referenciával: numerikus hiba, spektrális
maradék, kis/erős bemenet, Drive-simítás, sztereó izoláció és CPU. Közelítés
nem kerül automatikusan a normál útba. Utána teljes motoros és natív mérés;
a CPU-kapu, magas rátás minőségpolitika és végleges integráció nyitott.


## Skaláris telítés számítási jelöltje — 2026-10-05

A #61 minden 17 ellenőrzése sikeres lett és beolvadt (`262177f`). A külön
`ResearchTanh` a tanh algebrai alakját egy exp hívással számolja: az abszolút
bemenethez q=exp(-2a), majd (1-q)/(1+q) előjelhelyesen. A nagyon kis,
1e-5 alatti tartományban x(1-x²/3) elkerüli a kivonás pontosságvesztését;
20-tól a double telítési kimenet ±1. Ez saját kutatási függvény, nem
a PremiumDrive új alapértelmezése. A production és a normál kutatási Drive
sem hivatkozik rá, nem változik karakter, FIR, faktor vagy latency.

A skaláris regresszió 400 001 egyenletes pontot vizsgál -20..20 között,
két előjellel a kettőhatványokat a legkisebb subnormálistól 4096-ig, a két
ágküszöb szomszédos double értékeit, signed zero, NaN és ±infinity eseteket.
A std::tanh-hoz képesti abszolút hibakorlát 5e-15; helyben a maximum
1,11022e-16 volt. A rácson monoton, páratlan és [-1,1] tartományú kimenet
szükséges. Ez nem minden double értékre kiterjedő formális pontosságbizonyítás.
A Release és az ASan/UBSan skaláris regresszió sikeres.

A külön mikromérés 65 536 előre számolt, 0,4*sin(n*0,057) bemenetet
szoroz 0/12/20/24 dB gainnel, 16 ismétlésben értékeli a függvényeket.
Bemelegítés után nyolc váltakozó sorrendű pár/gain, 32 nyers sor. Helyi
macOS Release research/std::tanh időarány-mediánok: 0,815 / 0,765 / 0,718 /
0,724. Ez csak a skaláris függvények és ellenőrzőösszeg ideje; a normál
Drive 0 dB-en eleve kihagyja a tanh-t, ezért annak nincs ígért gyorsulása.
A mérés nem oversampled jeltörténet, nem FIR vagy teljes motor, nem natív
host, és más fordító/platform más eredményt adhat. Nincs CI-időzítési küszöb.
[Nyers párok](../experiments/premium_filter/measurements/2026-10-05-tanh-scalar.csv).

Következik a külön, csak kutatási oversampled Drive-változat numerikus és
spektrális összevetése: erős/kis/összetett gerjesztés, Drive-simítás, több
ráta, 2x/4x, sztereó izoláció, latency és teljes motoros CPU. Skaláris
azonosság közeli hiba önmagában nem minősíti a teljes szűrt hangot; a
normál út cseréje és a CPU/minőség kapuk lezárása még nem indokolt.


## Telítésjelölt Drive és motor mérföldköve — 2026-10-05

A #62 zöld ellenőrzésekkel mainbe került (`c53d025`). A skaláris jelölt
most külön, explicit kutatási template-változatban fut a változatlan 2x/4x
FIR-en keresztül. A normál FixedRatePremiumDrive alapértelmezése továbbra
is std::tanh; a production DSP és az elfogadott preview-generátor nem vált át.
A külön engine-probe saját névteret és TanhStudyEnginePremiumFilter adaptert
használ: nincs eltérő Synth-layout közös szimbólumokkal vagy production link.

A Drive-kontroll 72 eset: 2x/4x, hat ráta 8..384 kHz, 1e-6/0,1/0,75
bemeneti amplitúdó és állandó 20 dB vagy 0/12/24 dB célváltás. Esetenként
8192 sztereó frame, két eltérő csatorna, clear, másolt állapot továbbfutása,
NaN csatorna-helyreállítás és reinit. A rögzített diagnosztikai korlátok:
2e-7 abszolút és relatív RMS eltérés; hat kiválasztott DFT-bin eltérésének
normalizált amplitúdója legfeljebb 2e-8. A helyi float kimeneten mindhárom
mérőszám nulla volt. Ez nem minden bemenetre bitazonossági bizonyíték,
sem nulla alias-energia: a DFT az eltérést, nem a teljes aliasingot vizsgálja.
A szándékosan hibás 0,99-es gain kontroll megbukik a waveform teszten.
[Nyers eltérések](../experiments/premium_filter/measurements/2026-10-05-drive-tanh-errors.csv).

A teljes offline 4x motor referencia/jelölt összevetése 12 eset: 48/96/192 kHz,
négy filtermód, 16 kitartott voice, OSC1/OSC2/SUB/noise, chorus/delay/reverb,
0/12/24 dB célváltás, 8192 frame és reset-csend. A 2e-6 abszolút kimeneti
korlát teljesül, helyi maximum nulla. Ez Poly-fixture, nem teljes Mono/Legato,
ARP, host automation, latency/state vagy natív wrapper-elfogadás. Három
célzott Release és két új ASan/UBSan regresszió sikeres; a normál Drive
korábbi két referenciával végzett azonossági tesztje is megmarad.

A Drive-benchmark 16 példány, 20 dB, 2x/4x, 48/96/192 kHz, 1024 frame
bemelegítés és 16 384 időzített frame; nyolc váltakozó pár/rátánként,
48 sor. Helyi macOS Release study/reference mediánok:

| Út | 48 kHz | 96 kHz | 192 kHz |
| --- | ---: | ---: | ---: |
| Drive 2x | 0,9169 | 0,9237 | 0,9224 |
| Drive 4x | 0,9348 | 0,9321 | 0,9339 |
| Teljes 4x motor FX-szel | 0,9431 | 0,9455 | 0,9448 |

A motorprofil 16 voice, 20 dB, négy mód, 2048 frame bemelegítés után
4096 időzített frame, négy váltakozó pár/mód/rátánként, 48 sor. A motor
értékei a négy mód 16 páronkénti arányának mediánjai. Init és allokáció
nincs időzítve; az ellenőrzőösszeg igen. Kb. 7–8% Drive és 5–6% motor
időcsökkenés ezen a gépen; nem platformfüggetlen vagy realtime ígéret.
Nincs blokk-percentilis, natív host vagy hosszú stresszfutás ebben a körben.
[Drive-párok](../experiments/premium_filter/measurements/2026-10-05-drive-tanh-cost.csv),
[motor-párok](../experiments/premium_filter/measurements/2026-10-05-tanh-engine-cost.csv).

A mérföldkő a külön jelölt helyi numerikus és CPU-minősítése, nem kész
filtercsere. Következik Windows/macOS CI és célgépes ismétlés, szélesebb
modulációs és spektrális gerjesztés, kis-bufferes deadline-próba. Csak ezek
alapján dönthető el a normál kutatási út átváltása; a CPU-kapu, magas rátás
policy, végleges latency/state/automation integráció és natív elfogadás nyitott.



## Telítésjelölt modulációs és teljes binspektrum-kontroll — 2026-10-06

A meglévő 72 Drive-fixture mellé 288 új eset kerül ugyanabba a CTest
targetbe: 2x/4x, hat mintavételi frekvencia, 1e-8 / 0,25 / 4 amplitúdó,
négy jel és állandó 24 dB vagy mintánként szinuszos 0..24 dB target.
A gerjesztések magas koherens szinusz, chirp, rögzített seedű xorshift zaj,
valamint bipoláris impulzussor. 4096 sztereó frame/eset, clear a 777. mintán;
a modulált eset 257 mintánként snapet is kap.

A rögzített waveform-korlátok 2e-7 abszolút és relatív RMS. Az utolsó
2048 frame két csatornájának **eltérését** radix-2 FFT vizsgálja, minden
egyoldalas binben, DC és Nyquist végpontokkal: legfeljebb 2e-8 normalizált
amplitúdó. Nincs ablak: a teljes véges rekord eltérését mérjük, nem egy
hang torzításának alias-energiáját. A chirp és zaj spektrális szivárgása is
benne van. Ez szélesebb véges fixture, továbbra sem abszolút aliasing- vagy
hallásos minősítés. Külön ismert 1e-4 amplitúdójú DC, koherens koszinuszok
és Nyquist kontroll ellenőrzi a normalizálást és meghaladja a korlátot;
a nulla kontroll pontosan nulla.

Élő historyból való független másolat 129 frame továbbfutásban bitazonos
a saját eredetijével. NaN/infinity a bal csatornán nem változtathatja meg
a felmelegített jobb csatornát. Clear után néma kimenet kell. Nulla Drive
mellett a két út impulzusválasza azonos, a csúcs 32 hostmintánál van,
a másik csatorna néma. A default Drive és production DSP továbbra sem vált át.

A main auditjavításait a PR új fejlesztési commitja is tartalmazza.
A Windows parancsfuttató környezeti hibája miatt helyi fordítás és új
CPU-mérés nincs; a friss Windows/macOS és sanitizer CI eredményét a PR
rögzíti. Időzítési küszöböt nem vezettünk be. Célgépes páros ismétlés,
kis-bufferes deadline-eloszlás, minőségpolicy és natív integráció nyitott.


## Páros teljesmotor-blokkidő eloszlás — 2026-10-06

A `sawstar_premium_tanh_deadline` külön research target. Argumentum nélkül
a fixture és statisztikák funkcionális CTest ellenőrzése fut: medián páros/páratlan
esetre, nearest-rank p95/p99, szigorú 100% fölötti számláló, hibás/üres időadat
elutasítása, valamint 12 szóló 16-voice motorfixture referencia/jelölt eltérése
legfeljebb 2e-6. A std::tanh és ResearchTanh alapértelmezett hangút nem vált át.

A `--deadline output-directory` mérés 4x/4x összevetés, nem a 2x/4x magas
rátás policy véglegesítése. 48/96/192 kHz, négy filtermód, 16 Poly voice,
20 dB, FX, 32/64/128 frame/csoport, négy váltakozó sorrendű pár.
Mindkét út minden párban új motort indít, 0,25 másodperc warmup után 256
blokkot időzít. Az allokáció, Init, warmup, jelvédelem-ellenőrzés és
diagnosztikai számtan nincs a blokkidőben; ProcessStereo és bufferírás igen.
Nincs VST3 callback, host scheduler, MIDI/automatizálási terhelés vagy
hosszú steady-state FX-bemelegítés. A korábbi motorprofilhoz képest itt
explicit natív egységű FX-beállítások szerepelnek: delay tone 2000 Hz,
reverb decay 2 s, damping 6000 Hz; a régi adatsor közvetlenül nem összevethető.

A `summary.csv` 288 sorában az audioidő százalékában szerepel a blokkok
mediánja, nearest-rank p95/p99, maximum és 100% fölötti blokkok száma,
plusz peak/RMS/checksum. A `blocks.csv` minden 73 728 blokkot megőrzi,
külön engine/rate/mode/buffer/pair/block kulccsal. A Python riport minden
kulcsot, darabszámot, futási sorrendet és a nyers adatokból újraszámolt
percentilis/számláló értéket ellenőriz, majd kilenc ráta/puffer kombinációban
16 páros jelenetarány mediánját számolja. Ez nem összevont blokkpercentilis.
A 256 blokkos p99 kevés felső-tail megfigyelésre támaszkodik; négy pár
feltáró mérés, nem megbízhatósági vagy hallásos elfogadási bizonyítás.

Külön `premium-deadline.yml` PR/manual workflow Windows x64 és macOS ARM64
Release mérésben a pontos PR-headet építi. A runner platformja, forrás-SHA,
compiler-leírás, nyers CSV és riport artifactban marad 90 napig; az összesítő
és riport a joblogban is olvasható. Az eredmény a PR-ben rögzítendő.
Nincs CPU/időzítési pass/fail küszöb. Közös CI-gép wall-clock szórása és
eltérő compiler miatt natív célgépes ismétlés nélkül nincs általános CPU-ígéret.
A CPU-kapu, magas rátás policy és végleges filterintegráció továbbra is nyitott.
