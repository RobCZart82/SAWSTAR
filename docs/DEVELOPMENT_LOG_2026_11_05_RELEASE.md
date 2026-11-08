# Fejlesztési napló — 1.2.0 előkészítés, 2026-11-05

Történeti napló: az 1.2.0 azóta megjelent 2026-11-07-én. A korábbi „nyitott”
és „publikálás nem történt” mondatai az adott dátum pillanatképét őrzik; az
aktuális publikált állapotot a [kiadási checklist](RELEASE_1.2.0_CHECKLIST.md),
a következő munkát pedig az [1.2.1 terv](RELEASE_1.2.1_PLAN.md) rögzíti.

Robert új kiadási célja 1.2.0. Friss GitHub-lekérés: main ec110a3 (#137),
4 workflow / 11 job sikeres, nincs nyitott PR. A #136 működési visszajelzését
és #137 ARP bypass javítását az új kiadás tartalmazza.

Az előkészítő ág az új mainből indul. 1.2.0 rc1 metaadat, config/plist,
README, kiadási leírás és aktuális terv készül; a régi dátumozott
1.1.0 RC bizonyítékok változatlan történeti források maradnak.
Az index/terv elavult állítását a hat új exportált hang elfogadásáról javítjuk.
A két kézikönyv az új candidate verzióból generálódik.

[Kiadási végrehajtási sorrend](RELEASE_1.2.0_CHECKLIST.md).
DSP, presetérték, paraméter-ID, FP-mód, mérési küszöb és alapmotor nem változik.
Implementáció, helyi ellenőrzés, távoli CI és natív elfogadás külön státusz;
a tényleges helyi/CI eredmény az előkészítő PR-ben kerül véglegesítésre.
Publikálás nem történt; a pontos RC elfogadása és végső kiadási döntés nyitott.

Helyi ellenőrzés: metadata sync --check sikeres; release_guard 21/21,
Premium RC packaging/provenance 5/5, Preview packaging 10/10, build display
metadata mind a hét release/dev/RC esete sikeres. Mindkét PDF 15 oldalas,
1.2.0 jelölésű; 30 oldal renderelve, kontaktlap vizuálisan és minden szó
oldalhatáron belül ellenőrizve. git diff --check sikeres. Távoli CI még függő.

## 2026-11-06 — #138 és az RC-fájlnév

A #138 beolvadt a `766ab99376afff22b8931401acf8f1f091a120fc` főágra.
A PR kilenc workflow-ja és 26 ellenőrzése, a főág öt workflow-ja sikeres.
A 42058959797 futás Windows x64 és macOS Universal RC-jében a manifest
1.2.0 rc1-et jelöl. Mindkét ZIP és a 30/32 payload hash ellenőrzött, a két
kézikönyv bájtra egyezik a verziózott fájlokkal. Három factory CSV-ben
egyenként 252 egyedi eset teljesíti a változatlan −6 dBFS küszöböt.

A workflow korábban 1.1.0 rc1-et rögzített a ZIP és SHA-256 fájl nevében.
A fájlnév most a release.json verziójából és RC-jelöléséből készül.
A helyi csomagtesztek és a névképzés két érvényes/két hibás bemenete
sikeres. Az új workflow eredménye és a natív elfogadás külön ellenőrzendő.

## Kiadási véglegesítés — 2026-11-07

Robert sikeres macOS/Windows használatot jelzett a 003b007 Premium RC-re,
majd kifejezetten engedélyezte a kiadást. A végleges 1.2.0 metaadat dátuma
2026.11.07. A normál telepítős Windows/macOS CI explicit Premium production
buildet készít, factory headroom-kontrollal. Az RC workflow a candidate
mező alapján csak RC esetén csomagol. A végleges kézikönyvek és release notes
rögzítik a migrációt és a megőrzött K1/CPU/ARM64 host-lefedettségi korlátokat.
A K1 okára vagy új natív CPU-mérésre vonatkozó eredmény nem keletkezett.

Helyben a végleges metaadatú arm64 Premium VST3 lefordult. A production
state/identity/latency próbák és a 252 factory eset sikeresek, a legrosszabb
limiter előtti csúcs −6,51292972 dBFS. A release guard 21/21, display metadata
7 eset, Premium RC csomag/provenance 5/5 és Preview csomagteszt 10/10 sikeres.
Az actionlint, metadata sync és diff-ellenőrzés sikeres. A két 15 oldalas PDF
mind a 30 oldala renderelve, kontaktlapon vizuálisan és szóhatárral ellenőrizve.
A végleges forrás távoli CI-je és csomagellenőrzése ezután következik.

A draft-előkészítő várakozása 75 perc, a workflow-é 85 perc, hogy a 60 perces
quality jobok végét megvárhassa. A release guard időtúllépés-regressziója
az új határt ellenőrzi; 21/21 teszt sikeres. A teszt- és mérési korlátok
változtatása nélkül készül a végleges PR.

A 7ff2a1e PR-head Windows VST3 x64/ARM64 teljes tesztbuildje a
parameter_registration fixture-ben elbukott. A Premium ág által hivatkozott
FactoryPresets deklarációja hiányzott a teszt include-jai közül. Pótlása után
a helyi próba az output.gain_db Classic defaultját váró állításon elbukott.
A fixture most a Premium factory Init kalibrált alapértékét követeli meg,
a többi paraméter korábbi bounds/default és kanonizálási állításával együtt.
Az alkalmazás regisztrációs kódja és a kalibrált hangerő nem változott.
A 42359244622 futás és a helyi negatív próba a javítás indoka.

A teljes helyi Premium CTest további 30 fixture-hibát jelzett: a factory
próbák ugyanazt a Classic Init-egyezést kérték számon, és a Classic riget
használták. A test rig motorparamétert kap, a factory teszt a PluginSynth
azonos motorját és a kalibrált Init kimeneti gainjét ellenőrzi. A Classic
rig meglévő neve és hívásai megmaradnak. Minden render/rátaválasztás,
lecsengési próba és numerikus küszöb azonos; a kiadott motorral futnak.

A javított teljes Premium Release CTest 182/182 sikeres (125,72 s).
A Classic factory és paraméterregisztráció célzott kontrollja 31/31 sikeres
(13,99 s). A korábbi 152/182 negatív és a javított teljes log megőrzött.
A javítás feltöltése a korábbi head összes futásának befejezése után történik.

A d11087a headen a Windows x64 és macOS Universal Premium teljes CTest-je
182/182, VST3 validátora 47/47 sikeres, mindkét macOS szelet validálva.
A factory headroom lépés a külön EXCLUDE_FROM_ALL mérőprogram hiányán bukott
el. A két telepítős workflow a használat előtt explicit lefordítja a
sawstar_premium_preview_levels célt. A Windows lépés a fordítás hibáján
azonnal kilép. A mérő helyi explicit buildje és 252 esete korábban sikeres.

A 810a388 headen új K1-jellegű negatív keletkezett: macOS Premium Preview
42365411396 / 128382317422, Sine / 48 kHz / LP24 / poly / FX-on, main frame
423. A bal pre-FX eltérés 8 ULP, a bal kimeneti eltérés 6 ULP, a jobb csatorna
azonos. MXCSR 0x1fb3, rounding 0. A folyamat Rosetta x86_64 futásként az
arm64/x86_64 workflow-sorrendből azonosítható; a megelőző arm64 Sine teljes
120 esete sikeres volt. A nyers joblog mentett és SHA-256-azonosított,
a géppel olvasható rekord a measurements/k1-sine-810a388-negative.json.
Az eredeti hibás offline executable nem került artifactba ezen a workflow-n.
A negatív nem infrastruktúrahiba, oka ismeretlen. Nem indítunk változatlan
mérési retryt kedvezőbb eredményért. A kiadási PR jelenlegi CI-je piros.

## Célzott K1 Sine-vizsgálat indítása — 2026-11-07

Robert az oknyomozást kérte a publikálás elé. A #142 időközben beolvadt
4b614501 főágra; a vizsgálat erről indul. A publikus Latest az ellenőrzéskor
1.0.4. A már megőrzött 810a388 negatív helyi rekordja és naplóhasha megmarad.

A normál macOS Preview-ellenőrzés most minden eredmény mellett megőrzi
az eredeti összehasonlító executable-t, cache-t, compiler/OS/commit adatot
és generált motorforrást. A célzott külön vizsgálat egyszer futtatja a
Sine 120 esetes rácsát (a hibás eset előtti hét setup/render/teardown
esettel együtt), normál, passzív FP- és belső trace-módban. Minden mód
binárisa, compilerparancsa, generált forrása és nyers naplója hash-elt.
Egy negatív után is lefutnak a kijelölt kontrollok; a végső job piros marad.
A két runner natív arm64/Rosetta és natív Intel összevetést ad. A program
nem írja át az FP-környezetet és nem használ numerikus toleranciát.

Két helyi fixture-regresszió igazolja, hogy szándékos numerikus negatív
és timeout után is megmarad minden további kontroll és bináris/hash,
és az eredménykönyvtár felülírása tiltott. A valós helyi futás eredménye
külön rögzítendő; új gyökérok vagy javítás még nincs igazolva.

A helyi célzott kör tíz folyamata sikeres: négy self-test és hat 120-esetes
Sine-rács (720 megfigyelés) arm64/Rosetta alatt. Minden bináris/naplóhash és
esetinventár ellenőrzött. Új eltérés nem reprodukált, az ok még ismeretlen.
Részletek: K1_SINE_INVESTIGATION_2026_11_07.md. A CI-kontroll eredménye függő.


## 2026-11-08 — K1 filter/FIR first-divergence instrumentation

Robert accepted the bounded, named-field protocol. The calibrated Reference
and Candidate share the same Premium filter/Drive representation; only the
oscillator dispatch differs. The existing trace already compares voice and
output fields per sample. A separate offline-only deep flag now observes active
FIR histories/cursors/taps, inline lane sums, SVF integrators/intermediates and
dry/mix state, retaining the current plus five preceding samples.
See [method and limits](K1_FILTER_STATE_FIRST_DIVERGENCE_2026_11_08.md).

Implemented and locally checked: one fixed Rosetta control passed five processes /
360 case observations without divergence, before final coverage and manifest-drift
validation additions. Final ARM64/Rosetta self-tests cover real Sine filter capture
at 4x and 2x; six new runner regressions, two existing ones, four rejected build
configurations and five ordinary chorus/pair/state/identity CTests pass. All 32
local payload/log hashes were verified. Remote CI/native acceptance are not claimed. No production DSP policy, state/preset, tolerance, release
metadata or publication change. K1 remains unexplained.
