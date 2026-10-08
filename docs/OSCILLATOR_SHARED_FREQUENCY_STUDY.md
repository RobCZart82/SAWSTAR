# Oszcillátor frekvenciaszámítás megosztása

A korábbi cache-jelölt modulált Windows SAW-nál lassult, ezért production
bevezetése elhalasztott. Ez a külön kutatás ugyanazon mintában számítja ki
és osztja meg a frekvenciát az aktív hullámformák között. A shipping
SevenSaw, a teljes motor és a filter minőség/rátapolicy változatlan.

## Algoritmus és referencia

A `SharedFrequencySevenSaw` a main `e830145` SevenSaw algoritmusát követi.
A detune simítása után egyszer számolja ki a `min(hz*pitch*ratio, rate*0.45)`
értéket sávonként. Ezt kapja a SAW és az aktív alternatív hullámformák setterje;
a háromszög integrátora is ugyanebből számít fázislépést. A műveleti sorrend,
fázisok, hullámforma-súlyok, detune, normalizálás és pan megmaradnak.
Nincs minták közötti cache, plusz állapot vagy változásfigyelő ág.
A referencia és jelölt objektummérete fordításkor ellenőrzötten azonos.

Ez optimalizálási hipotézis: a compiler már megoszthatja az eredeti számítást,
és a kódelrendezés önmagában is változtathat időt. Egyetlen kedvező medián
nem indokol teljesmotoros vagy production aktiválást.

## Numerikus és mérési kapuk

Az `oscillator_shared_bit_identity` 96 jelenetben, 795 648 sztereó frame-en
követel float-byte egyezést. Hat ráta (8–384 kHz), négy hullámforma, statikus,
lépcsős, clamp/hibás vezérlő és folyamatos pitch-moduláció szerepel, továbbá
Snap, reinit, élő állapot másolása és hullámformaváltás. A jeleneteknek szólniuk
kell; a negatív kontroll szándékos hangeltérést utasít el.

A benchmark 32 oszcillátorbankot (16 voice OSC1+OSC2) mér, három rátán,
négy hullámformával, statikus és audio-rate pitch mellett, négy váltakozó
párral. Init és 0,25 s warmup kívül, 8192 frame bankfeldolgozás, pitch-setter
és energiadiagnosztika belül van a timeren. Ez izolált költség, nem teljesmotor
vagy REAPER callback. A 192 soros CSV compiled `shared-frequency-v1` markerét
a reporter külön ellenőrzi; a cache v2 eredménye nem címkézhető át erre.

A Windows/macOS Release workflow a bitazonossági és riportkontroll után
méri a párokat. Nyers CSV, forrás/compiler hash, kontraktusnapló és minden
lassabb cella megmarad. Nincs CPU pass/fail küszöb. Teljesmotoros és
production bevezetés kizárólag későbbi elfogadással történhet.

## Következő döntés

Először helyi Release/sanitizer és platformos eredmények. Következetes
izolált nyereség esetén külön lookup-motor kontroll és modulált deadline
vizsgálat következik; vegyes vagy rosszabb eredménynél a jelölt kísérleti marad.
A CPU/minőség kapu, a natív REAPER-próba és a tényleges filtercsere nyitott.
Linux/32 voice halasztott; a lezárt click/pop kutatás nem nyílik újra.

## Helyi numerikus ellenőrzés

Mac mini M1, Apple Clang Release: a 96 jelenet / 795 648 frame bitazonos.
A shipping SevenSaw, a korábbi cache és a hét Python riportteszt is sikeres.
A külön új jelölt ASan/UBSan alatt is megfelelt. A CPU-adat és a platformos
CI külön következő eredmény; ezek nélkül nincs teljesmotoros aktiválás.
