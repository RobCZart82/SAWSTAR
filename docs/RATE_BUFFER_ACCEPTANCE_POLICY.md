# Ráta-/pufferpolicy — döntés-előkészítés

Állapot: **az offline bizonyíték összesítése kész; natív CPU-elfogadás és
végleges támogatási tartomány nincs kijelölve**. Ez nem módosítja a plugin
működését, nem korlátozza a host által kért rátát/puffert, nem vált kisebb
filterminőségre, és nem emeli production állapotba a kutatási jelöltet.

## Mihez van bizonyíték?

A 2026-10-16-i M1- és Windows-kampány forrása ugyanaz:
`2f401939db95c4da51e80d041e8911d0258f3b39`, DaisySP
`599511b740f8f3a9b8db72a0642aa45b8a23c3a3`; mind a 20 forráshash egyezik.
Platformonként 32 grid és 288 cella eredeti nyers ellenőrzése kész.
A mért út a Premium lookup teljesmotor-referencia és a Saw-dispatch jelölt.
Ez nem a production Classic/Premium Preview összehasonlítása és nem REAPER CPU%.
A gyűjtéskori háttérterhelés nem volt függetlenül ellenőrizhető.

A [teljes ráta-/pufferösszesítő](measurements/target-rate-readiness-2026-10-17.md)
36 sorban őrzi a két platformot, két kört és kilenc ráta/puffer párt.
Nincs körök vagy gépek közötti átlagolás. Egy sorban a négy hullámforma és
két terhelés mind a nyolc cellája szerepel; a referenciaismétlés külön
összehasonlítás, nem levonható „zaj”. A jelölt p50-tartománya és a legrosszabb
p99 mellett mindkét út túllépésszámlálója látható. Az eredeti 576 cella
változatlanul a két célgépes JSON-ban marad.

## A bizonyítékból következő jelenlegi korlátok

- A Windows célgépen 96/192 kHz-en minden vizsgált puffer és mindkét kör
  összes jelöltcellájában 4096/4096 blokk lépte túl az offline időkeretet,
  a referenciában is. A gyorsabb p50 ezt nem teszi elfogadottá. Ez az adott
  gép/forrás/teszt bizonyítéka, nem minden Windows gépre szóló ítélet.
- Windows 48 kHz / 128 mintán is maradt túllépés: az első kör nyolc
  jelöltcellájában összesen referencia 109 / jelölt 152, a másodikban 134 / 97.
  Emiatt a nagyobb puffer sem kaphat pusztán az offline adatból natív elfogadást.
- M1-en a második kör 48 kHz / 64 és /128 jelöltcelláiban nem volt túllépés,
  de az első körben rendre 4 és 2 megmaradt; a p99-romlások szintén láthatók.
  A kedvezőbb második kör nem írja felül az elsőt.
- Az offline blokkidő-túllépés nem azonos a DAW által jelentett xrun/dropout
  számmal. Ezekből nem következik, hogy a REAPER-ben hallható hiba történt.
  A natív próba hiánya viszont nem tekinthető sikernek.

## Következő végrehajtandó kapu

1. A [natív REAPER-protokoll](NATIVE_REAPER_CPU_ACCEPTANCE.md) szerint pontos
   plugin/buildazonosság és változatlan tesztkörnyezet rögzítése. A jelenlegi
   Preview külön tesztelhető, de nem igazolja a még nem integrált Saw-dispatch
   jelöltet vagy a későbbi production RC-t.
2. Első, előre kijelölt tartomány: **48 kHz / 128, majd /64**, M1 és Windows
   külön, a meglévő 1/8/16 hang, négy filtermód és statikus/modulált protokollal.
   Ez a teszt sorrendje, nem már elfogadott támogatási ígéret.
3. Mindkét A→B/B→A sorrend, GUI nyitva/zárva, tail/idle és minden hibás eredmény
   megőrzése. Szemmel olvasott CPU-tartományból nem készül p99.
4. A 32 mintás puffer és 96/192 kHz külön minőség/költség és natív
   tartalékvizsgálatot igényel. A mostani adat nem indokol automatikus
   ráta-limitet vagy hallható minőségcsökkentést.
5. A végső támogatási döntés platformonként nevezze meg a pontos buildet,
   audioeszközt/drivert, elfogadott tartományt, hibás/nem vizsgált tartományokat
   és az elfogadó személyt. Amíg ez hiányzik, `native_host_acceptance=false`
   és `production_promotion_allowed=false` marad.

Nem indítunk új lezárt célgépes kampányt kedvezőbb számokért, nem lazítunk
küszöböt és nem korrigáljuk utólag a régi nyers adatokat. A filterminőség,
production-integráció, GUI/state, preset/lifecycle és végső RC kapuk külön maradnak.

## Újragenerálás mérés nélkül

```sh
python scripts/report-target-rate-readiness.py --format markdown
python scripts/report-target-rate-readiness.py --format json
```

A parancs csak olvas és stdout-ra ír. Pontosan a rögzített forrású M1/Windows
összefoglalópárt fogadja el; hiányos/dupla/ismeretlen cellát, hamis platformcímkét,
eltérő forráshashokat, hibás számot és túllépésszámlálót elutasít.
Nem fordít, nem futtat motort, nem ír az archívumba és nem ellenőrzi újra a
2×2 359 296 nyers blokkot. A bemenetek SHA-256-a a kimenetben szerepel.
A JSON külön megőrzi a legrosszabb p99-cellákat, azonos legrosszabb értéknél
minden érintett terhelést/hullámformát. A kedvező, szintetikus tesztadat sem
válthatja az elfogadási flag-eket igazra.
