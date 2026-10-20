# K1 — Passzív FP-állapotmérés és renderelési sorrend

Állapot: implementált, helyileg és a `8f617709` CI-binárisán ellenőrzött diagnosztika; **K1 nem reprodukált,
oka/javítása továbbra ismeretlen**. Alap: main
`848e4fccc495a46d4e1c2f8b83970cb2cf70cb3e` (#122 után).
[Belső jelút-trace](K1_INTERNAL_TRACE.md),
[külső hipotézisek értékelése](K1_EXTERNAL_REPORT_REVIEW_2026_10_20.md).

## Mit mérünk, és mit nem változtatunk?

Az offline összehasonlító új `--fp-observe` kapcsolója mindkét motor
ProcessStereo() hívása előtt és után rögzíti az FP-pillanatképet. x86-on:
`raw = _mm_getcsr()`, `status = raw & 0x003f`, `controls = raw & 0xffc0`.
Külön mező a C `fegetround()` eredménye. Nem állítunk FTZ/DAZ-t vagy
kerekítést, nem töröljük a státuszjelzőket, nincs új FP-fordítási policy.

ARM64 natív futásban MXCSR nem érhető el: a raw/status/controls mező **NA**, nem
hamis nulla/PASS. A C-kerekítés megfigyelt, de ez nem ARM FPCR/FPSR-pillanatkép.

Esetenként hét határpont is készül: objektumlétrehozás/setup és reset körül.
A main/tail/reset mintákban számláljuk a motoron belüli, belépések közötti és
motorhívásokon kívüli eltéréseket. Legfeljebb négy státusz- és négy vezérlőpéldát
tárolunk esetenként; a teljes számlálók minden mintára vonatkoznak. Az adatok
kiírása a kampány végén történik, nem a megfigyelt motorhívások között.
Numerikus hibánál a négy aktuális pillanatkép még a kivétel előtt kiíródik;
félbeszakadt kampánynál az addigi teljes esetek `state=interrupted` jelöléssel
maradnak meg. A hibadiagnosztika saját FP-olvasása is megelőzi a formázást.

A külön `--candidate-first` csak megfigyelési módban használható, és kizárólag
a ProcessStereo() hívások sorrendjét fordítja meg. Az objektumok létrehozása,
Setup, MIDI/események és reset sorrendje változatlan: egy változót vizsgálunk.
A meglévő teljes numerikus grid, bitazonosság, nemvéges-/kimenetvédelem és
életciklus-ellenőrzések megmaradnak. A Synth/DSP kódja e munkában nem változik.

## Helyi mért eredmény — macOS arm64 / AppleClang 21

Ugyanaz az új Universal executable, architektúránként egy baseline, egy
Reference→Candidate megfigyelt és egy Candidate→Reference megfigyelt Triangle-
folyamat. Összesen **720 esetmegfigyelés**. Minden folyamat exit 0, minden
output/pre-FX hibamező nulla; az adott architektúra három teljes CSV-je bájtra egyezik.
A CSV hibametrikákat tartalmaz: egyezése nem a külön folyamatok teljes audiojelének
hashazonossága. A bitazonossági ellenőrzés minden folyamaton belül a két motorra vonatkozik.

Rosetta/x86_64 alatt az első grid-eset (48 kHz, LP12, Poly, FX OFF), main frame 0:

| Renderelési sorrend | Első motor előtt | Első motor után | Második motor előtt/után |
| --- | --- | --- | --- |
| Reference → Candidate | `0x1fa0` | `0x1fb2` | `0x1fb2` / `0x1fb2` |
| Candidate → Reference | `0x1fa0` | `0x1fb2` | `0x1fb2` / `0x1fb2` |

A megfigyelt státuszváltozás PE → PE+DE+UE (`0x20 → 0x32`) **az elsőként futó
motort követi**. Mindkét sorrendben minden megfigyelt vezérlőmező `0x1f80`,
C-kerekítés 0; vezérlőeltérést/átmenetet nem mértünk. A jelzők ezután már
beállítottak, így a második motor nulla új státuszváltozása nem bizonyítja,
hogy ott nem fordult elő ugyanolyan aritmetikai esemény.

Az eredeti K1-konfigurációhoz érve (HP12/Poly/FX ON) a `0x1fb2` már a setup
előtt jelen volt, és ebben az új futásban sem keletkezett audioeltérés.
Az eredeti hibánál mért `0x1fb3` IE bitjének eredete továbbra nem ismert.
A megfigyelés **nem** a régi 197. mintabeli eltérés reprodukciója vagy magyarázata.

ARM64-en a C-kerekítés nem változott; az MXCSR-counterekből nem állítunk
ARM hardveres módokra/flag-ekre vonatkozó következtetést.

## CI-adat és ugyanazon bináris helyi kontrollja

A [39434092656-os FP-workflow](https://github.com/RobCZart82/SAWSTAR/actions/runs/39434092656)
a `8f6177091fcf2590e7b0509a85bcd96a9d3f6e08` forrást vizsgálta,
macOS 14.8.9 / arm64 hoston, AppleClang 15 / Xcode 15.4 fordítóval.
A 12272015564-es artifact ZIP-, executable-, cache- és mind a húsz loghasha
külön ellenőrzött. A hat teljes grid 720 esete nulla output/pre-FX eltérést
adott; a négy diagnosztikai önteszt is sikeres.
[Mért manifest és helyi visszajátszás](measurements/k1-passive-fp-ci-2026-10-20.json).

A CI x86_64 naplója a helyi AppleClang 21-es buildtől eltérő setup-státuszt mutat:

| Első eset határpontja | Reference → Candidate | Candidate → Reference |
| --- | --- | --- |
| Objektumok létrehozása után | `0x1fa0` | `0x1fa0` |
| Reference setup után | `0x1fa1` | `0x1fa1` |
| Candidate setup után | `0x1fa1` | `0x1fa1` |
| Első render előtt → után | `0x1fa1 → 0x1fb3` | `0x1fa1 → 0x1fb3` |
| Második render előtt → után | `0x1fb3 → 0x1fb3` | `0x1fb3 → 0x1fb3` |

Az IE (`0x01`) ebben az új binárisban már az első Reference-setup
határpontjai között megjelenik, még a ProcessStereo() előtt. A setup sorrendje
nem fordult meg. Ez az intervallum a Reset/setter/MIDI-hívásokat foglalja magába;
a pontos utasítást nem azonosítottuk. Az első render további DE+UE bitjei továbbra
az elsőként futó motort követik. A teljes megfigyelt x86 vezérlőmező `0x1f80`,
C-kerekítés 0; audioeltérés egyik sorrendben sincs. A régi hibanaplóban látott
`0x1fb3` így **önmagában nem K1-hibajelző**. Az eredeti bináris IE-bitjének
keletkezését ettől még nem bizonyítottuk.

A CI-ből letöltött, `0784efd241afaea03a938b8a631b697906619a362c77571d40ff8371e2b6075c`
SHA256-ú Universal executable-t újrafordítás nélkül, helyi macOS 26.7.1 alatt,
x86_64/Rosetta módban is futtattuk (`--waveform 2 --fp-observe`). A 120 eset
hibametrikái nullák; a teljes FP-napló bájtra azonos a CI naplójával
(SHA256 `282a7649bef0bc0df6a54b870dde0187f8329c94a15b5bd5be9c59640b468e91`).
Ez az új bináris setup-jelenségének második környezetben való megfigyelése.
Nem különíti el a compiler, kódgenerálás, könyvtárak és környezet szerepét,
és nem az eredeti K1-bináris vizsgálata.

A #123 másik workflow-jának első piros eredménye külön infrastruktúrahiba:
[39434092345, tanh/macOS job](https://github.com/RobCZart82/SAWSTAR/actions/runs/39434092345).
A tesztek és a 288 összesítő / 73 728 nyers blokk validálása sikerült,
az artifact véglegesítése `404 artifact not found` hibát adott. Csak ezt az
egy jobot indítottuk újra; a küszöbök és tesztek változatlanok. Ez nem K1-reprodukció.
Az itt leírt CI-bizonyíték a fenti pontos headre vonatkozik; minden új commitnak
saját friss CI-kapuja van.

## Ellenőrzések és bizonyíték

- Normál Release CTest: **141/141 sikeres**, az új FP-önteszttel.
- Az FP- és hibadiagnosztikai önteszt mindkét architektúrán sikeres: szintetikus
  státusz-/vezérlőeltérés, fordított sorrendű attribúció, tárolási korlát, NA-jelölés,
  tényleges passzív olvasás és a numerikus hiba előtti négy snapshot ellenőrzött.
- A meglévő belső Synth-trace ON módban is fordult; az önteszt mindkét szeletén
  sikeres, egy x86_64 Triangle-grid 120/120 nulla eltérés. Ez külön integrációs kontroll.
- A helyi x86 disassemblyben a renderpárok körüli külön STMXCSR-olvasások
  megmaradtak. Ez a helyi fordítás vizsgálata, más compiler/binárisra nem garancia.
- [Helyi manifest, összesítők és első pillanatképek](measurements/k1-passive-fp-local-2026-10-20.json).
  Módosított munkafa, nem committed CI-eredmény; kód-/bináris-/loghashokkal.

A [külön FP-workflow](../.github/workflows/k1-fp-observation.yml) ugyanebből az
egy, belső Synth-trace nélküli binárisból indítja a három előre kijelölt módot
mindkét architektúrán; megőrzi a CSV-t, FP-naplót, executable-t, cache-t és a
környezet-/commit-/hashmanifestet. A futásidőhatár és hiányos grid hiba.
A státuszváltozásokat megfigyelésként tárolja, nem automatikus K1-gyökérok-PASS-ként.

## Megmaradó korlátok és következő döntés

Ez új bináris; a helyi és CI-környezetek sem helyettesítik az eredeti hibás futást; a read-only megfigyelés is módosítja a fixture
kódját, időzítését és hívásmintáját. A határpontok nem zárnak ki a motoron belül
átmenetileg módosított, majd visszaállított FP-módot. A szövegformázás/validáció/
eseménykezelés közötti változás nem tulajdonítható automatikusan a motoroknak.
Nem mértünk közvetlen ARM FPCR/FPSR-t, és nem futott FTZ/DAZ ON kampány.

Következő nyomozati sorrend:

1. Az eredeti, megőrzött `3228d4d2…` hashű bináris véges, előre kijelölt
   kampánya ugyanazon macOS 14/Rosetta környezetben, hash- és környezetnaplóval.
   Az eredeti teljes gridsorrendet meg kell őrizni; a csak Triangle-t futtató
   próba más előzményt hoz létre. A sikeres kontroll is megőrzendő, nem lezárás.
2. Az új bináris IE-jelenségének külön lokalizálása: setup-határok finomítása
   Reset/setter/MIDI között, majd az első érintett művelet és disassembly
   ellenőrzése. Ez önálló diagnosztikai adat, nem automatikusan K1-gyökérok.
   Az új mérőkód külön bináris, ezért hashazonos régi kontroll szükséges.
3. Ha új audio-divergencia jelentkezik, a négy FP-pillanatképet a belső
   jelút első eltérésével összevetni. FTZ/DAZ ON/OFF csak páros diagnosztikai
   kísérlet legyen, változatlan numerikus kapuval; az ON siker OFF-reprodukció
   nélkül nem okbizonyítás és nem production-javítás.

A sikeres új próbák nem oldják fel K1-et és nem engedélyeznek production-promóciót.
