# Natív REAPER CPU- és dropout-elfogadási jegyzőkönyv

Állapot: **előkészített eljárás, formális natív elfogadás nincs**.
A korábbi pozitív hangkarakter-/hangerő-visszajelzés és REAPER CPU-pillanatképek
megmaradnak, de nem töltik ki automatikusan ezt a jegyzőkönyvet.

## Mit mérünk?

A cél a tényleges plugin/host működés és hallható kimaradások ellenőrzése.
Az offline oszcillátor-dispatch jelölt **nincs ettől beépítve** a Preview-ba:
a meglévő Preview tesztje nem igazolja automatikusan a kutatási jelölt natív
teljesítményét. Mindig a tesztelt bináris pontos azonosságát és szerepét írjuk le.
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

## Első mérési sorozat

1. Rögzített kezdeti tartomány: 48 kHz / 128, majd 48 kHz / 64 minta.
   A teszt leírása nem állítja, hogy más tartomány már támogatott/elfogadott.
2. Azonos MIDI-menettel 1, 8 és 16 hang, statikus és automatizált cutoff;
   a négy filtermód külön kerüljön sorra. A hangok/release-ek átfedését is jegyezd fel.
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

## Kitöltendő eredménytábla

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
