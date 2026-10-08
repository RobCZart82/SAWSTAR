# Oszcillátor-frekvencia cache — kutatási jelölt

A #86 beolvadt a `645cab8` main-ba, mind a 35 exact-head ellenőrzés
sikeres. A FIR-kibontás nem hozott igazolt teljesmotoros CPU-nyereséget.
A komponensprofil szerint az OSC1/OSC2 oszcillátorbank költsége is érdemi,
különösen magas rátán. Ez a következő, izolált optimalizálási hipotézis.

## Változás és referencia

A külön `experimental_oscillator::CachedSevenSaw` a main `SevenSaw`
algoritmusát használja, de csak változó frekvencia-byteoknál hívja újra
az egyes DaisySP oszcillátorok `SetFreq` függvényét. A rögzített DaisySP
commit `599511b740f8f3a9b8db72a0642aa45b8a23c3a3`: a setter frekvenciát
és fázislépést ír, nem indítja újra a fázist. A cache csak az ismételt,
azonos érték beírását kerüli el; nincs közelítő összehasonlítás vagy
vezérlésritkítás. A bitenkénti összehasonlítás a signed zero értékét is
megőrzi. Minden oszcillátor saját cache-t kap, Init invalidálja őket.

A detune, pitch, Nyquist-clamp, waveform súlyok, háromszög-integrátor,
pan és normalizálás változatlan. Inaktív hullámformák nem kezdenek
folyamatosan futni; a saját fáziselőzményük megmarad. Másoláskor a cache
és az oszcillátorállapot együtt másolódik. A külön jelölt 28 floatot,
112 byte cache-adatot ad az objektumhoz; ezt a költséget is mérni kell.

Az eredeti `src/dsp/SevenSaw.*` és a production Synth változatlan.
Referencia a main `645cab8` SevenSaw-forrása; a riport mindkét saját
implementáció és a rögzített DaisySP-fájlok byte-hash-eit rögzíti.
Ez még nem teljesmotoros integráció vagy realtime elfogadás.

## Numerikus kapu

A `oscillator_cache_bit_identity` 72 jelenetet, összesen 596 736 sztereó
frame-et vizsgál: hat rátát (8–384 kHz), négy induló hullámformát és
három gerjesztést. A statikus kontroll mellett hangmagasság/detune/mix/
waveform váltás, Nyquist-clamp, nulla és signed zero, subnormál frekvencia,
NaN/Inf vezérlők utáni helyreállás, Snap, reinit és élő állapot másolása
szerepel. Az output float-bytejai egyeznek; a jelenetek nem lehetnek
csendesek. A negatív kontroll szándékos hangeltérést utasít el.

Helyi MSVC 19.44 x64 Release, a rögzített valódi DaisySP-forrásokkal:
a numerikus kapu sikeres. Négy Python riportteszt sikeres, beleértve
az aszimmetrikus párok mediánarányát, hiányzó/duplikált grid, rossz
sorrend/bank/frame, nem véges mérés és eltérő páros energia elutasítását.
Windows/macOS Debug/Release és sanitizer kvalifikáció friss CI-kapu.

## Páros költségmérés

A benchmark 32 SevenSaw példányt használ, azaz 16 voice OSC1+OSC2
bankját modellezi. Nem tartalmaz envelope-ot, SUB/noise-t, filtert, FX-et
vagy host callbacket. Három ráta × négy waveform × statikus/modulált
pitch × négy váltakozó pár × két út: 192 nyers sor és 24 összesítés.

Mindkét út frissen indul; Init és 0,25 s warmup a timeren kívül van.
8192 frame-et mérünk, bankfeldolgozás és energiadiagnosztika belül;
modulált esetben audio-rate szinuszos pitch-setter is belül. Az ismétlési
sorrend páronként váltakozik. A riport a study/reference időarányok
mediánját számolja; nem a két független medián hányadosát.

Az `oscillator-cache.yml` a numerikus kapuk után méri a két platformot.
CSV, compiler-leírás, kontraktusnapló, JSON és Markdown artifact készül;
a JSON a CI-naplóban is megjelenik. Nincs CPU pass/fail küszöb.
Jobb izolált idő nem bizonyít teljesmotoros nyereséget; a modulált
kontrollban regresszió is lehetséges a cache-ellenőrzés többletköltsége miatt.

Következő döntés: platformeredmények értékelése, majd megfelelő eredmény
esetén külön teljesmotoros lookup-kontroll és modulált deadline mérés.
Production aktiválás, magasrátás policy és natív REAPER továbbra is
külön, nyitott kapuk; nem következnek a zöld izolált tesztből.
