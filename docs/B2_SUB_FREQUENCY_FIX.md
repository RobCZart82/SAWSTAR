# B2 — A SUB mintavételarányos frekvenciakorlátja

Állapot: implementált; az ellenőrzési kapuk alább külön szerepelnek.
Forrásalap: `feda3ea9d3f62f01905b16f187e8490fe7088a4c` (#118 után).
[Feladatjegyzék](BUGFIX_DEVELOPMENT_PLAN_2026_10_20.md#6-b2--sub-felső-frekvenciapolicy).

## Rögzített policy és hatás

A SUB végső frekvenciája a többi oszcillátorhoz és a Synth hívóhelyéhez
igazodva legfeljebb a mintavételi frekvencia 45%-a. A külön, abszolút
20 kHz-es belső korlát megszűnik. A normalizált fázislépés számítása double
pontos, véges bemenetnél `[0, .45]` tartományra korlátozott; NaN és végtelen
esetén nulla. A negatív frekvencia szintén nulla lépést eredményez.

Például 96 kHz-en a 25087,708 Hz-es cél a korábbi 20000 Hz helyett a kért
frekvencián szól. 48 kHz-en ugyanezt a célt 21600 Hz-re korlátozza a motor.
Ez szélső, nagyrészt ultrahangos regisztert érintő P3 javítás, nem normál
basszusregiszterbeli hangolási hiba. A korábbi plafon felett az érintett
hangszínek szándékosan változnak; meglévő preset és automatizálás is elérheti
ezt a tartományt magas MIDI-hanggal és pitch benddel.

A fázis, a hullámformák és a 10 ms-os váltási fade képlete változatlan.
Nincs új paraméter, preset-/state-formátum vagy pluginazonosság. A nulla
frekvencia fázismegállítást jelent: a square kimenete ettől nem feltétlen néma.

## Tartós regresszió és eredeti-forrásos kontroll

- `tests/sub_oscillator.cpp`: öt ráta (44,1/48/96/192/384 kHz), három
  hullámforma, hét frekvencia egy már futó oszcillátoron. A 105 szakasz a
  szokásos basszust, a régi 20 kHz-határ két oldalát, a hajlított MIDI 127-et,
  a mintavételi határ környékét és a legnagyobb véges float bemenetet fedi le.
  Pozitív nullátmenetekből mért frekvencia, véges/korlátos kimenet; külön
  negatív, nulla, NaN/±Inf fázismegállítás és helyreállás. A régi fade-próbák
  is megmaradnak.
- `tests/mixer.cpp`: izolált SUB, dry filter, 48/96/192 kHz és mindhárom
  hullámforma. MIDI 127-en közép → maximális +12 félhang bend → közép,
  beállás után mért pre-FX frekvencia; 27 szakasz. A korábbi basszus-,
  keverő-, forrás- és kimenetvédelmi próbák megmaradnak.
- A két bővített tesztet a változatlan eredeti headerrel futtatva mindkettő
  hibázott: a közvetlen teszt 48 kHz / sine / 21000 Hz kérésnél 20000
  nullátmenetet mért; a motorpróba a magas SUB bend frekvenciájánál bukott.

Futtatás:

```sh
cmake --build build --target sawstar_sub_tests sawstar_mixer_tests
ctest --test-dir build -R '^(sub_waveform_transitions|source_mixer)$' --output-on-failure
```

## Ellenőrzési állapot

- Eredeti-forrásos negatív kontroll: mindkét bővített regresszió hibázott.
- Javított forrás, macOS arm64 / AppleClang 21: teljes Release CTest
  **139/139 sikeres**, benne a két bővített teszt. Debugban az érintett két
  teszt szintén sikeres (2/2). Ez új futás, nem a B1 eredményének átvétele.
- Új Release Universal `sawstar_premium_native_pair`: arm64 és x86_64/Rosetta
  alatt egy-egy teljes futás, négy hullámforma × 120 eset architektúránként.
  Összesen 960 esetmegfigyelés, minden output és pre-FX peak/relative-RMS
  hiba nulla; mindkét folyamat exit 0. Ez véges helyi numerikus kontroll,
  nem K1-okbizonyítás vagy teljes magasregiszter-lefedettség.
- #119 beolvadt, head `e0342da9636adcec197730852ffb2447874c06a8`: mind a 12
  PR-workflow completed/success. Új main `661fe6147fe3539ee160e7236ecb555c98053f1b`;
  main CI-je a következő munka kezdetén még futott, lintje sikeres volt.
  Ez a korábbi függő PR-kaput frissíti; natív elfogadást nem jelent.
- Natív REAPER, érintett magas hang/bend automatizálás és célgépes elfogadás
  még nincs elvégezve. CPU-előnyt nem állítunk.

A K1 Rosetta-biteltérés oka továbbra ismeretlen. A B2 javítása nem oldja fel
ezt a numerikus elfogadási blokkolót, és nem lazít bitazonossági küszöböt.
