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
a cutoff karaktere és a pad teltsége vizsgálandó. A közepes rezonancia
szélesebb elfogadása után következik a Drive külön minőségi prototípusa.
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
