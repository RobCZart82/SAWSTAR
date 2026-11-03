# Repository-audit javítások — 2026-11-02

Forrásalap: main `b24f4451187c0f552e4e7cabddf3d35a10a894f3` (#131).
Az audit után a távoli main ellenőrzése ugyanezt a commitot adta.
Ez helyi implementációs feljegyzés; távoli CI- vagy natív host-PASS-t nem állít.

## 1. macOS futtatókörnyezet

A fő build már macOS 15-ön futott, de 18 további workflow még a
2026-11-02-án kivezetett macOS 14 címkét használta. Ezek is explicit
`macos-15` gépre állnak át. A dispatch CPU-kampány macOS artifactjának
előállítója és letöltője együtt változik. A fő build két örökölt kötelező
checkneve megmarad; ezek nem futtatógép-választók.

A natív Intel kontroll továbbra `macos-15-intel`. Az architektúrák, fordítási
opciók, tesztek, benchmarkküszöbök és időkeretek változatlanok. A korábbi
macOS 14 mérési archívumok történeti bizonyítékok maradnak; új runneren új
eredetet kell rögzíteni, teljesítményjavulás nem következik a címkecseréből.

## 2. LFO nullára tartó mélység

A `Lfo::Process()` négy célpontjának simítója nullára állítás vagy rerouting
után szubnormális float állapotban maradhatott. A javítás kizárólag nulla
célértéknél, `1e-24` alatti abszolút állapotot zár nullára. A nem nulla
célérték és az ezt megelőző simítás képlete megmarad. Ez nem K1-javítás és
nem igazolt CPU%-gyorsulás.

Új CTest: `lfo_zero_target_smoothing`. Öt ráta (44,1/48/96/192/384 kHz),
négy célpont, depth=0 és célpontváltás: összesen 40 eset. A két másodperces
fade után a korábbi célpontnak pontosan semlegesnek kell lennie; gradual
underflow runtime-on `FE_UNDERFLOW` nem keletkezhet. FTZ runtime-on a teszt
külön jelzi az alulcsordulási ellenőrzés korlátját.

Az új teszt az eredeti, `git show HEAD:src/dsp/Lfo.h` forrással külön
fordítva **40/40 esetben hibázott**, exit 1. A régi forrás nem lett átírva
a negatív kontroll kedvéért. A javított teszt és a teljes csomag eredménye
az ellenőrzési részben szerepel.

## 3. Kedvencek zárolása

A `ChangeFavorite()` korlátlan process-local mutexvárakozása megszűnik.
A már meglévő `FavoritesLock` Windows `LockFileEx`, illetve POSIX `flock`
zárral külön fájlhandle-ök között, ugyanazon folyamaton belül is kizárást
biztosít. A teljes read-modify-write művelet e zár alatt marad; az atomi
csere, sérült fájl megőrzése és sikertelen mentés védelme nem változik.

A fájlzár megszerzési próbálkozásai továbbra körülbelül két másodpercig
tartanak, majd foglalt hibát adnak. Ez nem a teljes fájlművelet határideje:
az open/read/write/fsync és más háttértár-I/O továbbra blokkolhat.
A teljes R1 natív/GUI-/lassúmeghajtós elfogadás ezért nyitott marad.

A meglévő `preset_reliability` teszt 16 párhuzamos szál és POSIX-on hat
folyamat kedvencfrissítéseit, az elveszett frissítések tiltását, sérült fájl
megőrzését és sikertelen írás utáni adatbiztonságot is ellenőrzi.

Új POSIX regresszió ugyanebben a tesztben: egy FIFO a kedvencek olvasását
feltartja az első mappában. A sikeres writer-open bizonyítja, hogy a reader
már elkezdte az olvasást; ezután indul a másik mappa kedvencmódosítása.
Ennek az első olvasás elengedése előtt be kell fejeződnie. A két másodperces
watchdog nem GUI-késleltetési cél vagy mérés. Az eredeti `UserPresets.cpp`
forrással külön fordított kontroll exit 1-gyel hibázott:
`favorites in another library waited on a process-global mutex`.
A javított Release-próba sikeres, és mindkét mappa mentett adata megmarad.
A FIFO-próba Windowson és az opcionális külső (akár FAT/exFAT) meghajtós
futtatásban nem fut; Windowson a meglévő szálas regresszió és a távoli
Windows CI szükséges. A zárak API-szerződése:
[Apple flock](https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/flock.2.html),
[Microsoft LockFileEx](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-lockfileex).

## 4. Dokumentáció és nyitott kapuk

A CHANGELOG Unreleased része most tartalmazza a kiadás utáni érdemi
változásokat és a jelen javításokat. Az aktuális index és roadmap erre a
feljegyzésre mutat. A történeti mérési eredmények és kiadási naplók megmaradnak.

K1 gyökérok, a 28 eset / 112 ablak natív CPU-/dropout-elfogadás,
Windows hostpróba, GUI-scale/projekt/preset/lifecycle elfogadás, production-
integráció és végső RC továbbra külön nyitott feladat. A korábbi sikeres
macOS felhasználói hangpróba hatóköre nem bővül.

## Ellenőrzés

- macOS arm64 / AppleClang 21, Release foundation/DSP fordítás sikeres;
  teljes CTest **142/142 sikeres**. A kedvencek új FIFO-regressziójának
  hozzáadása után a módosított `preset_reliability` külön is sikeres.
- Debug: `lfo_zero_target_smoothing` és `preset_reliability` **2/2 sikeres**.
- `actionlint` **1.7.12**, a hivatalos kiadási SHA-256-tal ellenőrzött
  darwin_arm64 bináris: az összes workflow sikeresen ellenőrizve.
- 58 Python-fájl szintaxisa rendben; release metadata check sikeres;
  README/CHANGELOG/docs Markdown-hivatkozások helyi célfájljai léteznek;
  `git diff --check` sikeres.
- A két eredeti forrással futtatott negatív kontroll a leírt hibával elbukott.

Az alábbi távoli állapot az eredeti audit-headhez tartozik; a későbbi
LFO-kódmódosítás saját CI-je új, külön kapu. Natív elfogadást nem jelent.


## Távoli CI és LFO-költség korrekció — 2026-11-02

A #132 eredeti headje `8b164082`, base `b24f4451`, a CI tesztmerge-je
`4160c8ed`. Mind a 23 workflow befejeződött: 22 sikeres; az összes 71 job közül 70 sikeres. A macOS foundation
Debug és Universal VST3 job sikeres; Release-ben 142/142 CTest és a külön
source/engine audit is sikeres, de a production CPU-kapu hibás.
[Negatív futás, attempt 2](https://github.com/RobCZart82/SAWSTAR/actions/runs/41503133369/attempts/2).
A hét, géphiány miatt el sem indult job korábbi infrastruktúrahibája külön
esemény; nem keverhető össze a tényleges CPU-küszöbtúllépéssel.

A megőrzött artifact `13183531221`, feltöltési ZIP SHA-256:
`6b8866bfbb24821ae62a32a8f629acb6b49e9ffae64ebf84b6461be0846aec39`.
Mind a 18 CSV hashét ellenőriztük és a mediánokat újraszámoltuk.
`full_fx / 8`: baseline 9,00337495%, current 11,2677562% valós idejű CPU;
+25,1503604%, a változatlan +25% és +1,0 százalékpontos kapu hibázik.
A határ 11,2542186875%; a túllépés 0,0135375125 százalékpont. A kis különbség
nem ok az eredmény figyelmen kívül hagyására vagy a mérés ismétlésére.

A korábbi zöld main [futásának](https://github.com/RobCZart82/SAWSTAR/actions/runs/40564944069)
18 CSV-je is hash-ellenőrzött. Azonos baseline bináris mellett a 16 hangos
full_fx baseline medián 17,1184% → 21,5343%, a current 19,7063% → 19,6055%.
A workload és a küszöbök azonosak, az operációs rendszer verziója eltér.
Ez időzítési változást mutat, de nem bizonyít infrastruktúra-eredetű felmentést.
A benchmark rögzített referenciája `08730644`, nem a PR előtti main;
a +25,15% ezért önmagában nem rendelhető a #132-höz.

A #132 egyetlen audioút-változása az LFO-mélységsimító lecsengési védelme.
AppleClang 21 arm64 `-O3 -DNDEBUG` külön fordított Process-próbájában az
új rövidzáras feltétel a teljes függvény feltételes ugrásait 7-ről 18-ra,
a gépi utasítássorokat 122-ről 188-ra növelte. Ez statikus kódelemzés,
nem a CI-lassulás gyökérok-bizonyítása és nem CPU-mérés.

A korrekció a két tiszta bool összehasonlítást rövidzár nélkül értékeli
(`&` bool operandusokkal). A zéruscél-feltétel, az 1e-24 küszöb és az
aritmetikai simítórecurrencia változatlan. Ugyanabban a fordítási próbában
7 feltételes ugrás és 131 utasítássor keletkezik; a nullázás feltételes
választóutasítást használ. Más fordító eredményét ez nem garantálja.

A korábbi rövidzáras implementációval végzett helyi differenciális próba
34 231 616 mintán az összes kimenet, Value és Phase bitazonosságát igazolta:
öt mintavételi ráta, 64 vezérlésváltás/rátánként, mind a négy cél és alak,
zéró/normál/nagyon kis nem zéró mélység, sync/retrigger és célváltás.
Ez numerikus ekvivalenciapróba, nem kedvezőbb eredményért ismételt CPU-kapu.
A Debug lecsengési regresszió sikeres (1/1). Az új Release újrafordítás
és a teljes 142/142 CTest sikeres (220,18 s); metadata és diff-ellenőrzés
sikeres. Az új head távoli CI-je még függő; a korábbi piros eredmény megmarad.
A korrekció nem tekinthető a CPU-blokkoló bizonyított lezárásának.
K1 és a natív REAPER/Windows/CPU-elfogadás változatlanul nyitott.


## Második negatív CPU-kapu és Classic Drive-költség — 2026-11-03

A `01884509` head minden workflow-ja befejeződött: 22/23 sikeres, 70/71 job
sikeres. Az egyetlen piros ismét a macOS Release CPU-kapu, most full_fx / 16:
baseline 14,739725%, current 19,25259795%, +30,61707698%; határ 18,42465625%.
A nyolchangú eset +14,94886%, de ez nem írja felül a tizenhathangú hibát.
[Második megőrzött negatív futás](https://github.com/RobCZart82/SAWSTAR/actions/runs/41541817295).
Artifact `13188426742`, feltöltési ZIP SHA-256
`2bf34fa8a299b132066b4c7f173298d82e43ff80f94973ca949a85cab21a6a98`.
Mind a 18 CSV hash-ellenőrzött; a meglévő read-only auditor is konzisztensnek
ítélte az eredeti, továbbra is sikertelen kaput. A LFO-korrekció nem igazolta
az első CPU-hiba okát vagy a blokkoló lezárását.

A rögzített `0873064487e02b2e1da49b42d0fd12ca83e11ec7` referencia forrását
külön megvizsgáltuk. A mai motor sok korábbi, szükséges javításban eltér:
Drive DC-szűrés, biztonsági/vezérlés-simítás, hangújrafelhasználás stb.
Nem vonunk vissza védelmet vagy hangminőségi javítást a kedvezőbb számért.

Helyi M1/AppleClang 21 Release hívásverem-profil készült a full_fx / 16
beállításokat használó meghosszabbított renderrel. Ez nem ismételt CI-kapu,
nincs belőle új CPU-arány vagy elfogadás. Az öt másodperces profil 4218
veremmintájából az OSC1/OSC2 ág 1203/934, a fő LowPass ág 1034 mintával
szerepel. A filterágban látható a Drive `pow` és normalizáló `tanh` hívása.
Ez költséghelyet jelöl, a CI-hiba gyökérokát nem bizonyítja.

A Classic filter minden mintán újraszámolta ezeket akkor is, amikor a
simított Drive double értéke már pontosan változatlan. 48 kHz/6 dB-nél a
recurrencia helyi számtani vizsgálata kb. 0,309 s után érte el ezt az állapotot.
A korrekció példányonként tárolja a pontos Drive-kulcsot és az eredeti
`pow(10.,drive_/20.)`, `tanh(driveGain)` eredményét. Csak pontos kulcsegyezés
mellett használja újra; a nullás Drive útja és minden változó mintán az
összes eredeti művelet megmarad. Init érvényteleníti a kulcsot, SnapToTargets
utáni értékváltást a következő Process felismeri; Clear a továbbra érvényes
vezérlési adatot megtarthatja. Nincs globális cache, másodlagos szálszinkron,
közelítő matematika vagy megváltozott slew/threshold.

Helyi differenciális ellenőrzés a `01884509` eredeti LowPass implementációval:
15 282 000 sztereó minta bitazonos, öt ráta × négy mód, tíz Drive-szakasz,
zéró/kicsi/6/12/24 dB, simítás, SnapToTargets, Clear, ismételt Init és NaN
bemenet után is. Külön műveletszámláló próbában 100 000 rögzített, már
snapelt Drive-minta korábban 100 000 pow és 100 000 normalizáló tanh hívást
végzett, most 1–1-et. Ez műveletszám, nem CPU-gyorsulási mérés.
Debug filter_character 1/1 sikeres; új Release újrafordítás és teljes
142/142 CTest sikeres (219,57 s). Metadata és diff-ellenőrzés sikeres.
Az új forrás távoli CI-je még függő, külön kapu.
A két korábbi negatív CPU-bizonyíték megmarad, CPU-/natív-/K1-lezárás nincs.
