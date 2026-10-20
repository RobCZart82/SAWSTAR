# K1 — Passzív FP-állapotmérés és renderelési sorrend

Állapot: implementált, helyileg ellenőrzött diagnosztika; **K1 nem reprodukált,
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

Ez új bináris és helyi környezet; a read-only megfigyelés is módosítja a fixture
kódját, időzítését és hívásmintáját. A határpontok nem zárnak ki a motoron belül
átmenetileg módosított, majd visszaállított FP-módot. A szövegformázás/validáció/
eseménykezelés közötti változás nem tulajdonítható automatikusan a motoroknak.
Nem mértünk közvetlen ARM FPCR/FPSR-t, és nem futott FTZ/DAZ ON kampány.

Következő: pontos-head CI-nyersadatok kiértékelése. Valódi negatív eredmény esetén
a pillanatképet az első belső jelút-eltéréssel összevetni; ennek hiányában az
eredeti bináris kontrollált, hashazonos vizsgálata marad indokolt. A sikeres
új próbák nem oldják fel K1-et és nem engedélyeznek production-promóciót.
