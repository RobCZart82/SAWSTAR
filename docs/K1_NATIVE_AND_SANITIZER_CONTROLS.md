# K1 — Natív Intel-kontroll és kalibrált ASan/UBSan kampány

Állapot: #126 pontos-head CI-je és helyi nyersadat-újraellenőrzése sikeres;
K1 nem reprodukált. [Mért eredmények és következő döntés](K1_NATIVE_SANITIZER_RESULTS_2026_10_20.md).
Alap: `bef9b8db7c36092485b544b7d1ed3ca9a71b20ce`, #125 után.
A párhuzamosan futó main workflow-k nem módosulnak és nem kerülnek leállításra.
A két új workflow saját concurrency-csoportot és külön PR-ágat használ.

## Miért változik a vizsgálati prioritás?

A K1 megőrzött negatív bizonyítéka egy x86_64/Rosetta futásból származik.
Az eredeti executable későbbi, 28 folyamat / 3360 esetmegfigyelésből álló
[kampánya](K1_ORIGINAL_BINARY_REPLAY.md) nem ismételte meg az eltérést.
Ezért most a natív működés és a felderíthető memória/UB-hibák elkülönítését
helyezzük előre, újabb nagy Rosetta-ismétlés vagy mélyebb IE-trace helyett.

Az IE a #125 részletes mérésében az elsőként futó motor Reset() hívásánál
jelent meg. Ez külön megfigyelés audioeltérés nélkül; nem igazolt K1-ok.
A külső környezet, a tesztprogram, a fordított kód és a motor közötti oksági
kapcsolat továbbra nyitott. A meglévő numerikus küszöbök változatlanok.

## 1. Az eredeti executable natív Intel macOS-en

A [GitHub hivatalos runner-listája](https://docs.github.com/en/actions/reference/runners/github-hosted-runners)
Intel architektúrájú `macos-15-intel` standard futtatót is biztosít.
A [külön Intel-workflow](../.github/workflows/k1-native-intel.yml) ezt használja.
Nem szükséges hozzá Robert saját Intel Macje.

A meglévő [archívumellenőrző runner](../scripts/replay-k1-original.py)
új `--execution-mode native-intel` módot kap. Az alapértelmezett
Apple Silicon / Rosetta kampány és a 14 folyamat eredeti terve változatlan.

Intel módban:

- A rendszer Darwin/x86_64, a runner X64 és a CPU vendor GenuineIntel kell legyen.
  A `sysctl.proc_translated` eredménye megmarad; a jelzett fordított végrehajtás elutasított.
  Intel gépen a hiányzó OID üres kimenetet adhat sikeres sysctl-kilépéssel is:
  ez nem Rosetta-bizonyíték. Az eredeti ellenőrzés ezt hibásan elutasította,
  még a SAWSTAR elindítása előtt; a kezelése javított és négy regressziós kontroll védi.
- Az eredeti két archívum ZIP-, forrás-, executable-, cache- és loghashainak
  ellenőrzése továbbra megelőzi a futtatást. Ugyanaz a `3228d4d2…` hashű
  Universal executable fut; nincs újrafordítás vagy újraszignózás.
- Csak az x86_64 folyamatok futnak: waveform 0/1/2/3, majd Triangle cold-1/2/3.
  A Triangle parancsa továbbra `--diagnostic-waveform 2`.
  Ez az eredeti 14 folyamat x86-részsorozata; ARM-kód nem fut Intel gépen.
- Két archívumeredet, eredetenként 7 folyamat: összesen 14 folyamat /
  teljes siker esetén 1680 esetmegfigyelés.
- Minden eredmény megőrzött, a cold-run-0 alap nem cserélhető későbbi sikerre.
  Az eredeti negatív log változatlanul az új csomag része marad.

Ez natív Intel utasítás-végrehajtás egy GitHub által hosztolt macOS-környezetben.
A macOS 15, rendszerkönyvtárak, runner és fizikai/virtuális környezet eltérnek
az eredeti macOS 14/Apple Silicon/Rosetta környezettől. A különbséget nem lehet
egyetlen változóra, kizárólag Rosettára visszavezetni.

A 19 parser-kontroll ellenőrzi az új x86-részsorozatot, a régi alapértelmezett
terv változatlanságát, az ismeretlen mód elutasítását és a CSV/fingerprint guardokat.
A sima Triangle-grid csak explicit `diagnostic=False` módban fogadható el
9 oszloppal; az eredeti replay továbbra 13 oszlopot követel.

## 2. ASan/UBSan a tényleges kalibrált Preview-páron

A meglévő `quality.yml` már futtat általános Linux ASan/UBSan ellenőrzést.
Az azonban nem kapcsolja be a `SAWSTAR_PREMIUM_PREVIEW` módot; nem azonos a
kalibrált Preview Reference/Candidate párral. Az új
[sanitizer-workflow](../.github/workflows/k1-sanitizers.yml) ezt a konkrét
lefedettséget adja hozzá, a régi quality jobok megváltoztatása nélkül.

Előre kijelölt négy konfiguráció:

| Forrás | Natív ARM64 / macOS 14 | Natív Intel / macOS 15 |
| --- | --- | --- |
| Eredeti `fb3114eb…` | Új, instrumentált build | Új, instrumentált build |
| Aktuális pontos PR-head | Új, instrumentált build | Új, instrumentált build |

Minden konfiguráció ugyanazt a kalibrált `sawstar_premium_native_pair` targetet
építi, plugin nélkül. A tesztelt forrás és a kampányrunner külön checkoutot,
külön SHA-mezőt kap. A runtime natív architektúráját és a translation probe-ot
ellenőrizzük; ez a kampány nem használ Rosettát.

A C++ fordítások és a végső link ASan/UBSan instrumentációval készülnek:
`-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer`.
A runner ellenőrzi a fordítási adatbázis minden C++ egységének kapcsolóit,
valamint az engine executable ASan/UBSan runtime-szimbólumait.
Nincs általános fast-math, epsilonemelés vagy FP-módváltás.

A [külön kontroll-executable](../tests/k1_sanitizer_controls.cpp) szándékos
heap-túlindexelést, illetve előjeles egész túlcsordulást végez, külön
folyamatokban. Mindkettőnek hibával és az elvárt konkrét sanitizer-jelentéssel
kell kilépnie. Egyszerű crash vagy sikeres kilépés nem elfogadott kontroll.
Ez a program soha nem linkelődik a motorhoz vagy pluginhoz.
A heap-kontroll az első tömbön túli elemet olvassa: a korábbi, távolabbi
próbacím az első ARM-futásban nem adott ASan-jelentést (exit 0), ezért a
kampány helyesen még a motor előtt megállt. Ez kontrollhiba, nem K1-találat.
A kontrollhoz a fordítási object is megmarad, hogy a dSYM ne ideiglenes,
már törölt object-fájlra hivatkozzon.

Csak a működő runtime-kontrollok után indul a négy teljes waveform-grid,
egy-egy előre kijelölt folyamatban. Az eredeti Triangle a régi
`--diagnostic-waveform 2` úton, az aktuális forrás a támogatott `--waveform 2`
úton fut. Mindkét formátum teljes 120 esetes, sorrendhelyes ellenőrzést kap.
A négy konfiguráció együtt 1920 grid-esetmegfigyelést jelent teljes siker esetén.

A sanitizer pozitív kontrolljainak hibái elvártak; a motorban észlelt sanitizer-
hiba vagy numerikus eltérés megbuktatja a kampányt. Nincs suppressions-lista
vagy hibás motorfutás sikeresként kezelése. A jobok fail-fast nélkül, legfeljebb
két párhuzamos sanitizer-konfigurációval futnak.

A [runner](../scripts/run-k1-sanitized.py) minden logot, hibakategóriát,
fordítási adatbázist, cache-/executable-/loghashokat és környezetadatot megőriz.
A dSYM-ek és a linkparancs is artifactba kerülnek. Az öt
[validator-kontroll](../tests/k1_sanitized.py) a runtime-ellenőrzés téves
sikerességét és a nem megfelelő hibajelentéseket utasítja el.

## 3. Mit jelent és mit nem jelent az eredmény?

- Az eredeti bináris natív Intel-eltérése új negatív bizonyíték; összevetendő
  az eredeti konfigurációval, mintával és bitmintákkal.
- Csak a Rosetta-futásban ismert eltérés a fordított útvonalra irányítja a
  figyelmet, de nem bizonyít Rosetta-hibát.
- ASan/UBSan találatnál először a konkrét stack trace, operandus és forráshely
  vizsgálandó. A sanitizer üzenete önmagában nem bizonyít kapcsolatot a K1-gyel.
- A sanitizer build új executable, más optimalizációval, memóriaképpel és
  időzítéssel. Sikere nem javítja meg a régi binárist.
- ASan/UBSan nem zár ki minden inicializálatlan olvasást, versenyhelyzetet,
  logikai hibát vagy környezeti problémát. MemorySanitizer nem része ennek.
- A macOS sanitizer-kampányban `detect_leaks=0`: memóriaszivárgást nem
  minősítünk. A meglévő Linux quality kampány saját leak-ellenőrzése megmarad.
- Egyik kampány sem natív REAPER/plugin/audiointerface/CPU-elfogadás.

Ha nem lesz konkrét negatív találat, K1 megőrzött, nyitott, nem újra előidézett
Rosetta alatti numerikus anomália marad. Következő prioritás a natív ARM64 és
Intel működés/host-elfogadás, nem korlátlan újabb Rosetta-ismétlés.

## Elsődleges módszertani források

- [GitHub runner-architektúrák és környezetek](https://docs.github.com/en/actions/reference/runners/github-hosted-runners)
- [Clang AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html)
- [Clang UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)
