# Fejlesztési alap a pattanásvizsgálat lezárása után

2026-09-15 — helyi, nem publikált 1.0.2-rc8 fejlesztési állapot.

## Döntés és kiindulópont

A felhasználó lezárta a pattanó/kattanó hang gyökérok-keresését. A hasonló hangot a Vital rögzített fázisú, szűrés és torzítás nélküli kontrolljaiban is hallotta. Ez nem bizonyítja, hogy minden szintetizátor hibamentes, vagy hogy azonos a belső mechanizmus. A vizsgálat státusza: lezárva felhasználói döntéssel, nem igazolt általános pattanásjavítás.

Új helyi ág: `codex/character-preserving-development`.
Alap: `4b91145`, a kiadott 1.0.1 utáni dokumentációs merge. A plugin hangforrása itt az 1.0.1 kiadási állapotával egyezik. A korábbi `codex/mono-crossfade-preview` ág, a rajta lévő helyi glide-javítás és a mérési anyagok megmaradtak. Nincs reset, force-push vagy kiadott bináriscsere.

## Átvett, külön ellenőrzött javítások

1. **Glide-előzmény:** a még nem renderelt akkordhang nem szolgálhat glide-kiindulópontként. A renderelt státuszt új, azonnal beállított hangmagasságnál töröljük; a valódi glide és Always Glide előzménye megmarad.
2. **Üres szólam burkolója:** a már inaktív szólam új foglalásakor az amplitúdó-burkoló nulláról indul. Ez megakadályozza a korábbi módváltásból ottmaradt sustain érték felhasználását. Aktív hangok retriggerének burkolókezelése marad az 1.0.1-é.
3. **Azonos mintájú elengedések:** Mono/Legato Note Off és pedálfelengedés után csak egyszer, a következő hangminta előtt választunk hangot. Nem kerül új audiominta-késleltetés a feldolgozásba. Az egyébként megmaradó Mono-visszalépés burkoló-újraindítási szabálya az 1.0.1-é.
4. **Tesztmemória:** a nagy zaj-, LFO- és voice-transition tesztpéldányok heapre kerültek a Windows Debug stack túlterhelésének megelőzésére. Ez nem érinti a plugin hangját.

## Ki nem választott kísérletek

- Hat milliszekundumos Mono hangág-keresztúsztatás és többágas tail pool.
- Ehhez kapcsolódó új Mono korrekciós rámpák és hangág-keverési változások.
- Szűrőindítás gerjesztésének lágyítása, harmonikus előkészítés, fázishelyes DC/szűrőállapot-cserék.
- Új frekvenciasimítás, speciális fallback-időzítés vagy hangerőcsökkentés.
- A Mono fallback és ismételt Legato hangok artikulációjának kísérleti átalakítása.
- Módváltások új, 6 ms-os kötelező kifuttatása: ez külön viselkedési változtatás, ezért nem került automatikusan vissza. Az ehhez kötött további megfigyeléseket megőrizzük.

Az alap létrehozásakor a `src/dsp`, GUI, presetek, paraméterazonosítók és hangmodul-beállítások forrása változatlan a választott 1.0.1 alaphoz képest. A verziómetaadat külön fejlesztési azonosítót kapott; rc8 nem publikált release és nem hallásos minőségígéret.

## Ellenőrzés

- `glide_history`, `idle_voice_reuse`, `release_order`: mindhárom új regresszió elbukott az eredeti 1.0.1 motorkódon, és sikeres a kiválasztott javításokkal. A tesztek 44,1/48/96 kHz-et vizsgálnak.
- További 10 meglévő teszt sikeres: voice_transitions, mono, overlapping_notes, reset_lifecycle, idle_start, parameter_transitions, engine_audit, torture, noise, lfo_wave.
- 27 normál játékmenet (3 mintavétel × Poly/Mono/Legato × saw/square/sine), 0 ms Glide-dal: a javított és az 1.0.1 motor teljes stereo mintasorainak hash-e azonos. Ez a vizsgált hangokra vonatkozik, nem minden preset és eseménysor bitazonosságának bizonyítása.
- Az igazolt hibahelyzetekben szándékos viselkedésváltozás van; általános hangkarakter-változtatás nincs hozzáadva.
- Helyi macOS C++ motorfordítás és offline tesztek. A fordító meglévő DaisySP `last_freq_` unused-field figyelmeztetést ad. Új VST3-build, telepítés, Windows-futtatás, GitHub-push vagy release nem történt ebben a lépésben.

A mérési nyomok a munkakönyvtár melletti `SAWSTAR-character-preserving-audit` mappában vannak; a forrásba három új, CMake-ből is futtatható regressziós teszt került.

## Megőrzött nyitott tételek

A retriggerelt LFO azonos mintájú On/Off sorrendfüggése külön specifikációs és tesztelési tétel; nem minősítjük javítottnak. A módváltás közbeni aktív szólamok folyamatos kifuttatása és pillanatnyi hangereje külön, célzott döntést igényel, nem hozzuk vissza a teljes rc7 átmenetkezelését.

A későbbi jobb filter, presetlista-rendezés és esetleges 32 szólam külön fejlesztési irány. A GUI mérete és pozíciói továbbra is rögzítettek. Hangkaraktert érintő fejlesztéshez külön összehasonlítás és jóváhagyott cél szükséges; a pattanáskutatás kísérletei nem az alapértelmezett motor részei.


## Az alapra épülő további ellenőrzött javítások

2026-09-19-ig elkészült az összesített kimeneti erősítés simítása,
a SetFreq által megszakított detune-simítás javítása és a Delay kezdő
olvasási idejének beállítása. Ezek már célzott DSP-/motorváltozások;
a fenti változatlansági állítás az alap létrehozására vonatkozik.
Részletek: [Output Gain](OUTPUT_GAIN_FIX.md),
[Detune és Delay](DETUNE_DELAY_FIXES.md).
