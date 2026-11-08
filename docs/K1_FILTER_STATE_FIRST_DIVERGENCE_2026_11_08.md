# K1 – név szerinti filter/FIR első divergencia és ötmintás előzmény

Állapot: implementált, helyileg ellenőrzött diagnosztika; távoli CI még nincs.
Ez lokalizáló eszköz, nem K1-gyökérok vagy DSP-javítás.
Előzmény: [átadási jelentés](K1_AI_DEBUG_HANDOFF_2026_11_08.md),
[meglévő trace](K1_INTERNAL_TRACE.md), [Sine-negatív](measurements/k1-sine-810a388-negative.json).

## Szemantikai illesztés

A `cmake/PremiumPreview.cmake` ugyanabból a kalibrált Preview Synth-forrásból
állítja elő a Candidate-ot. A két oszcillátor típusa tér el; mindkét motor
`RateScaledLookupEnginePremiumFilter`-t használ, benne ugyanaz a
`BasicRateScaledPremiumDrive<true,true,true>` és `PremiumLowPass`.
Az aktív faktor ugyanaz: 4x, 176.4 kHz-től 2x. Az inaktív faktorpéldány
nem része a snapshotnak. Nem a skalár orákulumot vetjük össze SIMD-reprezentációval.

A meglévő `NumericalTrace` már mintánként ellenőrzi a voice-, oscillator-,
filter-output-, pre-FX- és FX-kimeneteket. A `--diagnostic-waveform` FNV-útja
másik megfigyelés: nem azonos e belső trace-szel. A hiányzó mélyebb
filter/FIR-megfigyelést külön `SAWSTAR_FILTER_STATE_TRACE=ON` kapcsoló adja.
Ehhez Numerical Trace, offline Premium Preview és `BUILD_TESTING=OFF` szükséges; production és
pluginépítés tiltott. Kikapcsolva a mély megfigyelés nem fordul bele a motorba.

## Megfigyelt mezők és inline pontok

- Aktív Drive-faktor; smoothed/target gain és slew; FIR- és fázistapok.
- Mindkét csatorna interpolátor/decimátor cursorja és összes deklarált
  history-eleme, beleértve a tükrözött részt is. Nincs padding/pointer tartalom.
- Minden feldolgozott fázis interpolált, shaped és kimeneti double értéke.
- A SIMD FIR négy double lane-akkumulátora minden négytapú csoport után,
  még az eredeti redukció előtt. Ez kijelölt számítási pont, nem minden
  gépi utasítás vagy Rosetta belső regiszterének megfigyelése.
- Filter mode/weights, cutoff/resonance, g/k és célértékek, a1/a2/a12,
  rate/slew/controls-moving, LP24 két és LP12 közös hálózat integrátorai.
- Minden SVF Step bemenete, előző band/low állapota és köztes band/low értéke.
- Adapter késleltetett dry puffere, cursorja és mix/cél/slew értéke.

Csak az adott mintában ténylegesen feldolgozott voice kap mély snapshotot;
a voice index és a meglevő note/noise/voice trace biztosítja a hozzárendelést.
Az inaktív hanghelyek nem futtatott lokális számításaihoz nem találunk ki adatot.
Ez nem a teljes Synth összes komponensének/minden rejtett inicializálásának bizonyítása.

A mezők `(name,index,kind,bits)` alakban szerepelnek. Float 32, double 64 bites
memcpy bitmintával, integrális mező pontos egészként tárolt. A név karaktereit
hasonlítjuk, nem stringpointer-címeket vagy raw struct memóriát.
A `drive.phase_taps` indexe `phase*33+tap`; a history index
`channel*history_size+slot`; lane-index `channel*1024+phase*128+tap+lane`.
A filter integrátor/Step indexe `channel*3+stage`, ahol 0/1 a LP24 első/második
Step, 2 a külön LP12 Step. Az adapter dry indexe `frame*2+channel`.

## Trigger és előzmény

Esetenként egyszer lefoglalt, hat elemes heap-ring tárolja az aktuális és
legfeljebb öt előző minta két motorhoz tartozó snapshotját, phase/frame és
Reference/Candidate előtti/utáni FP-pillanatképpel. A ring a main→tail→reset
határon folytatódik, a következő grid-esetnél újraindul. Az első mintánál
értelemszerűen nincs öt korábbi adat.

Bármely mező-, schema- vagy meglévő voice/jelút-eltérés triggerel; nem kell
megvárni a 423. vagy bármely előre kijelölt mintát. Az előzmény név szerinti
páros bitmintái csak triggerkor íródnak ki. A normál bitguard és nemvéges
ellenőrzések megmaradnak. Kapacitástúllépés hiba, nem csendes csonkítás.
A fejléc `localization_only=1`: az első megfigyelt eltérés nem bizonyított ok.

## Véges kontroll és megőrzés

Új workflow: `.github/workflows/k1-filter-state.yml`, kizárólag Apple Silicon
runner, x86_64/Rosetta végrehajtás. A hostarchitektúrát és a
`sysctl.proc_translated=1` eredményt külön ellenőrzi.

A `scripts/investigate-k1-filter-state.py` a meglévő Sine-runner 120-esetes,
eredeti sorrendű validálását, 180 másodperces folyamatkorlátját és
negatív/timeout utáni előre kijelölt kontrollokat használja:

1. Normál önteszt és mély önteszt.
2. Egyszeri normál Sine-grid.
3. Egyszeri passzív FP Sine-grid a normál executable-ből.
4. Egyszeri mély filterállapot Sine-grid a külön instrumentált executable-ből.

Összesen öt folyamat, három 120-esetes grid. Nincs automatikus retry,
FTZ/DAZ-váltás, küszöbemelés vagy mintaszámcsökkentés. A normál build mindkét
trace-kapcsolója OFF; a mély build mindkettője ON, ezt cache és önteszt-marker
ellenőrzi. A normál jelen-forrású kontroll nem az elveszett 810a388 executable.

A manifest még a futtatás előtt hash-eli és másolja a két executable-t,
cache-t, compile commandokat és generált motorforrásokat; minden stdout/stderr
hash-elt és megőrzött. A kiegészítő manifest a mély trace/FIR/filter források
hashait, translation probe-ot és a hatminta-előzmény szabályát is rögzíti.
Létező eredménykönyvtár elutasított, negatív eredményt későbbi PASS nem cserél le.

Az állapotmásolás, további mezők és lane-kiolvasások változtathatják a
kódgenerálást, memóriaképet és futási időt. PASS esetén csak nem reprodukált
állapotot mondhatunk; sem időzítési/Rosetta-okot, sem K1-javítást nem állítunk.
Negatív esetnél a legkorábbi különböző név szerinti állapot/műveletből indulhat
új, célzott oksági kísérlet. E kör után nincs automatikus új kampány.

## Helyi ellenőrzött eredmény — 2026-11-08

- Universal mély build elkészült; ARM64 és Rosetta öntesztek sikeresek.
  A végső önteszt valódi Sine/LP24/Poly/FX első mintán is ellenőrzi mindkét
  aktív faktort (48/176.4 kHz), az összes kijelölt snapshot/inline pont jelenlétét
  és a két motor egyezését. Ez nem újabb teljes mérési kampány.
- A signed-zero belső eltérés azonos külső audio mellett is triggerel;
  nyolc egyező minta után a 8. frame negatív kontrollja pontosan a 3–8.
  frame-et őrzi. A mezőnév/index/mezőszám eltérése és a kapacitástúllépés
  elutasított. Ezek szándékos diagnosztikai negatívok, nem K1-reprodukciók.
- Egyetlen előre kijelölt helyi Rosetta-kör: öt folyamat, három teljes Sine-grid,
  összesen 360 esetmegfigyelés, minden exit 0 és nulla eltérés. A helyi kör még
  a végső coverage-assert/self-test és manifest-drift guard hozzáadása előtti
  instrumentált forrásból futott; a végső teljes-grid ellenőrzés a CI feladata.
  [Hash-ellenőrzött helyi manifest](measurements/k1-filter-state-local-2026-11-08.json),
  32 payload/loghash ellenőrzött. Módosított helyi munkafa, nem committed CI-PASS.
- A legelső helyi indítás a translation sysctl hozzáférésénél, a gridfolyamatok
  és az eredménykönyvtár létrehozása előtt megállt. A hozzáférés rendezése után
  indult a fenti egyetlen numerikus kör; nem kedvezőbb számokért történt retry.
- Hat új Python evidencia-regresszió és két meglévő runner-regresszió sikeres:
  hiányzó mély marker, módosított baseline, nem igazolt translation és forrás-drift
  nem fogadható el PASS-ként; a negatív megmarad.
- Négy érvénytelen konfiguráció (trace nélküli mély mód, production mód,
  pluginépítés, nem kalibrált általános CTest-mód) megfelelően elutasított. Normál build újrafordítva; a kijelölt
  öt chorus/pair/state/identity CTest sikeres. A linker két ismert duplicate-library
  figyelmeztetést adott; numerikus vagy fordítási hiba nem jelentkezett.
- Actionlint, Python-szintaxis, release-metaadat és diff ellenőrzés sikeres.

K1 továbbra nem reprodukált ebben a kontrollban, gyökérok/javítás ismeretlen.
Nincs új host-, CPU-, teljes bank- vagy publikálási elfogadás.
