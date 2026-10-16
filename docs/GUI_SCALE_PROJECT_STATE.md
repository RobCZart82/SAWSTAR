# GUI-méret mentése DAW-projektbe

## Állapot — 2026-10-16

**Javítás implementálva; natív Windows/macOS REAPER-elfogadás még nyitott.**
A natív macOS REAPER-próba során a Classic SAWSTAR és a külön
SAWSTAR Premium Preview példány egyaránt elvesztette a 75%-os GUI-méretet
a projekt újranyitásakor. Ez használhatósági/state-kezelési feladat;
önmagában nem jelent CPU-, hangminőségi vagy release-elfogadást.

## Reprodukció és megfigyelés

1. Egy REAPER-projektben legyen egy Classic és egy Premium Preview példány.
2. Nyissuk meg és tűzzük ki mindkét pluginablakot, majd a SAWSTAR
   fogaskerék menüjében állítsuk a GUI méretét 75%-ra.
3. Mentsük a projektet, zárjuk be, majd nyissuk meg újra.
4. Felhasználói megfigyelés: mindkét GUI 100%-os mérettel nyílik meg,
   a mentés előtti 75% helyett.

Bejelentés: Robert, 2026-10-15, macOS / REAPER.
A mellékelt képernyőkép időpontja: 11:45:55.
A külön Premium Preview vizsgált forrása:
`f63e5ab89a237eb4a0fcad439483ebdaf1ab39c9`.
A Classic telepített binárisának pontos forrásazonosítója ebben
a bejelentésben nincs külön igazolva.

## Forráskódban azonosított ok

A bejelentéskori `src/plugin/SAWSTAR.h` fájl `mGuiScale` értéke csak
memóriában tárolt editorbeállítás volt. A GUI-menü átírta és átméretezte
az ablakot, de a `SAWSTAR::SerializeState` csak a hangszínparamétereket
kódolta. A `SAWSTAR::UnserializeState` sem állított vissza GUI-skálát.
Új példány létrehozásakor a méretet ezért ismét a képernyőhöz tartozó
alapértelmezés határozta meg.

## Elkészült megoldás és ellenőrzési korlát

A host-state egy külön, v1 olvasók számára ismeretlen editorrekordban tárolja
a példány 75/100/125%-os választását. A régi olvasó ezt átugorja, a hangot
továbbra is be tudja tölteni; a `.sawstar` hangszínfájlok kódolása változatlan.
Régi projektben a hiányzó rekord automatikus képernyőméretet jelent. Hibás vagy
nem támogatott méret szintén automatikusra esik vissza; duplikált editorrekord
hibás állapot, ilyenkor sem a hang, sem a GUI-állapot nem változik.

A host szálán atomikus beállításátadás történik, natív átméretezés nélkül.
A GUI létrehozása vagy idle-je alkalmazza a méretet. A menü módosítása a VST3
opcionális `IComponentHandler2::setDirty` jelzésével jelöli mentendőnek az
állapotot, paramétergesztus és hangparaméter-módosítás nélkül. Ha a host nem
biztosít ilyen handlert, az explicit projektmentés ettől még tartalmazza a
méretet; az automatikus dirty-jelzés hostfüggő korlátja külön ellenőrizendő.

A Classic és Preview ugyanazt a host-state/editor implementációt használja.
A helyi codec- és a termelési metódusokat fordító host/GUI-átadási regressziók
sikeresek, és a teljes macOS ARM64 VST3 lefordult, de a teszt-host nem natív DAW.
A helyi Steinberg-validator konfigurálását a teljes Xcode hiánya megakadályozta
(Command Line Tools van); nincs helyi validator-siker állítva. A platform-CI,
a validator és a lenti REAPER-próbák külön kapuk; a hiba még nincs felhasználói
próbával lezárva.

## Elvárt működés

- A kiválasztott GUI-méret kerüljön a **DAW-projektben tárolt
  pluginpéldány-állapotba**, és újranyitáskor álljon vissza.
- A beállítás példányonként külön maradjon: például két azonos
  SAWSTAR-példány 75%-os és 125%-os mérete ne írja felül egymást.
- A Classic és a Premium Preview változatot egyaránt fedje le a javítás,
  amennyiben a külön preview build továbbra is használatban van.
- A GUI-skála ne legyen automatizálható hangparaméter, és a factory vagy
  felhasználói hangszín betöltése ne változtassa meg az editor méretét.
- Régi, GUI-méretet nem tartalmazó projektek maradjanak betölthetők;
  ilyenkor használható a meglévő alapértelmezett méret.
- A méretválasztás önmagában is legyen menthető a hostban, hangparaméter
  módosítása nélkül.
- A visszaállítás működjön a GUI létrehozása előtt, illetve már megnyitott
  editor esetén is; a natív átméretezés a megfelelő GUI-szálon történjen.
- A hangparaméterek, presetfájlok, parameter ID-k, automatizálás és a VST3
  bypass állapotának visszafelé kompatibilitása maradjon meg.

## Elfogadási ellenőrzések

- [ ] 75%, 100% és 125% mentése és visszatöltése ugyanabban a projektben.
- [ ] Két azonos pluginpéldány eltérő méretének független visszaállítása.
- [ ] Classic és Premium Preview együttes projektpróbája.
- [ ] Editor bezárása/újranyitása, valamint teljes DAW-újraindítás.
- [ ] Méretváltás utáni projektmentés más paraméter módosítása nélkül.
- [ ] Presetváltás és presetimport megtartja a választott GUI-méretet.
- [ ] Régi host-state és hibás/hiányzó skála kompatibilis kezelése.
- [ ] VST3 bypass és hangállapot változatlan round-trip ellenőrzése.
- [ ] Natív REAPER-ellenőrzés macOS-en és Windowson.

A feladat a roadmap **GUI-méret és presetkoherencia/plugin-életciklus**
pontjához tartozik. Az implementáció és a helyi regresszió nem módosítja
a CPU-mérési sorrendet vagy a prémium DSP elfogadási kapuit.
