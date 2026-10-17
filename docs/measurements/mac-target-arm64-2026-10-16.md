# Mac célgépes motoridő-mérés — 2026-10-16

**A gyűjtés és nyers ellenőrzés kész; a CPU-/natív elfogadás nyitott.**

2026-10-17-i kiegészítés: az azonos forrású Windows-kontroll és a két teljes
nyers archívum összevetése is kész. [Eredmény és fennmaradó kapuk](windows-target-x64-2026-10-16.md).
Az alábbi eredeti Mac-mérési összefoglaló változatlan történeti eredményt őriz.

Ez a Premium lookup teljesmotor-referencia (`rate-lookup-seven-saw`) és az
oszcillátor-dispatch jelölt (`rate-lookup-saw-dispatch-v1`) offline összevetése.
Nem Classic/Premium REAPER CPU%-mérés, és nem mért DAW-dropout.

## Eredet és ellenőrzés

- Forrás: `2f401939db95c4da51e80d041e8911d0258f3b39`; a checkout a gyűjtés után is tiszta.
- DaisySP: `599511b740f8f3a9b8db72a0642aa45b8a23c3a3`, tiszta pinned submodule.
- Mac mini `Macmini9,1`, Apple M1, 16 GB; macOS 26.7.1, AppleClang 21.0.0, AC.
- Friss Release build, hat CTest kvalifikáció és a komplex/négyszeres hullámforma-kontroll sikeres.
- 2026-10-16 **17:22:06–17:59:15 Europe/Budapest** (15:22:06–15:59:15 UTC), 37 perc 9 másodperc.
- 32/32 grid, 288 külön cella, 2 359 296 nyers blokk. Két teljes, fordított sorrendű kör;
  külön referenciaismétlés és jelölt, statikus/modulált terhelés, Saw/Square/Triangle/Sine,
  48/96/192 kHz × 32/64/128 minta. Cellánként 4096 blokk útvonalanként.
- A pinned checkout `review-dispatch-engine-target.py --verify-raw` ellenőrzése sikeres (exit 0):
  minden fájlhash, kampánysorrend, nyers szekvencia, percentilis, szigorú időkeret-túllépés és páros eredmény újraszámolva.
- Robert Caffeine.app használatáról és más programok mellőzéséről számolt be. A teljes háttérterhelés
  nem volt függetlenül ellenőrizhető; `background_load_controlled=false`, nincs CPU-affinitás.

## Eredmények körönként

A tartományok a kör 72 cellájának legkisebb/legnagyobb **páros mediánarányai**.
Az arány jelölt/referencia, a kontrollnál referenciaismétlés/referencia; 1 alatt gyorsabb.
Nem egyesített nyers percentilisek, nem egyetlen átlagos CPU%. A túllépésszámok a 72 cella
összegei, útvonalanként 294 912 blokkból; cellaszinten a csatolt JSON őrzi őket.

| Kör | Összevetés | p50 arány min–max | p99 arány min–max | Referencia / vizsgált túllépés |
| --- | --- | --- | --- | --- |
| 1 | reference-repeat | 0.992153–1.005692 | 0.943913–1.054599 | 1786 / 1985 |
| 1 | candidate | 0.924875–1.033740 | 0.924512–1.075380 | 2529 / 3811 |
| 2 | reference-repeat | 0.992659–1.008938 | 0.937040–1.067323 | 1680 / 1904 |
| 2 | candidate | 0.926655–1.020954 | 0.919007–1.050148 | 1640 / 1897 |

A Saw p50-idő minden mért cellában javult: az első körben 4,13–7,51%, a másodikban
3,97–7,33%. Ettől a p99 és a többi hullámforma még nem lett mindenhol jobb.
A legrosszabb jelölt p50-arány 1,033740 (+3,37%), a p99-arány 1,075380 (+7,54%).
Utóbbi az első kör statikus Triangle, 192 kHz / 64 mintás cellája:
20 referencia- és 395 jelöltblokk lépte túl az időkeretet a 4096-ból.
A második kör legrosszabb jelölt p99-aránya 1,050148 (+5,01%), statikus Triangle,
96 kHz / 128 minta, 0 / 4 túllépéssel. A rosszabb első kör is megmarad.

Az azonos referencia ismétlése is mutat szórást (p99 maximum +5,46%, illetve +6,73%),
és mindkét kontrollkörben voltak túllépések. Ezt nem vonjuk le a jelöltből,
és nem tekintjük bizonyítéknak arra, hogy minden lassulást a háttérterhelés okozott.
Nem történt újramérés kedvező eredményért, zajlevonás vagy küszöblazítás.

## Döntés és következő kapuk

A mérés értékelhető, a Saw mediánjavulása megismétlődött, de általános CPU-elfogadás
és production-promóció nincs: a p99/túllépések, alternatív hullámformák, valamint a
támogatott magasráta-/pufferpolicy még döntést igényelnek. Következik az azonos forrású
Windows célgépes kontroll és a natív REAPER/dropout-vizsgálat. A filterminőség, production-
integráció, preset/lifecycle és release-kapuk külön maradnak.
`native_host_acceptance=false`, `production_promotion_allowed=false`.

## Megőrzött bizonyíték

[Tartós eredetjegyzék és mind a 288 cella](mac-target-arm64-2026-10-16.json).
A teljes eredeti gyűjtés, gépadatok, lezárási eredetjegyzék és nyersellenőrzés lezárt ZIP-ben
helyileg és külön tartós privát másolatban megőrizve; a nagy nyers CSV-k nincsenek a Gitben.
Az archívum nem nyilvános release-artifact, ezért ebből a repositoryból önmagában nem tölthető le.

- Fájlnév: `SAWSTAR-M1-target-2026-10-16.zip` (15226695 byte).
- ZIP SHA-256: `98aaf722af0f28da906a4c11d3f9014b584134ff5373b798314765c4f199f215`.
- Eredeti manifest SHA-256: `e46cf1cb1ca2274dacdb2899c34c0a3cfc42a73c9522a0475813c5b78d123548`.
- A hashok belső sértetlenséget/azonosságot ellenőriznek, önmagukban nem hitelességi aláírások.
- A korábbi negatív CI- és CPU-bizonyítékokat ez az új mérés nem törli vagy írja felül.
