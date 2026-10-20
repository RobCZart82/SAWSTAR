# B1 — A nullára simított vezérlők szubnormális lecsengésének javítása

Állapot: **implementált; helyi és pontos-head CI/natív eredmények alább külön**.
Forrásalap: `ea4961c544ae4f01eec1b610ab229511074e9cd7` (#117 után).
[Feladatjegyzék és lezárás](BUGFIX_DEVELOPMENT_PLAN_2026_10_20.md#3-b1--szubnormális-dsp-simítóállapotok).

## Hiba és célzott változás

A `Synth::ProcessStereo()` mixer-, mod wheel- és pressure-simítói nullára
tartó float állapota szubnormális tartományba jutott, majd megállhatott.
A javítás a meglévő rekurzió után csak **pontosan nulla célértéknél**, `1e-24`
alatti abszolút állapotot zár nullára. Nem nulla célhoz és a küszöb felett a
képlet változatlan. A 48 kHz-es 5 ms simítónál a lezárás körülbelül 276 ms
után történik, még jóval a float szubnormális tartománya előtt.

A küszöb körülbelül −480 dB amplitúdóarány: nem hallható fade-rövidítésre vagy
általános kvantálásra szolgál. A változás nem ad új paramétert, nem módosít
gain-policyt, state-formátumot vagy pluginazonosságot. A közös Synth-forrásból
képzett Preview/Candidate példányok is megkapják a javítást újrafordításkor.

Ez a B1 numerikus hibájának célzott megoldása. Nem igazolt CPU%-gyorsulás,
natív REAPER-elfogadás vagy a K1 Rosetta-biteltérés megoldása.

## Regresszió

`tests/control_smoothing.cpp`, CTest: `control_smoothing`.

- Öt ráta: 44,1 / 48 / 96 / 192 / 384 kHz; külön mixer, CC1 és channel pressure,
  összesen 15 részpróba. A mixer négy forrása együtt nullára kerül.
- 100 ms nem nulla előkészítés után nullára váltás, majd két másodperc feldolgozás.
- Nincs aktív hang: ez a simítók alulcsordulását különíti el a filter/FX/audio
  lecsengésétől. A kimenet végig pontosan néma maradjon.
- Gradual-underflow környezetben az `FE_UNDERFLOW` jelző nem keletkezhet.
  A teszt megőrzi/visszaállítja az FP-környezetet és megfigyeli, hogy a runtime
  valóban képez-e szubnormális értéket. Flush-to-zero runtime-on külön jelzi,
  hogy az alulcsordulási ellenőrzés nem alkalmazható; abból nem állítja a régi
  forrás IEEE-környezetbeli biztonságát.
- A meglévő átmeneti/modulációs tesztek a hangzó vezérlés viselkedését továbbra
  ellenőrzik. Az új teszt nem a javítási küszöböt vagy segédképletét másolja.

Helyi macOS arm64 / AppleClang 21 Release: az új teszt az **eredeti Synth-kóddal
mind a 15 részpróbában hibázott**, exit 1, `Zero-target smoothing underflow`
üzenettel. Ez eredeti-forrásos negatív kontroll, nem pusztán új PASS-teszt.

Futtatás:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSAWSTAR_CHECK_DAISYSP=ON
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure --timeout 600
```

## Ellenőrzési állapot

- Eredeti-forrásos negatív kontroll: 15/15 részpróba kimutatta a hibát.
- Javított forrás macOS arm64 / AppleClang 21: teljes Release CTest **139/139**
  sikeres; az új `control_smoothing` Debug buildben is sikeres, 15/15 részpróbával.
  Ez új helyi ellenőrzés, nem az audit korábbi 138 tesztjének átvett státusza.
- Új Release Universal `sawstar_premium_native_pair`: arm64 és x86_64/Rosetta
  alatt egy-egy teljes futás, architektúránként négy hullámforma × 120 eset,
  összesen 960 esetmegfigyelés. Mindkét folyamat exit 0; a kimeneti és pre-FX
  peak/relative-RMS hibák minden sorban pontosan nullák. Véges helyi numerikus
  kontroll; nem K1-okbizonyítás és nem CPU-mérés.
- PR-head platform-CI és a merge utáni main: az alábbi friss bejegyzés szerint sikeres.
- Natív REAPER, Windows célgép, tényleges CPU/dropout-hatás: még nem mért.

## Beolvasztás és platform-CI — 2026-10-20

#118 beolvadt, head `4ae076df0b2881c3c84ddcd161f74d8e78bb250a`: mind a 17
PR-workflow completed/success. Az új main
`feda3ea9d3f62f01905b16f187e8490fe7088a4c` négy fő workflow-ja szintén sikeres:
[macOS](https://github.com/RobCZart82/SAWSTAR/actions/runs/39372390500),
[Windows](https://github.com/RobCZart82/SAWSTAR/actions/runs/39372390379),
[Code quality](https://github.com/RobCZart82/SAWSTAR/actions/runs/39372390297),
[workflow lint](https://github.com/RobCZart82/SAWSTAR/actions/runs/39372390263).
Ez a B1 platformkapuit lezárja; a natív/CPU-elfogadást és K1-et nem.

## Következő kapuk

A fenti CI-kapuk teljesültek; hátravan az érintett hangzó automatizálási
próba és a Preview/Candidate célgépes numerikus kontrolljai. A K1 továbbra külön ügy:
#116 beolvasztás nélkül lezárult, a hibás diagnosztikai head bizonyítéka megmarad.
A következő külön diagnosztikai munkának a dokumentált első belső jelút-eltérést
kell keresnie, nem a B1 javítását K1-megoldásként feltételeznie.
