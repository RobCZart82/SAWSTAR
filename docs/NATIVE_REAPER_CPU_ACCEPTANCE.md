# Natív REAPER CPU- és dropout-elfogadási jegyzőkönyv

Állapot: **előkészített eljárás, formális natív elfogadás nincs**.
A korábbi pozitív hangkarakter-/hangerő-visszajelzés és REAPER CPU-pillanatképek
megmaradnak, de nem töltik ki automatikusan ezt a jegyzőkönyvet.

Ez a dokumentum a végrehajtási adatlap. A kiadásig vezető sorrendet,
a teljes C/V/G/F tesztmátrixot és a döntési feltételeket a
[release fejlesztési terv](NEXT_RELEASE_DEVELOPMENT_PLAN.md) tartalmazza.
A terv natív tartalékcélja futás előtt rögzítendő javaslat, nem meglévő
mérési eredmény vagy az offline CPU-küszöb átírása.

## macOS alappróba — visszajelzés 2026-10-27

Robert mindkét telepített Reference/Candidate plugin működését és hangját
hibamentesnek jelezte. [Pontos hatókör, forrás és telepített hashok](NATIVE_HOST_SMOKE_2026_10_27.md).
Ez sikeres felhasználói alappróba; a 28 eset / 112 ablak CPU-/dropout-mérése,
a külön GUI/state/preset-próbák és a Windows-elfogadás még nyitottak.
Következő: a C 48 kHz /128 AUTO négy módjának rögzített AB/BA előszűrése
**(sajátgépes reaper)**. K1 és a production/RC/publikálási kapuk változatlanok.

## Kiegészítő hibajavítási próbák — 2026-10-20

Az [audit javítási terve](BUGFIX_DEVELOPMENT_PLAN_2026_10_20.md) B1/R1/B2/Q1
feladatai az érintett buildhez külön kapcsolódó próbák. Nem módosítják a
meglévő 28 CPU-eset / 112 mérési ablak számát, és jelenleg nincs natív PASS.

| ID | Kiegészítő vizsgálat | Rögzítendő eredmény |
| --- | --- | --- |
| B1 | Mixer/CC1/pressure nullára simítása, hosszú kitartás, javítás utáni átmenetek | Numerikus regresszió külön; natív CPU/dropout/FP-policy csak tényleges mérésből. |
| R1 | A példány importál, B ment/átnevez/töröl; azonos modul/hostfolyamat, azonos és eltérő mappa | Batchméret, háttértár, bridging mód, GUI-várakozás és audio/dropout külön; nincs előre feltételezett akadás. |
| B2 | SUB magas frekvencia/bend a kiválasztott policy szerint | SR, waveform, cél és mért frekvencia, korlátozás; normál basszusregresszió. |
| Q1 | Végleges production RC MIDI/editor/state/preset/lifecycle/render/installer próbái | Pontos bináris/csomaghash és esetenkénti PASS/FAIL/nem tesztelt; a régi RC eredménye nem másolható át. |

A K1 Rosetta-biteltérés külön numerikus elfogadási blokkoló az audit tervében;
CI-siker vagy natív halláspróba nem helyettesíti annak okfeltárását.

## Mit mérünk?

A cél a tényleges plugin/host működés és hallható kimaradások ellenőrzése.
A Saw-dispatch jelölt külön **SAWSTAR Premium Candidate** VST3-ként épül.
A **SAWSTAR Premium Preview** a Reference; a meglévő Preview önálló tesztje
nem igazolja a külön Candidate natív teljesítményét.
Mindig a tesztelt bináris pontos azonosságát és szerepét írjuk le.
A végleges production RC-t integráció után újra, önállóan el kell fogadni.

## Futtatásonként rögzítendő adatok

- Dátum, géptípus/CPU/RAM/OS, táp- és energiabeállítás, ismert háttérterhelés.
- REAPER pontos verziója és architektúrája; plugin formátum/név/architektúra,
  build forráscommitja, letöltési/build-eredete és a telepített bináris SHA-256-a.
- Audioeszköz, driver/CoreAudio vagy ASIO, sample rate és tényleges puffer.
- REAPER anticipative FX, FX multiprocessing és live-monitoring beállítások;
  GUI nyitott/zárt, többi FX és sáv állapota. Ezek ne változzanak az A/B között.
- A tesztprojekt, preset/paraméterek, MIDI és cutoff-automatizálás mentett példánya.
- Pontosan melyik REAPER-kijelző: sáv/FX CPU%, teljes CPU vagy RT CPU.
  Különböző kijelzők és a plugin saját DSP%-a nem kerülhet közös oszlopba.

Azonos kontrollnál ugyanaz a MIDI és automatizálás, azonos megszólaló hangszám,
filtermód/Drive/rezonancia/FX és kimeneti beállítás szükséges. Egyszerre egy
vizsgált instrument szóljon; a másik legyen offline, ne pusztán némítva.
A két egyidejű instrument külön rendszerterhelési próba lehet.
A korábban elfogadott 0,44 dB RMS-különbséget nem korrigáljuk újabb globális gainnel.

## A célgépes bizonyíték alapján kijelölt sorrend

A [ráta-/pufferpolicy előkészítése](RATE_BUFFER_ACCEPTANCE_POLICY.md) és a
[36 soros összesítő](measurements/target-rate-readiness-2026-10-17.md) mindkét
kört és a rosszabb eredményeket is megtartja. Ezek nem kitöltött natív
jegyzőkönyvek. A 48 kHz / 128, majd /64 sorrend megmarad; a 32 minta és
96/192 kHz külön kockázati/minőségi kapu. A pluginban ettől nem lép életbe
új korlátozás vagy automatikus minőségváltás.

## Első mérési sorozat

1. Rögzített kezdeti tartomány: 48 kHz / 128, majd 48 kHz / 64 minta.
   A teszt leírása nem állítja, hogy más tartomány már támogatott/elfogadott.
2. A release terv C/V/G/F mátrixa szerint 1/8/16 hang, mind a négy filtermód,
   statikus és automatizált cutoff, GUI- és FX-kontroll. A 28 kijelölt eset
   nem minden változó teljes keresztszorzata; az ott megadott fedettséget
   tartjuk meg. A hangok/release-ek átfedését is jegyezd fel.
3. Rögzített 10 másodperc bemelegítés után 60 másodperc mérési ablak minden
   esetben; egyszer A→B, egyszer B→A sorrend, mindkét eredmény megtartásával.
4. GUI nyitva és zárva külön kontroll; végül stop után a teljes tail kifutása
   és aktív hang nélküli idle. A némaság önmagában nem bizonyítja a DSP leállását.
5. Rögzítsd a REAPER átlag/maximum értékeit, ha a használt kijelző támogatja;
   szemmel olvasott tartományt csak megfigyelésnek jelölj. Ebből ne számolj p99-et.
6. Dropout/xrun jelzés, hallható kattogás/kimaradás, rendellenes CPU-csúcs,
   plugin/host hiba és clipping külön oszlopba kerül. A host számlálójának
   induló/végértékét is őrizd meg, ha elérhető. A képernyőkép kiegészítő bizonyíték.
7. A 96/192 kHz és 32 mintás terhelések a magasráta-/kis-pufferpolicy vizsgálatához
   külön sorok. Az offline negatív cellákat nem hagyjuk el a kedvező átlag miatt.

Az A/B nem általános piaci szintirangsor, a két sorrend sem kedvező eredmény
keresése. Minden előre kijelölt eset és ismétlés megmarad; hibánál nincs
automatikus ismétlés. A CPU-méréshez szükséges headroomot mindkét oldalon azonos,
rögzített beállítással biztosítsd, ne menet közben állíts hangerőt vagy limitert.

## Teszteset-azonosítás és ablakonkénti adatok

A teljes mátrix és időbecslés a release terv 4.3. pontjában található.
Példa azonosítók: `C-128-LP12-STATIC`, `C-64-BP12-AUTO`,
`V-128-8-HP12`, `G-64-LP24-OPEN`, `F-128-P1`.
Minden azonosítóhoz A→B és B→A sorrend, összesen négy külön megfigyelési
ablak tartozik; a buildazonosság és az ablak kezdete/vége is rögzítendő.

- CPU-átlag/maximum csak ha a mérő biztosítja; egyébként megfigyelt tartomány.
- RT-adat külön oszlop, az FX CPU%-ból nem számolunk tartalékot.
- Xrun induló/végérték, vagy `N/A – nem hozzáférhető`; az N/A nem nulla.
- Hanghiba, clipping, crash és state/GUI-hiba külön, időponttal és bizonyítékkal.
- Döntés: `PASS`, `nyitott` vagy `blokkolt`, pontos scope-pal és indokkal.

A fő A/B összevetés, a kétpéldányos tartalékpróba és az idle/tail külön
adatsor. Nincs automatikus újrapróbálás vagy a hibás ablak törlése.

## Kitöltendő eredménytábla

A [macOS mérési csomag előkészítője](NATIVE_HOST_KIT.md) ellenőrzi a két ZIP
azonosságát és üres, 112 ablakos adatlapot készít. Ez nem natív mérés vagy PASS.

Az alábbi üres sorok nem mérési eredmények; szükség szerint minden esetre másold őket.

| Gép/build | Ráta/puffer | Hang/mód/terhelés | Sorrend/GUI | REAPER mérő és időablak | CPU megfigyelés | Xrun induló→vég | Hallható hiba/clipping | Bizonyíték |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| nincs mérve | — | — | — | — | — | — | — | — |

## Elfogadási döntés

A tesztelt és elfogadott működési tartományt Windows/macOS külön kell megadni,
a hibás/nem tesztelt tartományokkal együtt. A kijelölt elfogadási esetekben
nem maradhat megmagyarázatlan dropout, instabilitás vagy hibás audio; a CPU-
tartalékot a célgépen dokumentálni kell. A fennmaradó p99/túllépések nem
tűnnek el attól, hogy az átlag alacsony. Bizonyíték nélkül nincs általános
`native_host_acceptance=true` vagy production-promóció.

CPU-elfogadás után is külön feladat a filterminőség/rátapolicy, production-
integráció, GUI-state, preset/lifecycle és végső RC telepítési/host-ellenőrzés.

