# SAWSTAR 1.2.0 — kiadási végrehajtási terv

Állapot: **1.2.0 rc1 előkészítés**, 2026-11-05. Robert célja az 1.2.0 publikálása.
Az RC még nem végleges kiadás; kiadási dátum és tag nincs kijelölve.
A korábbi 1.1.0 rc1 nem jelent meg nyilvános kiadásként.

## Kiinduló, ellenőrzött forrás

Main: `ec110a3b8e2e63c06fda70a6bac3f8a051f0a19a` (#137).
Mind a négy workflow és mind a 11 job sikeres: workflow-validálás
42007938630, Windows 42007938760, macOS 42007938775, Code quality 42007938849.
Az ellenőrzéskor nincs nyitott PR. Ez a kiinduló main eredménye, nem az új RC-é.

- 30 beépített factory preset; a régi 24 értéke/indexe megőrzött.
- Hat új exportált snapshot hangzását Robert meghallgatta és jóváhagyta.
- A chorus ring-határhibája regresszióval javított (#135).
- Az ARP bypass felesleges rendezése megszűnt (#137).
- [Windows/macOS felhasználói működési visszajelzés](NATIVE_HOST_FEEDBACK_2026_11_05.md)
  megőrzött; a linkelt általános main buildhez kapcsolódik.
- A Premium production build explicit kapcsoló, kalibrált lookup referencia,
  32 minta latency. A CMake alapértéke továbbra Classic.

## Sorrend és lezárási feltételek

| Lépés | Feladat | Állapot / szükséges bizonyíték |
| --- | --- | --- |
| 1 | Verzió és dokumentáció egységesítése | 1.2.0 rc1, config/plist/About, magyar/angol PDF, új release notes; metadata- és csomagtesztek. Az aktuális PR CI-je külön ellenőrzendő. |
| 2 | Azonosított Premium RC csomagok | Aktuális head macOS Universal/Windows x64 build, validator, ZIP/payload hashok, dependency provenance, 252 factory eset platformonként. Korábbi RC eredményét nem másoljuk át. |
| 3 | Pontos RC natív ellenőrzése | Mac/Windows REAPER: CPU/dropout, ráta/puffer, filtermódok/Drive/rezonancia/automatizálás; projekt/state/preset/GUI/bypass/latency/életciklus. A hiányzó esetekhez célzott sajátgépes jegyzőkönyv szükséges. |
| 4 | Zenei és numerikus döntések | Hat új snapshot már jóváhagyott. Teljes végleges bank és konkrét RC elfogadása, filterminőség/támogatási tartomány; K1 megőrzött negatívjának rendezése. |
| 5 | Kiadási csomagolás | Csak az elfogadott motor/build publikálási útja; a jelenlegi általános kiadási build Classic, az explicit Premium RC külön út. A Premium release kiválasztását és elfogadási bizonyítékát nem helyettesíti a verziószám vagy a candidate mező törlése. |
| 6 | Végső publikálás | Pontos forrás és csomagok, kézikönyvek és release notes véglegesítése; teljes zöld PR/main; konkrét RC és kiadási tartalom jóváhagyása, majd tag/release és publikus letöltés ellenőrzése. |

A lépések sorosan követik egymást, egyszerre legfeljebb egy fejlesztési PR-rel.
CI-hibát ugyanabban a PR-ben diagnosztizálunk; nincs kedvezőbb eredményért
változatlan mérési retry, küszöblazítás vagy natív eredmény feltételezése.
Nem kezdünk új általános optimalizálási kampányt a kiadási kapuk helyett.

## Megőrzött korlátok

[K1: 001847b Rosetta pre-FX 1 ULP eltérés](measurements/k1-passive-001847b-negative.json)
nyitott; a későbbi zöld futás és a külön post-FX chorus-fix nem gyökérok-bizonyítás.
A korábbi CPU-negatívok megmaradnak. A hat új preset jóváhagyását és az alapvető
Windows/macOS sikert nem kérjük újra, de nem terjesztjük ki hiányzó részletes mérésekre.

## Ellenőrzési napló

Implementált: 1.2.0 rc1 metaadat és kiadási dokumentáció.
Helyi tesztek, PDF-ellenőrzés és távoli CI pontos eredménye az előkészítő PR
leírásában szerepel majd; nem jelöljük előre sikeresnek.
Natív 1.2.0 RC-elfogadás és publikálás: **nyitott**.
