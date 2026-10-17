# Teljesmotoros azonossági teszt időtúllépése — 2026-10-17

A #109 utáni `623a0e4f26f128e13e1691005351a259d3ee81c0` main
[Code quality első kísérletében](https://github.com/RobCZart82/SAWSTAR/actions/runs/38965411368/attempts/1)
a Linux code coverage 128/129 tesztje sikeres volt. Az
`oscillator_dispatch_engine_identity` 600,10 másodpercnél timeouttal állt le.
A teszt 480 teljesmotoros esetet futtatott egyetlen CTestként, `-O0`
coverage-instrumentálással. A kiírt részeredmények nem helyettesítik a
hiányzó teljes tesztsikert. Ez nem mért audio-deadline és nem bizonyít
DSP-eltérést sem.

A második kísérlet sikeres lett ugyanazon a forráson. Ez nem bizonyítja a
futásidő-ingadozás pontos okát és nem törli az első hibát. A workflow
eredeti futása és a mérési bizonyíték változatlan marad.

## Javítás

A teljesmotoros azonossági bináris `--waveform 0|1|2|3` szűrőt kap.
CTest négy külön hullámformát regisztrál, egyenként 120 esettel. Minden
eredeti ráta, filter, voice mód, FX-beállítás, moduláció, MIDI-esemény,
tail- és reset-ellenőrzés megmarad. A különböző OSC-hullámformákra váltó
esemény is mindegyik részben lefut. Az argumentum nélküli bináris továbbra
is a teljes, eredeti sorrendű 480 esetet futtatja.

A kutatási workflow és az aktuális gyűjtő CTest-szelektora mind a négy
részt tartalmazza. Hibás hullámforma-argumentum külön negatív tesztet kap.
A 600 másodperces tesztlimit, a 60 perces coverage-jobkeret, a numerikus
bitazonosság és a CPU-/minőségi küszöbök nem változnak. A részeket nem
indítjuk külön automatikus újrapróbálással.

A lezárt Windows/M1 kampány a rögzített `2f401939...` checkout régi
kvalifikációjával már elkészült; azt a javítás nem módosítja és nem indítja
újra. A jelen fejlesztési head saját CI-eredménye külön bizonyíték lesz.
Ez a javítás nem production DSP-változás és nem CPU-/natív elfogadás.

## Helyi ellenőrzés

Linux x64, GCC 13.3, Release: mind a négy 120 esetes rész sikeres, a
három hibás selector elutasítása és a gyűjtő regressziója is sikeres
(8/8 célzott CTest). A négy rész pontosan az eredeti 480 egyedi
ráta/filter/voice/FX esetet tartalmazza; a teljes, argumentum nélküli
futtatás CSV-kimenete soronként egyezik a négy rész együttes kimenetével.
Az archívum- és platformközi összevető 22 regressziója is sikeres.
Ez funkcionális teszt, nem új CPU-kampány.
Az instrumentált Linux coverage és a Windows/macOS CI eredménye a pontos
PR-headhez külön szükséges; a natív hostelfogadás továbbra is nyitott.
