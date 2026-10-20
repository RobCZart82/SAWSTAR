# SAWSTAR — fejlesztési terv a következő nyilvános kiadásig

Terv dátuma: **2026-10-17**, állapotfrissítés: **2026-10-20**. Tulajdonos és zenei elfogadó: Robert.
A kiinduló nyilvános verzió 1.0.4; az új verziószám és kiadási dátum még nincs kijelölve.
A végrehajtási terv nem jelent release-, tag- vagy publikálási engedélyt.

[Dokumentációs index és karbantartási rend](DEVELOPMENT_DOCUMENTATION_INDEX.md).
A kiinduló PR/main-azonosítók dátumozott ellenőrzési alapok; a mindenkori
CI-állapotot a pontos aktuális headen külön kell ellenőrizni.

## K1 — eredeti executable kontrollja a #123 után

#123 beolvadt (`9383d46d`), mind a 19 PR-workflow / 59 job sikeres.
Az [eredeti bináris CI-visszajátszása](K1_ORIGINAL_BINARY_REPLAY.md) külön
kampányként implementált: két eredeti archívum, hashazonos executable,
14–14 rögzített folyamat, az eredeti `--diagnostic-waveform 2` Triangle-úttal.
A kampány eredménye és az új main teljes CI-je külön kapu; K1 továbbra nyitott.

## 1. Kiinduló állapot

**Friss audit és javítási munkacsomag:**
[BUGFIX_DEVELOPMENT_PLAN_2026_10_20.md](BUGFIX_DEVELOPMENT_PLAN_2026_10_20.md).
A 2026-10-20-án ellenőrzött main `547f84d4` (#115) négy fő workflow-ja sikeres.
#116 beolvasztás nélkül lezárt, megőrzött head `fb3114eb`; az eredeti Triangle x86_64/Rosetta biteltérés
oka nem ismert. Ez külön M1 numerikus blokkoló; a későbbi zöld diagnosztika
nem okbizonyítás. A részletes bizonyíték a #116 rögzített headjén van, nem mainben.

A B1 szubnormális mixer/MIDI-simítóállapot P2 [javítása/regressziója](B1_CONTROL_SMOOTHING_FIX.md)
#118-ként beolvadt: 17 PR-workflow és az új `feda3ea9` main négy fő workflow-ja
sikeres, natív elfogadás külön. A B2 SUB-frekvenciaplafon P3
[policyja/javítása és regressziója](B2_SUB_FREQUENCY_FIX.md) implementált
M3-feladatként #119-ben beolvadt, 12 PR-workflow sikeres; új main/natív kapu külön.
Az R1 többpéldányos preset-lock [process-local részjavítása](R1_PRESET_MUTATION_CONTENTION.md)
#120-ként beolvadt, hét PR-workflow sikeres; új main és teljes natív/I/O-elfogadás M5-ben külön kapu.
D1/D2/D3 a dokumentáció/metadata munkája, Q1 a pontos RC natív jegyzőkönyve;
T1 opcionális CMake-minimum kontroll. A javító PR-k az érintett M1/M2/M5
ellenőrzéseket új forrásazonossággal végzik el; a lezárt régi kampányok megmaradnak.
Az alábbi korábbi implementációs eredmények ettől nem válnak natív elfogadássá.

- A #102–#111 fejlesztési sorozat beolvadt, a javító #106-tal együtt.
  A #111 merge-je `f1b559fa6a74f0931bfe748e432c93644cfcce06`.
  Előtte mind a 15 releváns workflow és 50 job sikeres volt. Robert kizárólag
  ehhez a PR-hez engedélyezte a korábbi piros main feltételének egyszeri feloldását.
  A merge utáni main CI-t külön kell ellenőrizni; ez nem általános kivétel.
- A #112 a lezárt célgépes bizonyíték ráta-/pufferösszesítőjével és e tervvel
  beolvadt (`d434bf03`). A beolvasztás nem CPU-/natív elfogadás.
- A #113 kalibrált motorpár numerikus kapuja beolvadt (`dd5853b1`), 7 workflow /
  18 job sikerével. A [külön natív Candidate identitása és macOS csomagolási útja](NATIVE_PREVIEW_PAIR_QUALIFICATION.md)
  a #114-ben beolvadt, 16 workflow / 55 job sikeres. A [Windows x64 csomagolás](PREMIUM_PREVIEW_WINDOWS.md)
  implementált; friss CI/validator és mindkét célgép tényleges telepítési próbája szükséges.
- Production: **Classic filter**. Premium: külön, alapból kikapcsolt Preview.
  A Saw-dispatch csak az explicit, külön Candidate identitásban választható;
  a meglévő Premium Preview a natív pár változatlan referenciája.
- A hangerőkorrekció Robert által elfogadva: a Premium átlagos RMS-szintje
  0,44 dB-lel alacsonyabb maradhat. Ez lezárt döntés; nem indítunk újabb globális
  hangerőemelést vagy automatikus hangerő-kiegyenlítést.
- M1 és Windows: azonos forrású, platformonként 32 grides offline gyűjtés,
  minden nyers ellenőrzés és az összevetés kész. Nem indítjuk újra kedvezőbb számokért.
  A p50-javulás mellett p99-romlások és túllépések maradtak; CPU-elfogadás nyitott.
- A GUI-méret host-state javítása implementált (#109), natív projekt-visszatöltési
  elfogadása még nyitott. Nem csupán dokumentált, de még nem igazolt natív siker.
- A korábbi negatív main/CPU bizonyítékok megmaradnak. Egy későbbi zöld CI nem
  magyarázza meg automatikusan a korábbi mérés okát.

## K1 aktuális diagnosztikai lépése

A #121/#122 után a [passzív FP-pillanatképek és renderelési sorrendpróba](K1_FP_OBSERVATION.md)
implementált (`848e4fcc` alap). A `8f617709` FP-workflow 720 esetében és
ugyanazon CI-bináris helyi, 120 esetes visszajátszásában nincs audioeltérés.
Az IE már setup alatt megjelenik; a `0x1fb3` önmagában nem K1-hibajelző.
Következő az eredeti bináris kontrollja és külön a setup-művelet lokalizálása.
K1 továbbra nyitott; a friss teljes PR/main CI külön kapu.
Az alábbi belső trace ettől függetlenül megmarad.

A #120 beolvadt, hét PR-workflow sikeres; a `c6507db0` main friss CI-je külön
kapu. A [belső jelút-trace](K1_INTERNAL_TRACE.md) előkészíti a következő
Rosetta-eltérés lokalizálását. Ez nem igazolt ok vagy javítás, M1 numerikus
elfogadását nem zárja le; a megőrzött eredeti hiba megmarad.

## 2. Fejlesztési sorrend és mérföldkövek

| Sorrend | Mérföldkő | Elvégzendő munka | Lezárás bizonyítéka |
| --- | --- | --- | --- |
| M0 | Stabil fejlesztési alap | #114 utáni main és minden új PR pontos headjének ellenőrzése; szükséges CI-hibajavítás. | Minden releváns PR-workflow és job sikeres, a main négy fő workflow-ja is sikeres; szabályos soros merge. |
| M0 audit-munkacsomag | B1 és súlyos reprodukált hibák | B1 eredeti-forrást elutasító regresszió és célzott javítás; R1 vizsgálatából származó súlyos hiba előresorolása. | Bizonyíték, pontos forrás/CI és az érintett numerikus/natív kontrollok; részletes feltételek a hibajavítási tervben. |
| M1 | Natív tesztelhető jelölt | Pontosan azonosított, külön Reference/Candidate tesztbuild a Premium lookup és Saw-dispatch úthoz; azonos filter, gain, paraméterek és buildpolicy. Numerikus regressziók, validator, telepítési próba. | Két egyértelmű buildazonosság és binárishash, azonos tesztprojekt; a jelölt nincs ettől productionné minősítve. |
| M2 | Natív CPU-/dropout-elfogadás | Az alábbi REAPER-terv végrehajtása M1 és Windows célgépen, minden eredménnyel és külön tartalékvizsgálattal. | Konfigurációnként kitöltött jegyzőkönyv; hibák rendezve; Robert név szerint elfogadja a megjelölt buildet és tartományt. |
| M3 | Filterminőség és végleges ráta-/pufferpolicy | LP12/LP24/HP12/BP12, rezonancia, Drive, cutoff, módváltás, magas hangok, nagy ráta; zenei és műszaki ellenőrzés. | Dokumentált minőség/költség döntés és támogatási tartomány; hibás és nem vizsgált tartományok is feltüntetve. |
| M4 | Premium production-integráció | Elfogadott motor beépítése a Classic helyére, a meglévő GUI/paraméter-ID-k megőrzésével; bypass, latency, state, automatizálás és régi projektek kezelése. | Tesztek/validator és natív integrációs próba sikeres; a hallható filtercsere és kompatibilitási viselkedés leírva. |
| M5 | GUI, preset és életciklus lezárása | 75/100/125% mentés/újranyitás; több példány; editor reopen; preset/import/mentés lejátszás közben; unload, eszköz- és rátaváltás, tail/idle. | M1/Windows natív jegyzőkönyv, ismételhető megmagyarázatlan hiba nélkül. |
| M6 | Factory hangszínek véglegesítése | Minden factory preset ellenőrzése a végleges motoron; megfelelő headroom, hangszínkoherencia, mentés/visszatöltés. | Presetenkénti lista, rögzített tesztterhelés és peak/RMS; clipping nélkül, a 0,44 dB-es elfogadott korrekció megtartásával. |
| M7 | Végleges RC | Verzió/RC kijelölése, tiszta forrásból Windows/macOS csomag, hashok, validator; telepítés és natív végellenőrzés az RC pontos binárisaival. | RC-forrás és csomagazonosság, végső host/CPU/state teszt és összes kiadási CI sikeres. |
| M8 | Kiadás előkészítése | Kézikönyv, README, install, release notes, ismert korlátok, támogatott rendszerek/architektúrák és mérési állítások frissítése. | Nincs elavult készültségi állítás; letöltési/telepítési útvonal és végső csomagok ellenőrzöttek. |
| M9 | Publikálás | Robert a konkrét RC-t és kiadási tartalmat jóváhagyja; ezután tag/release és a jóváhagyott csomagok publikálása. | Publikus letöltés, csomaghashok és telepítési smoke-teszt visszaellenőrizve. |

A kritikus bug és a piros funkcionális CI megelőzi a következő lépést.
A natív próbára adott engedély nem jelenti automatikusan a motor production-promócióját.
Linux plugin, 32 voice és függőségfrissítés továbbra halasztott.

## 3. Mit jelent a CPU-/natív elfogadás?

**Egy konkrét buildet, konkrét gépen, audioeszközzel, rátával/pufferrel és
terheléssel fogadunk el.** Nem kell minden cellában gyorsabbnak lennie a
Classicnál vagy a Serumnál. A zenei haszon mellett működnie kell stabilan,
megfelelő tartalékkal. A piaci pluginok összevetése tájékoztató, nem kiadási kapu.

Három eltérő összehasonlítás marad külön:

| Pár | Mire ad választ? | Mire nem ad elfogadást? |
| --- | --- | --- |
| Classic ↔ jelenlegi Premium Preview | Zenei használhatóság, hangerő, jelenlegi Preview natív költsége. A már elfogadott hang/hangerő megőrzendő. | A még nem integrált Saw-dispatch optimalizációra. |
| Premium lookup Reference ↔ azonos Premium filterű Saw-dispatch Candidate | A kutatási optimalizáció natív CPU-/dropout-viselkedése, azonos zenei munkával. Ez az M2 fő összehasonlítása. | A későbbi production wrapper és végleges RC teljes elfogadására. |
| Elfogadott Premium tesztbuild ↔ production RC | Az integráció és csomagolás utáni működés/CPU/state ellenőrzése. | Másik, később módosult binárisra. |

A Reference/Candidate legyen külön azonosítható és szükség esetén egymás
mellé telepíthető. A pontos VST3-azonosság és projektállapot ne keveredjen.
DSP-/host-adapter-/buildpolicy-változás után az érintett natív eseteket újra
kell értékelni; egy dokumentációs módosítás önmagában nem indok új célgépes kampányra.

## 4. Valós REAPER CPU-tesztterv

### 4.1 Környezet és bizonyíték

A [részletes natív jegyzőkönyv](NATIVE_REAPER_CPU_ACCEPTANCE.md) a végrehajtási
adatlap. Minden platform külön eredményt kap. Kezdés előtt rögzítjük:

- dátum, gép/CPU/RAM/OS, hálózati táp és energiaprofil, ismert háttérterhelés;
- REAPER verzió/architektúra, plugin név/szerep/architektúra, forráscommit,
  build-eredet, VST3-csomag és tényleges executable SHA-256;
- audioeszköz, CoreAudio/ASIO driver, sample rate, tényleges puffer;
- anticipative FX, multiprocessing, live monitoring, többi sáv/FX állapota;
- tesztprojekt `.rpp`, MIDI és cutoff-automatizálás, preset és minden paraméter;
- a használt REAPER-mérő pontos neve és a leolvasás módja. FX CPU%, teljes CPU,
  RT CPU és a plugin saját DSP%-a külön adat, nem helyettesítik egymást.

A fő CPU-összevetésben **egyszerre egy instrument online**, a másik offline.
Némítás önmagában nem elég. Párhuzamos példányok csak külön tartalék-/stresszpróbában.

### 4.2 Két rögzített teszthangszín

- **P0 — filter/motor kontroll:** azonos saw-alap, azonos két oszcillátor,
  unison, detune, sub/noise, borítékok és kimenet; chorus/delay/reverb kikapcsolva.
- **P1 — zenei/FX terhelés:** Robert korábbi összevetésének mentett hangszíne
  vagy egy előre kijelölt factory pad/saw preset, rögzített FX-beállításokkal.

A MIDI 1/8/16 ténylegesen megszólaló hangot adjon. A 16 hangos műszaki tesztnek
nem kell zeneinek lennie; velocity, hanghossz, release-átfedés és akkordmenet
azonos legyen a két buildnél. Az automatizált cutoff-görbe is ugyanaz a mentett
adat legyen; ne kézzel próbáljuk hasonlóan mozgatni a potmétert.

A headroomot **a futás előtt** állítjuk be és elmentjük, majd nem módosítjuk.
Nem használunk új limitert/AGC-t a hiba elfedésére. A factory presetek saját,
változatlan alapértékű clippingtesztje az M6-ban külön feladat marad.

### 4.3 Futtatási szabály és időigény

Egy mérési ablak: **10 s bemelegítés + 60 s megfigyelés**. Egy A/B esetre
A→B, majd B→A sorrend készül: összesen négy ablak. Minden eredmény megmarad.
A hibás eredményt nem írjuk felül kedvezőbb újrapróbálással; javítás utáni új
futás külön azonosítót és okot kap. Külső zavar esetén a futás „nem értékelhető”,
az eredeti bizonyíték akkor is megmarad.

| Tesztcsalád | Ráta / puffer | Hang | Mód / cutoff / GUI | Esetszám gépenként |
| --- | --- | --- | --- | ---: |
| C — fő CPU-mátrix, P0 | 48 kHz / 128 és /64 | 16 | Mind a 4 filtermód × statikus és automatizált cutoff; GUI zárva. | 16 |
| V — hangszámkontroll, P0 | 48 kHz / 128 | 1 és 8 | Mind a 4 mód, automatizált cutoff, GUI zárva. | 8 |
| G — GUI-terhelés, P0 | 48 kHz / 128 és /64 | 16 | LP24, automatizált cutoff, GUI nyitva; a zárt kontroll a C megfelelő esete. | 2 |
| F — zenei FX-terhelés, P1 | 48 kHz / 128 és /64 | 16 | Előre kijelölt mód, automatizált cutoff, GUI nyitva. | 2 |

Összesen **28 összehasonlítás × 4 ablak = 112 ablak / gép**: nettó 130 perc
40 másodperc, plusz setup, állapotváltás, tail/idle és jegyzetelés. Részekre
bontható, a kihagyott eset „nincs mérve”; nem teljes körű paraméter-keresztszorzat.
Ez egy Reference/Candidate pár becslése, nem minden későbbi buildre ismétlendő vak kampány.

Gyors előszűrésként előbb a C 48 kHz /128 automatizált, 16 hangos négy módját
futtatjuk le: nettó 18 perc 40 másodperc/gép. Ez korai hibát találhat, de nem
helyettesíti a teljes kijelölt elfogadási mátrixot. A sorrend: /128, majd /64.

### 4.4 Külön stressz-, idle- és hostpróbák

- **Tartalékpróba:** a fő összevetés után két előre kijelölt, azonos terhelésű
  példány együtt, 60 s megfigyeléssel, az alapbeállítások változtatása nélkül.
  Ez rendszerterhelési próba; nem keverjük a fő A/B CPU-számaival.
- **Tail/idle:** MIDI-stop után a teljes release/FX-tail kifutása; utána 60 s
  aktív hang nélküli megfigyelés. GUI nyitva és zárva külön, mindkét pufferen.
  VU-csendből nem következtetünk a motor leállására.
- **Sűrű MIDI/voice steal:** release-átfedések, sustain, all-notes-off/panic;
  nincs beragadt hang, váratlan ugrás vagy hosthiba.
- **Minőség:** rezonancia/Drive szélei, cutoff végpontok, filtermódváltás;
  a szándékos torzítás megkülönböztetendő a nem kívánt hibától.
- **State/GUI:** 75/100/125%, editor bezárás/újranyitás, projekt mentés/újranyitás,
  két példány eltérő mérettel, presetváltás. A tesztelt GUI-fix natív lezárása külön rögzítendő.

### 4.5 Magas ráta, kis puffer és támogatási döntés

A 32 minta és 96/192 kHz **külön kiterjesztési kapu**. A meglévő offline
negatív eredményeket a [ráta-/pufferösszesítő](measurements/target-rate-readiness-2026-10-17.md)
őrzi. Ha ezekre is teljesítményígéretet teszünk, előre kijelölt natív mátrixot
kell hozzájuk végrehajtani. Ha nincs bizonyíték, „nem elfogadott/nem vizsgált”
állapot szerepel a dokumentációban; a plugin nem kap ettől automatikus tiltást
vagy alacsonyabb minőséget. A 44,1 kHz-es audio/state kompatibilitás is ellenőrzendő;
a 48 kHz-es CPU-elfogadás nem címkézhető át 44,1 kHz-es mérési eredménnyé.

## 5. Mit rögzítünk minden ablakban?

| Adat | Követelmény |
| --- | --- |
| Teszteset, build és sorrend | C/V/G/F azonosító, paraméterek, A vagy B, AB/BA, pontos dátum/idő. |
| CPU | Ugyanazon REAPER-mérő átlag/maximum, ha elérhető; egyébként megfigyelt tartomány, egyértelműen így címkézve. |
| RT-tartalék | A projekt RT-mérője külön, ha értelmezése/skálázása igazolt; nem számoljuk át az FX CPU%-ból. |
| Xrun/dropout | Host/driver számláló induló és végértéke, ha van; elérhetetlenség „N/A”, nem nulla. |
| Hang | Hallható kimaradás/recsegés, nem kívánt kattogás, torzulás, clipping; időpont és reprodukció. |
| Kimenet | Peak és szükség esetén RMS, rögzített headroom mellett. A már elfogadott 0,44 dB eltérés megengedett. |
| Működés | Crash, fagyás, elvesző state, beragadt MIDI, GUI-hiba külön. |
| Bizonyíték | RPP/preset/MIDI, képernyőkép vagy képernyőfelvétel, render és napló elérési helye/hash. |

Szemmel leolvasott CPU-ból nem készül p99. Az offline motoridő, a REAPER
CPU-kijelző és a hallható dropout három eltérő bizonyíték.

## 6. Elfogadás vagy további fejlesztés?

| Eredmény | Döntés |
| --- | --- |
| A kijelölt mátrix teljes, azonos build/környezet; nincs megmagyarázatlan pluginhoz köthető xrun, hallható hiba, instabilitás vagy clipping; dokumentált tartalék és Robert zenei elfogadása megvan. | Az adott gép/build/tartomány elfogadható. A másik platform külön döntés. |
| Minden hangpróba jó, de hiányzik eset, számláló/tartalék-bizonyíték vagy pontos buildazonosság. | Nyitott / részleges; nem „PASS”. Az N/A nem bizonyít nulla hibát. |
| Ismételhető dropout, hanghiba, crash vagy működést megzavaró CPU-csúcs. | Az érintett tartomány blokkolt. Hibajegy, reprodukció, célzott javítás és annak újraellenőrzése szükséges. |
| Csak a kisebb puffer vagy magas ráta hibás. | A sikeres tartomány külön elfogadható; a hibás tartományhoz javítás vagy őszinte támogatási döntés kell. |
| P99 relatíve kissé rosszabb, de a natív működés jó és van tartalék. | A relatív szám önmagában nem kizáró ok; az abszolút határidő/túllépés és natív bizonyíték együtt értékelendő. A régi offline hiba megmarad. |
| Ugyanazon kijelzőn a Candidate kicsit drágább, de stabil és zeneileg indokolt. | Nem kötelező optimalizálni pusztán egy alacsonyabb százalékért; a rögzített CPU-tartalék és scope döntsön. |

**Javasolt, futás előtt rögzítendő natív tartalékcél:** legalább 20% megfigyelt
RT-tartalék a kijelölt tesztprojektben, azaz legfeljebb 80% megfigyelt RT-terhelés,
ha a REAPER-mérő értelmezése ezt lehetővé teszi. Ez e terv javasolt működési célja,
nem iparági kötelező határ és nem a meglévő offline küszöb módosítása.
Ha csak FX CPU% áll rendelkezésre, ebből nem állítunk RT-tartalékot: a tartalék
vizsgálatának módját a futás előtt kell rögzíteni, vagy ez a kapu nyitott marad.
A kétpéldányos stresszpróba kiegészítő, nem a hiányzó RT-adat számszerű pótlása.

Az elfogadási sorban szerepeljen: **Robert; dátum; build és binárishash;
gép/audioeszköz/driver; ráta/puffer; terhelés/preset; PASS / nyitott / blokkolt;
indok; ismert korlátok; bizonyíték**. A határt nem igazítjuk utólag a kedvező
méréshez. Hiba esetén a hiba okát vagy az elfogadott scope-ot tisztázzuk.

## 7. Végső release kapu

A production-integráció utáni RC pontos binárisával a kijelölt natív CPU-eseteket
és a host/state/regressziós próbákat ismételten igazolni kell; korábbi Preview
vagy más executable eredménye nem másolható át. A factory szintek végső ellenőrzése
változatlan presetekkel történjen. Minden kiadni kívánt architektúrához legyen
validator/build-bizonyíték és a ténylegesen elvégzett natív próbák pontos leírása;
a nem vizsgált architektúrát ne nevezzük natívan elfogadottnak.

A végső forrás/head összes releváns workflow-ja és jobja sikeres legyen,
a csomagok tiszta forrást, rögzített függőségeket és ellenőrzött hashokat használjanak.
A kézikönyv/release notes nevezze meg a Classic→Premium hangzásváltozást,
a kompatibilitást és a támogatási korlátokat. Robert a konkrét RC és release
áttekintése után ad külön publikálási engedélyt.

## 8. Felelősség és munkaritmus

- Fejlesztői munka: implementáció, tesztbuild, projekt/tesztadat előkészítése,
  automatizált numerikus/CI/validator ellenőrzés, nyers bizonyíték kiértékelése.
- Robert: a valódi célgépes lejátszás/hallgatás, környezetadatok, mérők és hibák
  rögzítése, zenei és megjelölt natív tartomány elfogadása.
- Natív adatot kizárólag tényleges géphozzáféréssel és megfigyeléssel rögzítünk.
  Felhős/Linux futásból nem állítunk M1- vagy Windows REAPER-sikert.
- Minden fejlesztési fázis külön ellenőrzött PR. Squash merge csak a pontos
  head összes releváns sikeres workflow-ja/jobja és az előző main kapui után,
  expected head SHA-val. Nincs force, védelmi bypass vagy vak auto-merge.

Kapcsolódó dokumentumok: [roadmap](DEVELOPMENT_ROADMAP.md),
[natív végrehajtási jegyzőkönyv](NATIVE_REAPER_CPU_ACCEPTANCE.md),
[ráta-/pufferpolicy](RATE_BUFFER_ACCEPTANCE_POLICY.md),
[GUI-state elfogadás](GUI_SCALE_PROJECT_STATE.md),
[elfogadott hangerő](PREMIUM_PREVIEW_LEVELS.md).
