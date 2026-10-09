# Izolált SAW-dispatch vizsgálat — 2026-10-09

## Hipotézis és hatókör

A komponensprofil alapján az oszcillátorbank 192 kHz-en érdemi költségcélpont.
A korábbi cache és frekvenciamegosztás Windows SAW-adatai kedvezőtlenek;
ezért új, független jelölt készül. A cél az inaktív alternatív hullámformák
hét oszcillátoron ismételt vizsgálatának kiváltása egy bankonkénti döntéssel.
Ez hipotézis, nem előre megállapított gyorsulás vagy gyökérok.

`SawDispatchSevenSaw` a shipping SevenSaw-ból indul, új állapot nélkül.
A súlyok simítása után, kizárólag pozitív SAW-súly és három nulla alternatív
súly esetén külön SAW-ciklust futtat. Minden más eset az eredeti általános
feldolgozás. A setterhívások, frekvenciaszámítás/clamp sorrendje, detune,
SAW-súly, pan és összegzés megmarad. Nincs cache, megosztott frekvencia,
új simítás, FIR/rátapolicy változás vagy production routing.

A visszatérő SAW csak akkor kerülhet a külön útra, ha az eredeti 1e-8
küszöb minden alternatívát lenullázott. Az inaktív alternatívák fázisa
és triangle-history-ja ugyanúgy megáll; visszakapcsoláskor megmarad.
Az új objektum mérete statikus ellenőrzéssel egyezik a referenciával.

## Numerikus kapu

`oscillator_dispatch_bit_identity`: a változatlan 96 jelenet / 795 648
sztereó frame-es grid, hat ráta, négy hullámforma, folyamatos pitch-moduláció,
detune/mix/váltás/Snap/reinit, élő másolat, NaN/Inf, előjeles nulla és
subnormális vezérlők. Ezen felül 602 115 frame hosszú átmeneti kontroll:
mindhárom alternatíváról SAW-ra, a teljes súlylecsengésen túl, majd vissza,
folyamatos pitch- és detune-változtatással. Minden minta bitazonos és véges.
A negatív összehasonlítási kontrollnak külön el kell utasítania az eltérő jelet.

Helyi Windows MSVC 19.44 Release: mindkét grid sikeres.
A riportoló 14, a gyűjtő kilenc Python-tesztje sikeres, workspace-fixture
adapterrel a helyi tempkönyvtár-korlát miatt. Ez utóbbi szimulált parancsfuttatás,
nem valódi célgépes mérés. Friss platformos build/sanitizer/minősítés következik.

## Külön mérési kapu

Variáns: `saw-dispatch-v1`; target: `sawstar_oscillator_dispatch_benchmark`.
A mérőbank és hosszú kampány azonos a korábbi módszerrel: 32 bank, három
ráta, négy hullámforma, statikus és audio-rate pitch, 0,25 s warmup,
131 072 frame és nyolc váltakozó pár. Az alternatív hullámformák költségét
is mérjük: a dispatch új vizsgálata nem okozhat figyelmen kívül hagyott lassulást.

A külön Windows/macOS workflow két körben fordítja meg a teljes referencia-
és jelöltkampány sorrendjét. Minden kampány külön CSV/report/eredet marad.
A reporter ellenőrzi a variánst, teljes gridet és páros energiát; a gyűjtő
a kiválasztott executable-t és CTest-kaput használja. A korábbi default és
archív eredmények megmaradnak. Hibás variáns elutasított.

Nincs automatikus CPU-elfogadás, kontrollból levonás vagy outlier-kizárás.
Kedvező izolált eredmény után külön teljesmotoros modulált p99/túllépés,
azonos forrású célgépes ismétlés, minőség/rátapolicy és natív REAPER kell.
A shipping DSP és a következő kiadás 4–8. kapuja továbbra is külön feladat.
