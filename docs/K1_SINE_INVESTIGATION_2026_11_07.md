# K1 Sine — célzott oknyomozás, 2026-11-07

## Vizsgált negatív

A 810a388 head macOS Preview referencia jobja, 42365411396 / 128382317422,
Sine, 48 kHz, LP24, poly/FX esetben a 423. main mintánál eltérést jelzett.
Bal pre-FX 8 ULP, bal kimenet 6 ULP, jobb csatornák azonosak. A gépi rekord
[itt található](measurements/k1-sine-810a388-negative.json). Az eredeti hibás
bináris hiányzik az artifactokból. A megőrzött joblog SHA-256 azonosítója
rögzített. Az ok ismeretlen.

## Előre kijelölt vizsgálat

Egyetlen normál Sine-rács, egy passzív FP-megfigyelés és egy belső trace-rács
architektúránként. Az eredeti 120 eset sorrendje, az első hibás eset előtti
hét teljes render/teardown és a mintánkénti MIDI/control események megmaradnak.
A trace összehasonlítja a 16 voice envelope/pitch/oscillator/sub/noise/mix/filter/
render értékeit, majd a voice sum, pre-FX és effektkimeneteket. Az első belső
különbségnél korlátos diagnosztikával kilép. A normál és trace bináris eltérő
instrumentálása befolyásolhatja a reprodukciót, ezért mindkettőt megőrizzük.

A runner egyszer futtatja az előírt módokat, negatív után is megtartja a
következő kontrollokat. Nincs FP override, küszöbmódosítás vagy negatív eredmény
felülírása. A numerikus negatív a jobot pirosra állítja. A CI natív arm64,
Rosetta x86_64 és natív Intel x86_64 platformot fed le.

Megőrzött adatok: mindkét exact executable, cache, compilerparancsok, generált
motorforrások, compiler/OS/commit, stdout/stderr és SHA-256 manifest. A normál
Preview workflow jövőbeli hibájánál is feltölti saját eredeti binárisát,
konfigurációját és architektúra/hullámforma szerinti naplóit.

## Helyi eredmény

Apple Silicon gépen arm64 és Rosetta x86_64 alatt összesen tíz kijelölt
folyamat futott. Négy diagnosztikai self-test és hat teljes Sine-rács sikeres,
összesen 720 esetmegfigyelés. A normál/passzív/trace módokban nincs észlelt
eltérés. A 120 egyedi eset és nulla hibamező minden rácsnál ellenőrzött,
a bináris- és naplóhashok egyeznek. Az adatok az outputs/k1-sine-local-2026-11-07
mappában vannak. Ez egy kör eredménye, a korábbi negatívok megmaradnak.
A trace-konfigurálás egy elgépelt, használatlan kiegészítő CMake-változóra
figyelmeztetett; a tényleges SAWSTAR_PREMIUM_PREVIEW és NUMERICAL_TRACE értéke
ON, a trace self-test mindkét szeletnél trace=on eredménnyel zárult.

Két fixture-regresszió ellenőrzi a negatív és timeout utáni folytatást,
a két bináris hashait és a korábbi eredménykönyvtár felülírásának elutasítását.
Az actionlint és szintaxisellenőrzés sikeres.

## Következő döntési pont

A rögzített CI-kör eredménye alapján a trace első eltérő mezőjét vizsgáljuk.
Ha a normál kör negatív, az instrumentált pozitív, a megőrzött normál executable
azonos forrású, célzott további kontrollt tesz lehetővé. Ha minden kör sikeres,
a jelentés csak nem reprodukált állapotot rögzít, javításra vonatkozó állítás
nélkül. A publikálás Robert kérésére az oknyomozás eredményére vár.
