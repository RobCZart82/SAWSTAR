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

Távoli workflow-futtatás, Windows build, teljes VST3-build és valódi
REAPER-próba ebben a körben nincs; ezek külön kapuk.
