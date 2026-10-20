# K1 — Belső jelút-nyomkövetés, 2026-10-20

Állapot: diagnosztikai eszköz implementált; **a K1 oka és javítása továbbra nyitott**.
Forrásalap: main `c6507db0886096ec999d2dd2517c2776f3702f86` (#120 után).
[Megőrzött negatív bizonyíték](ROSETTA_NUMERICAL_INVESTIGATION_2026_10_20.md).

## Változatlan kiinduló bizonyíték

A `fb3114e` hibás Universal bináris helyi SHA-256-ja ismét ellenőrzött:
`3228d4d226e7e5f6b213773831406b326a74a260340ed738e012efa1009cccd0`.
A hibás Triangle-logé:
`d893ac670bfe10981e3ae55c34593c433363ec7f38ecfd34cd875908c24f2c09`.
Ezek megegyeznek a korábban rögzítettekkel. A 48 kHz / HP12 / Poly / FX ON,
main frame 197 eltérésének bizonyítékát sem új bináris, sem későbbi PASS nem írja felül.

## Mit rögzít a diagnosztika?

A normál numerikus összehasonlító a kivétel előtt most egyszerre kiírja mindkét
motor output és pre-FX bal/jobb értékét és float bitjeit; waveform/rate/filter/
voice/FX, main/tail, nullától indexelt minta, FP-kerekítés és x86 MXCSR mellett.
A meglévő bitazonossági és nemvéges-kimenet ellenőrzések megmaradnak.

A külön `SAWSTAR_NUMERICAL_TRACE=ON` offline build ezen felül minden mintánál
összehasonlítja az explicit, definiált megfigyelési mezőket:

- 16 hanghely: note, RNG-állapot, envelope, bend és routed pitch; két oszcillátor
  sztereó kimenete, SUB/noise, filter előtti és utáni jel, végső voice-jel, velocity.
- Voice-sum, pre-FX, chorus, delay, reverb, width, skálázott és védett kimenet.
- Sample rate, resonance, filter mix, output gain, protection és LFO amp.

A mintában talált belső eltérés akkor is megállítja a diagnosztikát, ha az audio
még bitazonos. Legfeljebb 16 eltérő mezősor és a teljes eltérésszám íródik ki;
egyező mintánál nincs napló. Ez strukturált értéklista, nem nyers objektumhash:
nincs padding, pointercím vagy nem definiált memóriaterület beolvasása.
A trace rögzített tömböket használ; a renderben nem végez I/O-t vagy allokációt.

**Korlát:** a mezők sorrendje megfigyelési sorrend, nem bizonyított oksági sorrend.
A legkorábbi talált belső minta/mező a további célzott keresést vezeti. Ez még
nem az összes filter/FIR-együttható és történeti állapot pillanatképe; ha azonos
bemenet mellett először a filter tér el, annak mélyebb állapotát külön kell feltárni.

## A normál build védelme és negatív kontroll

A kapcsoló alapból OFF. A Synth trace-tárolója és megfigyelési kódja ilyenkor
előfeldolgozással kimarad. ON mellett a pluginépítés configure-hibával tiltott;
nincs diagnosztikai VST3/promóció. A trace-build megváltoztatja az állapotméretet,
memóriaképet és fordított kódot; egy sikeres trace nem bizonyítja a régi bináris
javulását, és nem CPU-mérés. Az FX-részlépések külön tárolása csak ON módban él.
A tail összehasonlításban a két Process-hívás sorrendje explicit Reference→Candidate,
mint a main szakaszban; az eredeti argumentumkiértékelési sorrendre nem építünk.
A MIDI/grid/események, a 4096 mintás main és a rátához kötött tail megmaradnak.

`--diagnostic-self-test`: szándékos output/pre-FX eltéréssel igazolja a hibajelzést
és mindkét mérési pont naplóját. ON módban 256 belső különbséggel ellenőrzi a
16 soros korlátot; külön ±0 bitkülönbség azonos audio mellett is hibázik. Egyező
kereteknél nincs hibajelzés/napló. A valódi renderben a trace pre-FX/guard pontja
egyezzen a tényleges audioértékkel, aktív hangoknál a trace ne maradjon üres.
Ezek diagnosztikai kontrollok; a zöld vizsgálat nem változik K1-javítássá.

## Futtatás és CI

```sh
cmake -S . -B build-trace -DCMAKE_BUILD_TYPE=Release -DSAWSTAR_BUILD_PLUGIN=OFF \
  -DSAWSTAR_PREMIUM_PREVIEW=ON -DSAWSTAR_NUMERICAL_TRACE=ON \
  '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64'
cmake --build build-trace --target sawstar_premium_native_pair
arch -x86_64 build-trace/sawstar_premium_native_pair --diagnostic-self-test
arch -x86_64 build-trace/sawstar_premium_native_pair --waveform 2
```

A [K1 offline workflow](../.github/workflows/k1-numerical-trace.yml) külön
concurrency-csoportban fut, nem szakítja meg a main/Preview ellenőrzéseit.
Rögzített négy önálló Triangle-folyamat architektúránként; eltérés után is
megőrzi az összes kijelölt eredményt. Nincs sikerig ismétlés vagy epsilon-emelés.
Az artifact executable/cache/logokat és hashokat, commitot, OS/compiler/host
adatokat őriz meg. A 120 másodperces folyamat-timeout hiba, nem PASS.
Az új bináris saját bizonyíték; a normál kvalifikáció továbbra külön workflow.

## Helyi ellenőrzési állapot

macOS arm64 / AppleClang 21, a módosított helyi munkafából:

- Végső normál Release CTest **140/140 sikeres**, az új diagnosztikai önteszttel.
- Végső instrumentált Universal bináris: négy külön Triangle-folyamat arm64-en
  és négy x86_64/Rosetta alatt, folyamatonként 120 eset, összesen **960**.
  Mind exit 0; minden output/pre-FX peak és relative-RMS hibamező nulla;
  belső eltérés nem jelentkezett. Az instrumentált önteszt mindkét architektúrán sikeres.
- Végső normál, nem instrumentált kalibrált pár: egy teljes, négy hullámformás
  futás architektúránként, összesen **960 eset**, minden hibamező nulla,
  mindkét folyamat exit 0. A normál öntesztek is sikeresek (`trace=off`).
- Trace+plugin configure szándékosan exit 1 az offline-only guardon. Workflow
  YAML és beágyazott Python helyileg parse-ellenőrzött; actionlint/platform-CI külön kapu.
- [Géppel olvasható helyi bizonyíték](measurements/k1-local-trace-2026-10-20.json):
  kód-/bináris-/loghashok, környezet, mindkét instrumentált sorozat és normál kontroll.
  A manifest kifejezetten módosított helyi munkafát jelöl, nem committed CI-eredményt.

Az első instrumentált kontroll-build
négy Triangle-folyamata mindkét architektúrán 120/120 esetet teljesített;
az ezt követő módosítás a trace tényleges renderhez illeszkedésének ellenőrzését
adta hozzá. Az első sorozat nem volt hibás, nem sikerig ismétlést indítottunk.
Mindkét diagnosztikai bináris új, módosított munkafából készült az ismert mainre;
a helyi munkafa nem azonos az eredeti megőrzött `fb3114e` committal/binárissal.

**Következő döntés:** a friss CI-trace első negatív eredményénél a két motor
első eltérő megfigyelési pontját vizsgálni. Ha nem reprodukálódik, K1 nyitott marad;
nem írunk találgatásra DSP-javítást, nem nevezünk meg igazolatlan Rosetta-/compilerhibát.
