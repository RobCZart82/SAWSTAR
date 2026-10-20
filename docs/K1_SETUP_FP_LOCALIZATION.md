# K1 — Setup-műveletek passzív FP-lokalizálása

Állapot: a #125 pontos-head CI-je sikeres; az IE API-intervalluma Reset()-re szűkült.
Nem igazolt K1-gyökérok vagy DSP-javítás. Alap: `16f024e3bad4a84cd21f22362694f711e18fdf07` (#124 után).

## Ellenőrzött CI-eredmény a #125-ben

A [39472339484-es workflow](https://github.com/RobCZart82/SAWSTAR/actions/runs/39472339484)
`4efe28cd7fd8b3b195ce36974ea847914680781c` headen sikeres.
Mind a 12 kijelölt folyamat exit 0; a két teljes grid 240 esetet ellenőrzött,
a hat rövid setup-próba audio/pre-FX bitazonossági smoke-ja is sikeres.

A részletes x86_64/Rosetta trace-ben az elsőként futó motor `Reset()`
hívása az IE megjelenésének legszűkebb megfigyelt API-intervalluma:

| Setup-sorrend | Motor / művelet | Előtte → utána |
| --- | --- | --- |
| Reference, Candidate | Reference / index 0 / Reset | `0x1f80 → 0x1fa1` |
| Candidate, Reference | Candidate / index 0 / Reset | `0x1f80 → 0x1fa1` |

A második motor már beállított sticky biteket örököl, így az új átmenet
hiánya nem bizonyítja, hogy ott nincs azonos aritmetikai esemény.
Ez az observed mód eredménye; önmagában nem azonosít gépi utasítást,
és nem kapcsolja az IE-t az eredeti K1 audioeltéréshez.

Artifact: 12286649829, ZIP SHA256
`188f4ec9b5ed33b3ece48ef601078e108a7a00f19410231212ecd0c8c68fc895`.
A fenti összesítés a befejezett job naplójára és a runner validációjára épül;
itt nem állítunk új, független helyi ZIP-auditot.

#125 beolvadt, main `bef9b8db7c36092485b544b7d1ed3ca9a71b20ce`;
11 PR-workflow / 26 job sikeres. A következő vizsgálat prioritása
[a natív Intel-kontroll és a kalibrált sanitizer-kampány](K1_NATIVE_AND_SANITIZER_CONTROLS.md),
nem a Reset() újabb mélyebb instrumentálása.

## Mi indokolja?

Az [eredeti executable visszajátszása](K1_ORIGINAL_BINARY_REPLAY.md) 28 folyamat /
3360 esetmegfigyelés alatt nem hozta vissza az audioeltérést.
A [#123 új FP-binárisa](K1_FP_OBSERVATION.md) viszont azt mutatta, hogy
az IE státuszbit már az első Reference-setup alatt beállhat. Ebben a
munkalépésben ennek a külön jelenségnek a helyét szűkítjük.

A kiválasztott eset az FP-grid első konfigurációja:
Triangle / 48 kHz / LP12 / Poly / FX OFF.
Ez **nem** az eredeti K1 HP12 / FX ON konfigurációjának reprodukciója.

## Mit változtat az implementáció?

A [közös setup-header](../tests/DispatchEngineSetup.h) az eddigi tesztgrid
API-hívásait, argumentumait és sorrendjét tartalmazza. A meglévő grid ugyanazt
hívja; a külön [mérőprogram](../tests/k1_setup_fp.cpp) egy passzív proxyval
veszi körül a hívásokat. Így nem két külön setup-listát tartunk karban.

A Synth/DSP/plugin forrása változatlan. Az új target
`sawstar_k1_setup_fp_probe` offline, EXCLUDE_FROM_ALL, nem linkelődik pluginhoz.
Az új header miatt az érintett régi workflow-k path-szűrői is frissülnek.
A helper kiemelése is új binárist eredményezhet; a fordítás bitazonosságát
nem feltételezzük. A normál teljes numerikus grid és a régi hashazonos
bináriskontroll külön bizonyíték marad.

## Három előre kijelölt mód ugyanabból az executable-ből

| Mód | Setup-sorrend | Megfigyelés |
| --- | --- | --- |
| `--coarse` | Reference, Candidate | Objektumok és teljes setup körüli határpontok |
| `--observe` | Reference, Candidate | Továbbá minden setup API-hívás előtt/után snapshot |
| `--candidate-first` | Candidate, Reference | Ugyanez, csak a setup-sorrend megfordítva |

Az objektumok létrehozása minden módban Reference, majd Candidate sorrendű.
A 17 kezdeti setter/reset hívást 16 Note On követi: **33 hívás motoronként**.
A trace tárolása fix kapacitású; nincs formázás, log-I/O vagy dinamikus
foglalás a hívások előtt/után rögzített két snapshot között.
Az adatok a teljes próba végén íródnak ki.

x86 alatt a raw/status/controls és C-kerekítés megfigyelt.
Natív ARM alatt MXCSR továbbra `NA`; ARM FPCR/FPSR közvetlenül nem mért.
Nincs státuszbit-törlés, FTZ/DAZ- vagy kerekítésváltás.
A már beállított sticky bit eltakarhat későbbi azonos eseményeket.

A setup után négy mintapár audio/pre-FX bitazonossági, véges- és kimenetvédelmi
smoke ellenőrzést kap. A render sorrendje minden módban Reference, Candidate,
tehát csak a setup-sorrend változik. Ez a rövid render nem a grid esemény-
menetrendje vagy teljes hangzási elfogadása. Az első mintapár négy snapshotja
külön is megmarad.

## Validáció és eredménycsomag

A [külön macOS 14 workflow](../.github/workflows/k1-setup-fp.yml) Universal
Release buildet készít. Mindkét architektúrán rögzített hat folyamatot indít:
meglévő FP-önteszt, teljes 120 esetes Triangle-grid a közös helperrel,
új probe-önteszt, majd a három setup-mód. Összesen 12 folyamat;
240 teljes grid-eset és hat rövid setup-próba.

A probe-önteszt a bitguardot (beleértve az előjeles nullát), a pass-through
hívást, megfigyelt/nem megfigyelt tárolást és a kapacitástúllépés elutasítását
ellenőrzi. A futás a két motor 33 valóban befejezett setup-hívását és 16 aktív
voice-át is megköveteli.

A workflow ellenőrzi a teljes gridet, a műveletnevek/indexek/sorrend pontos
egyezését, a snapshotok formátumát és az ARM `NA` jelölését. Összesíti az
újonnan megjelenő IE-bitet és a vezérlőváltozásokat; az IE hiánya nem workflow-
hiba, és egy IE-megfigyelés nem automatikus gyökérok-PASS.

Minden log, a két executable, cache, x86 disassembly, forrás-/OS-/compiler-
adat és a bináris/cache/loghashokat tartalmazó manifest megmarad.
Minden kijelölt folyamatnak külön eredménye van; numerikus/validációs hiba
nem helyettesíthető egy későbbi sikerrel.

## Következő döntés és korlátok

Ha a coarse mód is előállítja a korábbi setup-átmenetet, és az observed mód
egy konkrét hívás előtt/után szűkíti azt, annak implementációját és fordított
utasításait vizsgáljuk tovább. Először azt kell eldönteni, hogy a jelzőt
kiváltó operandus/művelet elvárt-e, és van-e kapcsolata tényleges audiohibával.

Ha az IE nem jelenik meg, vagy a megfigyelés változtatja meg a mintázatát,
az eredmény negatív/megfigyelésfüggő adat; nem tippelünk meg műveletet.
A proxy és az új target megváltoztathatja az inline-olást, kódgenerálást,
regiszterhasználatot és időzítést. A híváshatárok nem azonosítanak magukban
egyetlen gépi utasítást, és nem zárnak ki a híváson belül visszaállított
átmeneti módváltást.

K1 az új eredményektől függetlenül nyitott marad, amíg a valódi numerikus
eltérés oka és célzott javítása nincs bizonyítva.
