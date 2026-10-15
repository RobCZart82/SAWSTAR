# GUI-méret mentése DAW-projektbe

## Állapot — 2026-10-15

**Nyitott, feljegyzett hiba; a javítás még nincs implementálva.**
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

A `src/plugin/SAWSTAR.h` fájl `mGuiScale` értéke példányon belüli,
memóriában tárolt editorbeállítás. A GUI-menü átírja és átméretezi
az ablakot, de a `SAWSTAR::SerializeState` csak a hangszínparamétereket
kódolja. A `SAWSTAR::UnserializeState` sem állít vissza GUI-skálát.
Új példány létrehozásakor a méretet ezért ismét a képernyőhöz tartozó
alapértelmezés határozza meg.

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

A feladat a roadmap **presetkoherencia és plugin-életciklus** pontjához
tartozik. A mostani bejegyzés a hibát és az elfogadási feltételeket rögzíti;
a CPU-mérési sorrendet és a prémium DSP elfogadási kapuit nem módosítja.
