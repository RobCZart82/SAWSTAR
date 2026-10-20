# R1 — Gyors foglalt válasz a példányok közötti presetműveleteknél

Állapot: a korlátlan process-local mutex-várakozás célzott javítása implementált;
teljes natív GUI-/I/O-elfogadás továbbra nyitott.
Forrásalap: main `661fe6147fe3539ee160e7236ecb555c98053f1b` (#119 után).
[R1 feladat és elfogadás](BUGFIX_DEVELOPMENT_PLAN_2026_10_20.md#5-r1--több-példány-presetműveleteinek-gui-várakozása).

## Hiba és viselkedés

A presetimport a teljes batch alatt foglalja a modulon belül közös mutexet.
A Save, Overwrite, Rename és Archive (GUI Delete) ugyanerre várakozott,
határidő nélkül. A két másodperces fájlzár-határidő csak e mutex megszerzése
után indult, ezért nem korlátozta a másik példány importjára várakozást.

A `detail::PresetMutationGuard` ugyanazt a közös kizárást UI-hívásnál
`try_lock()` módban szerzi meg. Ha foglalt, a művelet azonnali foglalt kivételt ad:
`Preset library is busy. Please try again.` A fájlzár és a védett művelet
ilyenkor nem indul el. Az UI-műveletnek nincs automatikus újrapróbálása vagy új háttérsora.
A felhasználó az import befejezése után ismételheti a műveletet.
A meglévő GUI-kivételkezelők mutatják az üzenetet. Az `ImportPresets`
alapértelmezése szintén gyors elutasítás; így a külső Save As utáni
könyvtármásolás sem vár a másik példány mutexére. A már sikeres külső mentést
a meglévő warning-csatorna őrzi meg. Kizárólag az import-worker adja meg a
`waitForOtherMutation=true` módot: a háttérimportok továbbra sorban lefutnak
és deduplikálnak, GUI-várakozás nélkül. Ez nem új audio-thread útvonal.

A kizárás továbbra globális ugyanazon modul példányai között, eltérő mappáknál
is; nincs lock-regiszter vagy új, mappanévvel azonosított mutex-policy.
A folyamatok közötti fájlzár, exkluzív létrehozás, stale-write védelem,
backup és archive névütközés-védelem változatlan. A fájlzár felszabadulása
továbbra megelőzi a process-local guard felszabadulását.

## Regresszió és negatív kontroll

A `preset_reliability` teszt egy workerrel determinisztikusan foglalja a
ténylegesen használt process-local guardot. E foglalás alatt indítja az öt
publikus műveletet: Save, Overwrite, Rename, Archive és Import; két külön
célmappában, összesen tíz versengési esetben. Mindegyiknek még a foglaló
elengedése előtt foglalt választ kell adnia. Ellenőrzi, hogy a preset bájtjai
és a fájl-/backupnevek nem változtak. Feloldás után minden művelet tényleges
fájlrendszeri újrapróbálása és az overwrite/archive másolat tartalma is ellenőrzött.

A két másodperces `future`-watchdog csak a blokkoló regresszió biztonságos
lezárására szolgál; nem natív GUI-késleltetésmérés és nem 2 s-os új lock-timeout.
A régi zárolás először változatlan blokkoló szemantikával került a guardba;
e köztes kontroll-buildben a teszt hibázott:
`preset mutation waited for another instance's process-local lock`.
Ez viselkedésmegőrző refaktor utáni negatív kontroll, nem a változatlan
korábbi commit binárisának futtatása. Ezután változott az UI-guard gyors elutasításra; a worker várakozó módja megmaradt.

A meglévő 12 konkurens háttérimport-próba továbbra egyetlen azonos hangot
importál hiba nélkül. A forrásfájlok e tesztben a workerek indítása előtt
készülnek el; az importok explicit worker-várakozó módot kérnek.
A meglévő Save/Rename/Delete versenypróba a Save által megelőzött, foglalt
Rename/Delete műveletet a két szál befejezése után újrapróbálja. Megmarad az
adat-/backupellenőrzés és a régi név feltámadásának tiltása; a végleges guarddal
futó korábbi Release-próba még e régi, mindig várakozó szerződést kérte számon,
ezért `user_preset_files` hibát jelzett. E negatív futás nem DSP-hiba.

## Ellenőrzési állapot és határok

- A blokkoló kontroll-build elbukott. Javított forrás, macOS arm64 / AppleClang 21:
  végső teljes Release CTest **139/139 sikeres**. Debugban a `preset_reliability`,
  `preset_library_import`, `preset_import_job` és `user_preset_files`
  **4/4 sikeres**. A tíz foglalt
  eset és a feloldás utáni újrapróbálások a `preset_reliability` részei.
- A javító PR pontos-head CI-je külön kapu, indulás/függő állapot nem PASS.
- **Nem teljes R1-lezárás:** a külön folyamatból foglalt fájlzár továbbra
  legfeljebb körülbelül két másodpercig próbálkozik. A fájlrendszeri I/O,
  könyvtárbejárás, backup és a külön favorites-lock is várakoztathat.
  A Save esetén a mappalétrehozás a guard előtt történik, adatmentés nélkül.
- Két valódi REAPER-példány, nagy import, lassú/külső meghajtó és bridging
  melletti GUI-/audio-próba nincs elvégezve. Az eltérő pluginmodulok vagy
  külön folyamatok nem feltétlen osztják ezt a mutexet. Audio/dropout- vagy
  CPU-javulást nem állítunk, K1 Rosetta-okfeltárást nem helyettesítünk.

Futtatás:

```sh
cmake --build build --target sawstar_preset_reliability
ctest --test-dir build -R '^preset_reliability$' --output-on-failure
```
