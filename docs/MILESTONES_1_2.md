# 1–2. mérföldkő — integrált tesztalap és MIDI-ütemezés

2026-09-19. Helyi fejlesztés, nem kiadás.

## 1. Teljes tesztalap: kész helyben

CMake 4.4.3 és Ninja 1.13.2 ideiglenes helyi eszközökkel,
AppleClang 21 és macOS SDK használatával Release build készült.
SAWSTAR_CHECK_DAISYSP=ON, BUILD_TESTING=ON.

A mainből átvett 42 teszt, a hat megőrzött új motorregresszió,
a benchmark parser és az új MIDI-teszt együtt: **50/50 PASS**.
Teljes futás 30,73 s. A változó blokkméretes ellenőrzés hozzáadása után
az érintett MIDI-teszt külön újrafuttatva szintén sikeres.
A telepített Xcode make shim licencpárbeszédét nem fogadtuk el automatikusan;
helyette Ninja és a fordító közvetlen útvonala működött.

## 2. Tényleges MIDI-komponens: kész helyben

A src/midi/BlockMidiQueue.h fix 1024 elemű, audio-szál által kezelt sora
ugyanaz a kód, amelyet a SAWSTAR::ProcessMidiMsg és ProcessBlock használ.
Nem tesztcélú másolat. Nincs új dinamikus memóriafoglalás a sorban.

Megőrzött szerződés:
- azonos offset esetén stabil érkezési sorrend;
- frame 0 és frames−1 az adott blokkban feldolgozódik;
- offset==frames a következő blokk első mintájára kerül;
- távolabbi események offsetje a feldolgozott blokkhosszal csökken;
- üres blokk nem fogyaszt MIDI-t és nem lépteti az időt;
- negatív offset a blokk elejére korlátozva; nem pozitív blokkhossz
  nem lépteti a sort;
- 1024 esemény megengedett, 1025-nél az egész bizonytalan csomag eldobva,
  ARP/sustain/hangok/FX globális vészleállítással elnémítva;
- overflow recovery üres blokknál is megtörténik, egyszer;
- új hangok a recovery után ismét megszólalnak;
- mono kimenet L/R átlaga, stereo kimenet az eredeti L/R.

A túlcsordulás bend/mod értékeit nem reseteli: ez az engine meglévő
All Sound Off viselkedése. Eltávolítottuk a wrapper korábbi átmeneti,
megtévesztő Bend=8192/Mod=0 telemetriáját, amelyet a blokk végén az engine
régi értékei azonnal felülírtak. A kijelzés az engine tényleges állapotát
követi. A CC121 controller reset külön, későbbi vizsgálat marad.

## Ellenőrzések

block_midi_queue:
- rendezetlen bemenet, stabil kötések, blokkhatárok, jövőbeli események;
- nulla/egymintás blokkok, Clear, kapacitás és overflow;
- sustain, ismételt hang, Note On/Off azonos és szomszédos offseteken;
- több csatorna, CC123 és CC120;
- 16 csatornás overflow aktív ARP Hold/sustain mellett, csend és újraindítás;
- 44,1/48/96 kHz és Poly/Mono/Legato: ugyanaz a MIDI-timeline
  1/32/64/512/2048 és ciklikusan változó blokkokkal bitazonos;
- mono/stereo/nulla csatornás kimeneti írás.

A MIDI-teszt külön ASan+UBSan buildben is PASS. TSan nem került hozzá:
ez a sor szándékosan egy szál tulajdona. A pinned iPlug2
IPlugVST3ProcessorBase::Process a paraméter- és MIDI-feldolgozást az
ProcessAudio előtt végzi, az editor MIDI külön framework-soron érkezik.
A tempo/transport Set-hívások továbbra is ProcessBlock elején maradnak;
az arpeggiator meglévő tempo/transport tesztjei a teljes készletben futottak.

SAWSTAR_BUILD_PLUGIN=ON, Release: valódi macOS VST3 bundle fordítása és
linkelése sikeres. Nincs automatikus telepítés, kiadás vagy GitHub push.

## Határok

Ez nem teljes hostszimulátor, nem teszteli a teljes iPlug parameter-event
lánc sample-pontos automatizálását, a host-state visszatöltést vagy az UI/audio
összes párhuzamos helyzetét. Ezek a következő mérföldkövek részei.
Windows/macOS Universal CI, teljes sanitizer suite, validator és hallásos
REAPER-próba a platformos mérföldkőben marad. A mostani VST3 build a helyi
architektúrát ellenőrzi; a meglévő telepített hangszer nem változott.

A jelentések alapján rögzített következő lépés a 3. mérföldkő:
paraméterátadás és projekt-visszatöltés. Most ennél megállunk a kérés szerint.
