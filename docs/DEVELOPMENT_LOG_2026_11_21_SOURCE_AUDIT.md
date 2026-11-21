# SAWSTAR forrásaudit és ellenőrzési terv — 2026-11-21

Forrásalap: a [vizsgált main commit](https://github.com/RobCZart82/SAWSTAR/commit/fbf967a827c8d8ab9fa199dcae9508d35045e3f2).
Az A1 implementációja elkészült. A javítás elfogadása és a többi tétel nyitott.
A kiadott 1.2.2 a vizsgált main előtti build.

## Igazolt hibák és lezárási feltételek

### A1 — VST3 projektállapot és bypass

A rögzített iPlug2 `IPlugVST3State` kód és a SAWSTAR állapotkód szimulált
host streammel futtatva három külön hibát mutatott. Mentett bypass=1 esetén
a controller paramétere bekapcsolt, a processzor bypass állapota kikapcsolva
maradt. Két elutasított írás után nulla bájttal is sikeres mentést jelzett.
Érvényes, bypass toldalék nélküli állapotnál szigorú EOF streammel a hang
beállítása után hibát jelzett, sikeres üres olvasást adó streammel sikert.
A SAWSTAR dekódere elfogadja a toldalék nélküli formátumot. A teszt nem
tartalmazott natív DAW projekt-visszatöltést.

Következő lépés: a határt a rögzített adapter tesztjében lefedni. Ellenőrizni
kell a teljes és rövid írást, az írás/seek hibáját, a két EOF viselkedést,
a legacy és jelenlegi értékeket, illetve az érvénytelen toldalékot. Sikeres
visszatöltéskor a controller és a processzor állapota egyezzen. Elutasított
visszatöltés ne módosítsa a hangot, a bypasst vagy a GUI-skálát. A toldalék
nélküli formátumhoz legyen explicit, streamtől független kompatibilitási
döntés. A bináris formátum és a paraméterazonosítók megőrzését külön
ellenőrizni kell. Az elfogadáshoz Windows és macOS REAPER-ben mentett
bypass on/off projekttel újranyitás és első audioblokk ellenőrzése szükséges.

[IPlug2 állapothíd](https://github.com/iPlug2/iPlug2/blob/d54f69050f517e43b941d88c2a170f0a840b9ee4/IPlug/VST3/IPlugVST3_Common.h#L25) · [SAWSTAR dekódolás](../src/plugin/State.cpp) · [Plugin állapotkezelése](../src/plugin/SAWSTAR.cpp)

#### A1 javítás — 2026-11-22

A rögzített iPlug2 VST3 állapothíd ellenőrzi a payload és a bypass toldalék
írásának státuszát és tényleges bájtszámát. A visszatöltés egy menetben olvas,
a SAWSTAR v1 állapot maximális méretéig. Támogatja a sikeres nulla bájtos
EOF-ot és az SDK FileStream részleges utolsó olvasását. A dekóder
alkalmazás előtt ellenőrzi a teljes állapotot és az opcionális bypass
értékét. Siker után beállítja a processzor bypasst, és frissíti a
vezérlőn regisztrált paramétereket, a bypass paramétert is, ha létezik.
Érvényes, toldalék nélküli projektállapot visszatöltésekor a bypass
kikapcsol. A processzor jelzőjét a
host és az audiószál atomi művelettel éri el. A mentés a meglévő bináris
formátumot és paraméterazonosítókat használja.

A [regressziós teszt](../tests/vst3_project_state.cpp) a tényleges patchből
veszi az adaptert és a produkciós állapotkóddal fut. A régi adapterrel a
processzor bypass-visszatöltése, a régi dekóderrel az érvénytelen bypass
elutasítása elbukik. A javított próba ellenőrzi a bájtpontos mentést, a
korábbi állapotformátumokat, a rövid vagy hibás írást, valamint a sikertelen
visszatöltés utáni hang-, GUI- és bypass-állapotot.

Az iPlug2 a SAWSTAR hangszerhez nem regisztrál VST3 bypass paramétert a
vezérlőn. A vezérlő bypass-ágát ezért a szimulált adapterpróba fedi le.
A natív próbában a VST3 állapotban ténylegesen tárolt toldalékot kell
ellenőrizni. A host saját FX bypass gombja ettől külön vezérlés.

A helyi célzott CTest, az ASan/UBSan próba és az LF/CRLF patchteszt sikeres.
A Premium Production macOS Universal buildet közvetlen VST3 SDK hostpróba
töltötte be arm64 és Rosetta x86_64 alatt. A két futásban a tényleges
komponens bájtpontosan visszamentette a 0/1 bypass állapotot. Egy C4 hang
első 512 mintás blokkja 48 kHz-en bypass mellett néma volt, kikapcsolva
0,0485115 csúcsértékű. A távoli CI és a Windows/macOS REAPER
projekt-visszatöltés visszaigazolása nyitott.

### A2 — Kitartott hang Sustain visszaemelésekor néma marad

A Premium motorban 48 kHz-en, egy tartott C4 hanggal és 1, illetve 1500 ms
Decay mellett az Amp Sustain 1 → 0 → 1 után a mért kimeneti csúcs 0 maradt.
A motor még lenyomottnak és aktívnak jelölte a hangot. Új Note On hatására
visszatért a kimenet. Nulla sustain esetén a Decay Idle állapotba jut, amelyből
a folytonosan aktív gate nem indít új szakaszt. A filterburkoló ugyanezt az
ADSR-t használja. A filterre gyakorolt hallható hatás külön ellenőrizendő.

Következő lépés: ADSR és Premium render regresszió rövid és hosszú Decay
beállításokkal. Folytonos gate mellett a Sustain visszaemelése Note On nélkül
állítsa helyre a burkolót. A nulla tartomány maradjon néma, a valódi Note Off
utáni lecsengés szabadítsa fel a voice-t. Legyen külön filter cutoff próba,
valamint régi preset-, statikus sustain-, headroom- és átmeneti kattanáskontroll.

[ADSR](../src/dsp/Adsr.h) · [Filterburkoló](../src/dsp/FilterModulation.h) · [Élő paraméterátmenetek](PARAMETER_TRANSITIONS.md)

### A3 — Mintapontos VST3 bypass és normálparaméter-automatizáció

A [#155-ben](https://github.com/RobCZart82/SAWSTAR/pull/155) beolvadt adapterdiagnosztika rögzíti, hogy a 16. mintán induló és
48.-nál végződő bypass impulzus elveszik. Egyetlen, a 48. mintára kért
bekapcsolás már renderelés előtt érvényesül. A normál paramétereknél is az
utolsó blokkbeli pont kerül alkalmazásra. A diagnosztika a jelenlegi hibás
viselkedést rögzíti. A javításkor ezt kell lecserélni a kívánt
viselkedést ellenőrző renderpróbára.

Következő lépés: először bypass, egy külön változtatásban. A [16, 48) tartomány
legyen néma, előtte és utána az audio egyezzen a megfelelő kontrollal.
A némított minták közben a MIDI, ARP, voice és FX állapota haladjon tovább.
Azonos offset, blokkhatár, nulla mintás hívás, 32/64 bites kimenet és következő
blokkba vitt bypass állapot kapjon tesztet. Utána külön lépcső a normál
paraméterek pontjainak és interpolációs szabályainak meghatározása.
Elfogadás: adapter render és REAPER automatizációs render több blokkméret mellett.

[Bypass-diagnosztika](DEVELOPMENT_LOG_VST3_BYPASS_TIMING.md) · [Adapterteszt](../tests/host_midi_points.cpp)

### A4 — Arpeggiátor CC121 és sűrű MIDI

Működési hiba: 48 kHz-en, 120 BPM-en, negyedhangos lépéssel és 100% gate-tel
egy lenyomott C4 mellett a CC121 azonnal lezárta a generált hangot. A következő
1024 mintában nem jött új Note On. Az aktív hang leállítása csatorna szerint
történik, a fizikai gyökér és a pedál miatt megmaradt gyökér megkülönböztetése nélkül.

Következő lépés: külön eset a fizikailag tartott, pedállal tartott és HOLD miatt
reteszelt gyökérre. Ellenőrizni kell az eseménysorrendet, a csatornák izolációját,
a lépés fázisát, valamint a CC120 és CC123 meglévő leállítási viselkedését.

Terhelési hiba: az ismételt Note On teljes mintát újraépít és rendez. Az
offline arm64 próbában 32 tartott gyökér és 1024 sorba férő esemény esetén
az ARP és a MIDI-sor 2,05 ms-ot vett igénybe. Összevetésként egy 64 mintás
blokk időkerete 48 kHz-en 1,33 ms. Ez mesterséges stresszpróba a többi DSP nélkül.

Következő lépés: a fölösleges újraépítést csökkenteni úgy, hogy a lejátszási
sorrend, velocity, azonos offseten érkező események és túlcsordulás utáni
helyreállítás megmaradjanak. Előtte és utána azonos eseményárammal hasonlítsuk
össze a kimeneti MIDI-sorrendet és a blokkonkénti időt. A natív CPU/dropout
elfogadáshoz külön hostpróba kell.

[ARP eseménykezelés és Rebuild](../src/midi/Arpeggiator.h) · [MIDI-sor](../src/midi/BlockMidiQueue.h)

### A5 — Telepítő és Preview dokumentáció

A publikált 1.2.2 macOS DMG gyökerében az INSTALLATION.md hivatkozása hiányzó
SYSTEM_REQUIREMENTS.md fájlra mutat. A read-only ellenőrzés a tényleges
lemezképen történt. A [#154](https://github.com/RobCZart82/SAWSTAR/pull/154) a PKG-be kerülő dokumentumfát ellenőrzi.
A Preview ZIP dokumentumlistájából platformonként hat relatív linkcél hiányzik.
A terjesztett telepítési útmutató 1.2.0, 1.2.1 és 1.2.2 állapotokat kever.

Következő lépés: a DMG saját fájllistáját és a Windows/macOS Preview ZIP-ek
végleges dokumentumelrendezését a meglévő linkellenőrzőn átengedni. A hiányzó
anyagokat mellékelni vagy megfelelő online dokumentumra mutatni. A stabil
és Preview útmutatókat a tényleges kiadási állapotra igazítani. Elfogadás:
hibás relatív cél blokkolja a csomagot, a létrejött DMG és ZIP dokumentumai
olvashatók, és az EN/HU telepítési útmutató ugyanazt az állapotot adja meg.

Kiadás-előkészítési ellenőrzési rés: a letöltött telepítőket a draft készítése
előtt név és típus alapján veszi át a script, majd a kapott fájlokból állítja
elő az új SHA-256 jegyzéket. A csomagoló CI mellékeli a saját .sha256
fájljait. A következő csomagolási ellenőrzés hasonlítsa össze ezeket az
átvett fájlokkal, és próbálja ki hiányzó, hibás és egyező hash esetén.
A jelenlegi nyilvános telepítőknél sérült fájlra nincs bizonyíték.
[Kiadási tervezet előkészítése](../scripts/publish-release.py).

[macOS csomagolás](../scripts/package-macos.py) · [Preview csomagolás](../scripts/package-premium-preview.py) · [Telepítés](INSTALLATION.md) · [Rendszerkövetelmények](SYSTEM_REQUIREMENTS.md)

## Indokolt gyanúk és célzott ellenőrzések

### G1 — Presetváltás és VoiceMode koherenciája

A GUI külön írja az egyes paramétereket, az audióblokk több külön olvasásból
állítja össze a motorvezérlést. A VoiceMode-ot az ARP és az editor
állapotának rendezése, illetve a motorvezérlés is külön olvassa. Lehetséges
köztes paraméterkészlet vagy egy blokkig eltérő módértelmezés. Hallható hatás
jelenleg nincs igazolva.

Megkülönböztető próba: két ismert preset és kényszerített átfedés a két
VoiceMode-olvasás, illetve a GUI írásai között. Naplózzuk a blokk tényleges
paramétereit és a MIDI-tulajdonosi állapotot. Vegyes érték vagy eltérő mód
esetén a javítás egyetlen koherens blokkpillanatképet használjon a módváltás,
reset és motorparaméterek számára. Valós hostban is ellenőrizni kell az
automatizációt és a presetváltást tartott hang mellett.

[GUI paraméterírás](../src/plugin/SAWSTAR.cpp) · [Motorvezérlés](../src/plugin/EngineControls.h)

### G2 — K1 pre-FX eltérés és inicializálási kontroll

A megőrzött Triangle és Sine negatívok közül kettő már a pre-FX jelben eltér.
A páros teszt közvetlenül a két Synth motort futtatja, ezért a host bypass,
projektbetöltés és ARP nem része ennek az útvonalnak. Az egyik ellenőrizhető
hipotézis a `Voice{}` értékadás a `Synth::Reset()` elején, a DaisySP
oszcillátorok külön Init hívása előtt. Az upstream konstruktor üres.
A SAWSTAR Triangle saját nullázott előzményt használ, így az upstream
`last_out_` közvetlen használata nem magyarázza a K1-et. Jelenleg nincs
bizonyíték arra, hogy a resetnél talált minta okozta a negatívokat.

Egy előre rögzített offline kontrollban hasonlítsuk össze azonos típusú
motorpár Reset és első render eredményét az eredeti és a csak inicializált
mezőket másoló resetváltozaton. Maradjon meg az eredeti Reference/Candidate
kontroll, a grid sorrendje és a bitazonosság. Eltérésnél a legelső oszcillátor-,
keverő- vagy filterbemenet, illetve a filter első belső értéke és az előző öt
minta legyen elmentve, együtt az executable hashével és az FP állapottal.
Azonos első bemenet mellett eltérő FIR/SVF érték a megfelelő filterműveletre
szűkítené a vizsgálatot. Az újabb sikeres kontroll önmagában nyitva hagyja a K1-et.

[K1 előzmények](K1_AI_DEBUG_HANDOFF_2026_11_08.md) · [Mély trace](K1_FILTER_STATE_FIRST_DIVERGENCE_2026_11_08.md) · [Reset](../src/engine/Synth.cpp) · [Triangle előzmény](../src/dsp/SevenSaw.cpp)

### G3 — macOS CPU kapu futási szórása

A #154 negatívjában az egyhangos teljes FX jelenet 2,865071%-ot mért
2,791633%-os határ mellett. A korábbi zöld main megfelelő mérése 1,726977%.
Mind a baseline, mind a current executable hash egyezett a két futás között.
A teljes folyamat összesített CPU idejének mediánja nem nőtt, de ez az érték
több jelenetet fed le. A kapu a Classic Synth render faliórával mért idejét
értékeli. A Premium K1-teszt és a sűrű MIDI-s ARP-próba saját mérési útvonalon fut.

Következő indokolt vizsgálat: konkrétan a jelenetre eső szál CPU-időt és
falióraidőt együtt rögzítő, előre kijelölt kontroll. Azonos renderelt minták,
buildazonosítók és változatlan CPU-küszöb mellett vizsgáljuk az arányukat.
Az ismételten nagyobb szál CPU-idő számítási többletre utalna, az eltérő
falióraidő hasonló szál CPU-idő mellett várakozás vagy ütemezés felé mutatna.
A korábbi negatív megmarad, új méréshez konkrét hipotézis szükséges.

[#154 PR és megőrzött adatok](https://github.com/RobCZart82/SAWSTAR/pull/154) · [Benchmark](../tests/audio_benchmark.cpp)

## Audit közben futtatott ellenőrzések

- ARM64 Premium Production offline Release: 181/181 CTest sikeres.
- ARM64 ASan/UBSan: Premium állapot, Premium identitás/késleltetés, host MIDI
  pontok. Mindhárom sikeres.
- Rosetta, aktuális forrásból épített x86_64 Premium Reference/Candidate:
  egy Triangle passzív FP megfigyeléssel és egy normál Sine futás. Két
  teljes rács, egyenként 120 különböző eset, minden output és pre-FX összevetés
  bitazonos. Executable SHA-256:
  `5093aa1a9b748d5d5d7213fc6cda1b11e81145d46fa5468b7088e8bc4451079f`.
- Külön Premium Sustain render, ARP esemény és stresszpróba, illetve
  szimulált VST3 streampróba. A helyi források és nyers eredmények a
  munkaterület `work/outputs/audit-2026-11-21-fbf967a` könyvtárában vannak.
  Ez a napló őrzi a minimális eseteket akkor is, ha a helyi fájlok eltűnnek.

Fejlesztési sorrend: A1 állapotkezelés → A2 Sustain → A3 bypass, majd külön
normálparaméterek → A4 ARP működés, majd külön teljesítmény. A5 önálló
csomagolási lépés. G1–G3 lezárása az adott célzott bizonyítéktól függ.
