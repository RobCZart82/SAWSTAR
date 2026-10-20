# Átfogó hibajavítási és fejlesztési terv — 2026-10-20

**Állapot: tervezett munka; nincs ettől implementáció, natív elfogadás vagy publikálás.**
Forrásalap: main `547f84d4f72a792aaf0b229c9d73b7a8a00fae40`.
A [következő release M0–M9 terve](NEXT_RELEASE_DEVELOPMENT_PLAN.md) marad a
kiadási végrehajtás elsődleges dokumentuma; ez annak hibajavítási munkacsomagja.
[Aktuális index](DEVELOPMENT_DOCUMENTATION_INDEX.md), [roadmap](DEVELOPMENT_ROADMAP.md).

## 1. Hatókör és bizonyítékszintek

A 2026-10-20-i kódvizsgálatot és Robert két megosztott statikus jelentésének
hasznos megállapításait egyesítjük. A két külső jelentés lényegében ugyanazokat
a dokumentációs/kiadási állításokat tartalmazta; nem két független reprodukció.
Az alábbi táblázat külön kezeli a reprodukált jelenséget, az ismert CI-hibát,
a kódból levezetett kockázatot és a karbantartási javaslatot.

- **Igazolt jelenség:** konkrét forrás és célzott próba/mért eredmény.
- **Alapos gyanú:** végigkövethető kiváltó út, de nincs még natív hatásmérés.
- **Ismert nyitott ügy:** meglévő bizonyíték és nyilvántartás; nem új hibaként számoljuk.
- **Karbantartás:** javítható egyértelműség vagy tesztlefedettség, nem bizonyított runtime bug.
- **Elvetett állítás:** az idézett forrásból nem következik; indoka megmarad.

A nyilvános Classic **1.0.4** és a következő **Premium Reference/Candidate/RC**
külön forrás/binary/elfogadási hatókör. Egy nyitott Premium-kapu nem bizonyítja
önmagában az 1.0.4 hibás publikálását. Az elfogadott 0,44 dB RMS-különbséget,
a lezárt célgépes kampányokat és a negatív eredményeket nem írjuk át.

## 2. Egységes feladatjegyzék

| ID | Prioritás és állapot | Feladat | Mérföldkő |
| --- | --- | --- | --- |
| B1 | P2, javítás és negatív kontroll implementálva; CI/natív kapuk külön | A mixer/MIDI-simítók szubnormális lecsengésének lezárása, átmeneti regressziókkal | M0, majd érintett M1/M2/M5 |
| K1 | Candidate numerikus elfogadási blokkoló; ismert, nyitott | #116 eredeti Triangle x86_64/Rosetta biteltérésének okfeltárása | M1, production-promóció előtt |
| R1 | Vizsgálandó P2-kockázat; natív próba nyitott | Példányok közötti preset-mutex és GUI-várakozás mérése, szükség esetén javítása | M5; súlyos reprodukciónál M0 |
| B2 | P3, frekvenciaeltérés reprodukált; policy/javítás nyitott | SUB 20 kHz-es abszolút plafon és mintavételarányos korlát egységesítése | M3, M5/M6 regresszió |
| D1 | P3, a státuszmondat ebben a dokumentációs változásban pontosítva | ARP történeti pending és későbbi felhasználói próba megkülönböztetése | M8 |
| D2 | Karbantartási adósság, nyitott | A `release_date` jelentésének és a jövőbeli build/publikálási dátummezőknek rendezése | M7–M8 |
| Q1 | Dokumentált lefedettségi hiány; következő RC-re kötelező | Pontos végleges RC-hez esetenkénti natív jegyzőkönyv és architektúránkénti státusz | M5, M7–M9 |
| T1 | Opcionális buildellenőrzés, nyitott döntés | CMake minimumverzió külön CI-lefedettségének mérlegelése | M0/build karbantartás |
| D3 | Folyamatos dokumentációs feladat | Aktuális index/roadmap/release terv egyeztetése; konkrét elavult mondatok javítása | M0–M9 |

P2: javítandó mérnöki probléma vagy vizsgálandó használhatósági kockázat.
P3: kis hatású/szélső eset. A K1 blokkoló minősítése a Candidate numerikus
elfogadására vonatkozik, nem bizonyított felhasználói crashre.

## 3. B1 — Szubnormális DSP-simítóállapotok

**Friss implementáció:** [B1 javítás és regresszió](B1_CONTROL_SMOOTHING_FIX.md).
A nulla célértékhez tartó mixer/CC1/pressure állapot `1e-24` alatt nullára zár;
a nem nulla cél simítása változatlan. Az új 15 részpróbás regresszió az eredeti
forrással hibázott. A pontos-head CI és a natív/CPU-elfogadás külön státusz.
Az alábbi eredeti leírás és lezárási feltételek megmaradnak.

**Hely:** [Synth.cpp](../src/engine/Synth.cpp), `ProcessStereo()`, a
`smoothWheel_`/`smoothPressure_` és `levels_` exponenciális simítása
(a forrásalapon 244–246. sor). A nullához tartó állapot itt nem zárul le.
A `noiseWeights_` és az `ampSustain_` másutt már célértékre zár.

**Helyi reprodukció:** macOS arm64, AppleClang 21, Debug DSP, 48 kHz.
`Synth::Reset(48000)`, `SetParameters(-12,1,1,1,10)`, Note On A4/127,
4800 minta; mixer minden forrása nullára; CC1=127, 4800 minta, CC1=0;
további 96000 minta. Diagnosztikai mezőkiolvasás eredménye:

```text
levels_[0]       = 1.68156e-43, FP_SUBNORMAL
smoothWheel_[0]  = 1.68156e-43, FP_SUBNORMAL
FE_UNDERFLOW    = 1
```

A privát mezők kizárólag külön diagnosztikai hozzáféréssel voltak megfigyelve;
a DSP-rekurzió nem változott. Az aftertouch azonos képletet használ, külön
helyi mérés nem készült. A numerikus állapot bizonyított, a REAPER CPU%-romlás
vagy dropout **nem**. Host/processzor FP-policytól függő teljesítménykockázat;
nem állítunk hallható maradékjelet vagy automatikus kapcsolatot a K1 hibával.

A `Synth.cpp` Git-blobja az 1.0.4 tagben és a forrásalapon azonos, tehát a
jelenséget okozó forrás a kiadásban is megvan. Ez nem a kiadott bináris natív próbája.

**Megvalósítási terv:**

1. A javító PR őrizze meg a minimális reprodukciót; az eredeti forrással bukjon
   a szubnormális állapotot tiltó ellenőrzés. Terjedjen ki mixerre, wheelre és pressure-re.
2. Kis érték/célhiba alatt megfelelő lezárás; a normál fade és a hallható átmenet
   maradjon folytonos. Ne változzon általános gain, paraméter-ID vagy state-formátum.
3. 44,1/48/96/192/384 kHz, kitartott hang, nullára és nem nulla célra váltás,
   gyors retarget, note release/reset. Az FP-környezetet rögzíteni kell;
   az FTZ/DAZ által eltüntetett jelenséget ne nevezzük eredeti-forrás PASS-nak.
4. A közös Synth-ből generált Classic/Preview/Candidate utakhoz kapcsolódó
   átmeneti és numerikus regressziók, pontos-head CI; érintett natív próbák.

**Lezárás:** reprodukció + célzott teszt + megindokolt küszöb + javított pontos
forrás/CI. CPU-nyereséget csak külön mérés igazolhat. Nem nyitjuk újra a lezárt
kampányt kedvezőbb számokért; szükséges új DSP-kontroll új forrásazonosságot kap.

## 4. K1 — A #116 Rosetta-biteltérése

A 2026-10-20-i auditkor #116 nyitott volt, head
`fb3114eb829442223083e103929a15ad25d3efeb`, base a fenti main.
Az eredeti `05b730d` head macOS Universal Reference-jobja Triangle x86_64/Rosetta
bitazonossági hibával állt meg: [workflow/job](https://github.com/RobCZart82/SAWSTAR/actions/runs/39126567559/job/117672833349).
Az ok és a mintánkénti hibanagyság ismeretlen. A későbbi véges diagnosztika
nem reprodukálta a hibát; ez nem DSP-javítás vagy feloldott elfogadási kapu.

A részletes eredeti napló, hashok, 28 stream / 3360 esetmegfigyelés és diagnózis
**a #116 rögzített headjén** található:
[#116 hibafeljegyzés, rögzített head](https://github.com/RobCZart82/SAWSTAR/blob/fb3114eb829442223083e103929a15ad25d3efeb/docs/PREVIEW_BIT_IDENTITY_FAILURE_2026_10_18.md).
Ez még nem mainben levő dokumentum. A folytatás ellenőrzésekor #116 már beolvasztás
nélkül lezárt; a K1-hiba ettől nem oldódott meg. A négy hidegfolyamat/architektúra véges
ellenőrzése és a megőrzött bizonyíték-visszaellenőrző sem okbizonyítás.

**Terv és lezárás:** az eredeti negatív bizonyíték megőrzése; bizonyított ok,
kapcsolódó javítás és az érintett numerikus kontrollok. Az eredeti hibás
executable és első eltérő minta hiánya külön reprodukálhatósági korlát.
Ne engedjük el a bitpontos feltételt, ne nevezzük tesztzajnak bizonyíték nélkül,
és ne ismételjünk azonos próbát pusztán újabb zöld eredményért. A jelen
dokumentációs PR nem zárja le, nem merge-eli és nem kerüli meg #116 kapuját.

### Friss negatív bizonyíték — 2026-10-20

A `fb3114e` head Candidate-jobja ismét Triangle x86_64/Rosetta biteltéréssel
hibázott: 48 kHz / HP12 / Poly / FX ON, main frame 197, output bal csatorna,
delta `-1.9790604710578918e-09`. Ez nem az eredeti hiba méretének utólagos
kiszámítása, hanem új, pontos diagnosztikájú negatív eredmény. A megőrzött
hibás és sikeres job ugyanazt a tesztbinárist használja. A teljes bizonyíték,
helyi kontrollok és a B1/B2/R1 kapcsolatok értékelése:
[Rosetta-vizsgálat 2026-10-20](ROSETTA_NUMERICAL_INVESTIGATION_2026_10_20.md).
Igazolt ok/javítás még nincs; a következő lépés az első belső jelút-eltérés trace-e.

## 5. R1 — Több példány presetműveleteinek GUI-várakozása

**Hely:** [UserPresets.cpp](../src/presets/UserPresets.cpp),
`PresetMutationLock` statikus mutex/`unique_lock`, az egész import-batch-et
védő lock; [PresetBrowser.h](../src/gui/Controls/PresetBrowser.h) GUI-s
Save/Save As/Rename/Delete hívásai.

A helyi mutex megszerzésének nincs határideje. A fájlzár két másodperces
határideje csak ezután indul. Ugyanazt a pluginmodult használó példányoknál
A worker importja ezért B GUI-s mentését várakoztathatja, eltérő mappáknál is.
B saját `importJob_.Busy()` értéke nem tükrözi A munkáját.
Ez nem közvetlen audio-callback lock bizonyítéka; a valós REAPER-akadás nincs kimérve.

**Vizsgálat:** két azonos modulból származó példány egy hostfolyamatban,
100/400 preset, majd előre rögzített nagyobb batch; B mentés/átnevezés/törlés
A importja alatt. Azonos és eltérő mappa, lehetőség szerint lassú/külső meghajtó;
GUI-várakozás és audio/dropout külön mérendő. Rögzítsük a host bridging módját.

**Döntés/javítás:** mérés alapján dokumentált várakozási policy; ha szükséges,
GUI számára gyors busy-válasz vagy workerre helyezett fájlműveletek. Maradjon
meg a folyamatok közötti lock, exkluzív létrehozás, stale-write védelem,
backup/favorites konzisztencia és unload-biztonság. Súlyos, reprodukálható GUI-
fagyás M0-hibává emeli; enyhébb korlát dokumentált M5-döntést kaphat.

**Lezárás:** kétpéldányos mérési jegyzőkönyv; szükséges javítás esetén célzott
konkurenciateszt és natív ismétlés. Pusztán a worker megléte nem lezárás.

## 6. B2 — SUB felső frekvenciapolicy

**Hely:** [SubOscillator.h](../src/dsp/SubOscillator.h), `SetFreq()`
abszolút 20000 Hz-es clamp; a [Synth.cpp](../src/engine/Synth.cpp) már előtte
`sampleRate * .45` határt alkalmaz. A fő oszcillátorok végső, szorzókkal
módosított frekvenciáját a mintavételarányos plafon korlátozza.

**Reprodukció:** `SubOscillator::Init(96000)`, `SetFreq(25087.708f)`;
96000 kimeneti minta pozitív nullátmeneteinek száma **20000**, nem körülbelül
25088. A cél a 43200 Hz-es mintavételarányos határon belül van.
MIDI 127, SUB octave=0, +12 félhang bend elméletileg ilyen célt adhat;
ezt a teljes pluginbeállítást REAPER-ben még nem vizsgáltuk.
Az érintett header és Synth Git-blobja az 1.0.4 tagben is azonos.

**Hatás:** szélső, jórészt ultrahangos tartomány; normál basszusregisztert
nem érintő P3. Döntsük el, szándékos termékkorlát-e a 20 kHz. Ha nem,
a végső frekvencia plafonja legyen mintavételfüggő, érvénytelen/negatív
bemenet védelmével. Szándékos eltéréshez egyértelmű dokumentáció és teszt kell.

**Lezárás:** rögzített policy; 48/96/192 kHz-en 20 kHz alatti/feletti,
Nyquist-közeli, bend/modulációs és waveform-próbák. Javítás esetén célzott
regresszió és az érintett numerikus/factory ellenőrzések; nem általános hangszín-átírás.

## 7. Dokumentáció, metadata és QA

### D1 / D3 — Konkrét státuszok, nem teljes archívum-átköltöztetés

Az [ARP-leírás](ARPEGGIATOR.md) kézi ellenőrzésének eredetileg pending mondatát
történeti időponthoz kötjük; a későbbi felhasználói REAPER-visszajelzés külön
marad, build-/esetazonos teljes natív jegyzőkönyvet nem állítunk belőle.
Az aktuális index, roadmap és M0–M9 terv együtt mutat erre a munkacsomagra.
Régi mérések, publikált tag/csomagok és negatív eredmények változatlanok.
Nincs bizonyítékot megőrző célt szolgáló tömeges fájlmozgatásra szükség;
csak konkrét, jelöletlenül elavult mondatot javítsunk.

### D2 — Dátummezők szerződése

Az 1.0.4 csomagmetadata dátuma **2026-10-01**, publikálása **2026-10-02**.
A [release notes](RELEASE_NOTES_1.0.4.md) ezt már külön megmagyarázza.
A `release.json` `release_date` neve ugyanakkor builddátumként használva
félrevezető. Következő metadata-változás előtt fel kell térképezni a CMake,
metadata-sync, config/plist/About, csomagolók, publisher és tesztek fogyasztóit.
Döntés: a jövőbeli mezők pontos jelentése, a dátumformátum és a migráció/
visszafelé kompatibilitás. Ne adjunk önkényesen három részben duplikált dátumot.
Tényleges publikálási időt csak megtörtént release után rögzítsünk.

**Lezárás:** dokumentált séma és szükség esetén minden fogyasztót együtt
frissítő célzott PR, sync/release-guard ellenőrzéssel. A publikált 1.0.4 tag,
bináris, hash és történeti dátum emiatt nem írható át.

### Q1 — A következő RC natív bizonyítéka

Az [1.0.4 publikálási jegyzőkönyve](RELEASE_1.0.4_CHECKLIST.md) kifejezetten
rögzíti: a végső draft teljes, friss N01–N16 mátrixához nem érkezett új
esetenkénti jegyzőkönyv; korábbi RC-visszajelzés és tulajdonosi publikálási döntés
volt. Nem állítunk utólag teljes PASS-t vagy bizonyított runtime hibát.

A következő Premium RC-re M5/M7 alatt a pontos telepített binárishoz tartozó
MIDI/editor ownership, sustain/ARP, preset save/import, bypass/reset, unload,
GUI-state, render/tail és installer próbák szükségesek. Az M2
[natív CPU-adatlap](NATIVE_REAPER_CPU_ACCEPTANCE.md) 28 esetes / 112 ablakos
mátrixa külön marad; B1/R1/B2 kiegészítő próbái nem módosítják csendben ezt a számot.

**Lezárás:** forrás/executable/csomaghash, OS/architektúra, REAPER/audio/SR/buffer,
teszteset és eredmény. Windows x64/ARM64, macOS arm64/natív Intel/Rosetta
státusz külön; hiányzó adat N/A vagy nem tesztelt, nem PASS. A CI/validator,
korábbi zenei visszajelzés és a pontos RC natív elfogadása külön bizonyíték.

### T1 — CMake minimum CI-lefedettség

A [BUILDING.md](BUILDING.md) helyesen különbözteti meg a foundation **3.21**
minimumát és a **3.25+ ajánlott** toolchaint. Nincs igazolt verzióellentmondás.
Mérlegelendő egy külön foundation configure/build kontroll 3.21-gyel;
a pluginfüggőségek saját minimumát külön kell kezelni. Ez nem magas súlyosságú bug.
**Lezárás:** indokolt döntés; bevezetéskor a tényleges CMake-verzió és hatókör
naplózása, meglévő platformellenőrzések megtartása. Dokumentációs PR nem vezeti be.

## 8. A külső jelentések elbírálása

| Állítás | Döntés |
| --- | --- |
| 3.21 minimum és 3.25+ ajánlás magas súlyosságú ellentmondás | Elvetve: az idézett szöveg maga elkülöníti; T1 opcionális ellenőrzés marad. |
| Publikált 1.0.4 + nyitott Premium natív kapu = hibás release | Elvetve mint általános következtetés: eltérő kiadási scope; a tényleges 1.0.4 lefedettségi hiány Q1-ben megmarad. |
| Aktuális/történeti docs általában nincsenek elválasztva | Túlzó: van index és történeti jelölés. Konkrét D1/D3 javítások indokoltak. |
| Október 1/2 eltérés bizonyított verzióhiba | Elvetve: a különbség dokumentált. A mezőnév/séma D2-karbantartás. |
| CI nem helyettesít natív QA-t | Helyes alapelv, már dokumentált; Q1 lezárási feltétele, nem új runtime bug. |
| Nem találtak runtime bugot | A statikus vizsgálat korlátja; nem cáfolja B1/B2 célzott eredményét. |
| Jelentés mentése `sha`/névütközés miatt sikertelen | A jelentést készítő eszköz fájlfrissítési problémája; nem SAWSTAR-hiba. |

## 9. Végrehajtás, sorrend és készültség

1. **M0:** friss main és minden érintett PR pontos forrás/CI ellenőrzése.
   B1 célzott reprodukció/javító PR; a K1 okfeltárás külön munkaszál,
   bizonyíték nélkül ne kapcsoljuk össze őket. R1 súlyos eredménye előre kerül.
2. **M1:** K1 lezárása a saját bizonyítékával; bármilyen DSP-változás után az
   érintett kalibrált pár numerikus ellenőrzése friss forrásból szükséges.
3. **M2/M3:** az elfogadandó pontos motorpár natív CPU/dropout/minőség/policy
   döntése; B2 explicit policyja. Lezárt régi kampányt nem ismétlünk kedvező számokért.
4. **M4–M6:** a ténylegesen elfogadott motor integrációja; B1/B2 érintett
   integrációs regressziói; R1 és GUI/state/lifecycle; factory hangszínek.
5. **M7–M9:** Q1 pontos-RC jegyzőkönyv, D2/D3 végleges dokumentáció/metadata,
   csomag/hash/validator és minden kiadási kapu; külön publikálási döntés.

Minden hibajavítás külön, körülhatárolt PR legyen; az új tesztnek a régi
hibát kell elutasítania, nem a javítás implementációját másolnia. Státuszlépcsők:
**tervezett → reprodukált/vizsgált → implementált → helyi ellenőrzött →
pontos-head CI sikeres → beolvadt → érintett natív elfogadás lezárt**.
Egyik lépcső sem következik automatikusan az előzőből. Merge csak friss,
szükséges sikeres ellenőrzésekkel; force/bypass/vak auto-merge nélkül.

## 10. Az audit ellenőrzési alapja és korlátai

- A forrásalap négy fő workflow-ja 2026-10-20-án completed/success:
  [Code quality 39081228932](https://github.com/RobCZart82/SAWSTAR/actions/runs/39081228932),
  [lint 39081229021](https://github.com/RobCZart82/SAWSTAR/actions/runs/39081229021),
  [macOS 39081229022](https://github.com/RobCZart82/SAWSTAR/actions/runs/39081229022),
  [Windows 39081228948](https://github.com/RobCZart82/SAWSTAR/actions/runs/39081228948).
  Ez a kiinduló commit állapota, nem e dokumentációs PR vagy egy jövőbeli javítás CI-je.
- A helyi macOS arm64 audit során Debug/Release DSP-build elkészült;
  a Release-csomag 138 tesztjének mindegyikéhez sikeres eredmény tartozik:
  137 az összfutásban, az archívumteszt külön, az eredeti fixture-bájtok
  helyreállítása után. A helyi sorvég-/fixture-probléma nem repository-bug.
- A két új numerikus jelenség külön diagnosztikai próbát kapott. Nem történt
  saját natív REAPER-/Windows-/kiadott-bináris-próba vagy teljes helyi
  VST3/installer-validáció. A 138 teszt nem bizonyítja ezek megtörténtét.
- A javító PR-knek a reprodukciót tartós regresszióként kell hozzáadniuk;
  az eredeti dokumentációs audit nem jelentett új tesztet vagy javítást.
  A B1 későbbi implementációjának új ellenőrzéseit a hivatkozott javítási lap rögzíti.

**Következő konkrét munka:** az implementált B1 javítás pontos-head CI-je
és érintett elfogadása; külön K1 bizonyítékalapú okfeltárás. Az R1 natív próba tervezhető,
de eredménye és elfogadása jelenleg nyitott.
