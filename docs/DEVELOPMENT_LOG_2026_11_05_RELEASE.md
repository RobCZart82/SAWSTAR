# Fejlesztési napló — 1.2.0 előkészítés, 2026-11-05

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
