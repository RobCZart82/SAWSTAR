# Kalibrált Preview Reference/Candidate — numerikus előfeltétel

Állapot: **az M1 mérföldkő első fejlesztési lépése; nem natív host- vagy CPU-elfogadás**.
A #113 numerikus előfeltétele beolvadt (7 workflow / 18 job sikeres).
A #114 külön Candidate-identitása és macOS Universal tesztcsomagjai beolvadtak
(16 workflow / 55 job sikeres, a két szerep mindkét szeletének validátora zöld).
A Windows x64 VST3-tesztcsomagolási út implementált; friss CI és tényleges
telepítési/natív elfogadás még szükséges.

## Miért kell külön kapu?

A lezárt célgépes kampány a Premium lookup kutatási referenciát és a Saw-dispatch
kutatási jelöltet vizsgálta. A valódi Preview azóta a jóváhagyott +3 dB wet /
dry unity kimeneti kalibrációval működik. A natív tesztpár előkészítéséhez a
jelöltet a **tényleges kalibrált Preview motorhoz** is külön kell hasonlítani.
A régi 480 eset nem címkézhető át automatikusan erre az új fordítási változatra.

## Implementáció és elhatárolás

- A referencia a meglévő `sawstar_premium_preview_engine` / `PremiumPreviewSynth`.
- A külön `sawstar_premium_native_candidate_engine` ugyanebből a generált,
  már kalibrált motorból készül. Csak az OSC1/OSC2 `SevenSaw` típusa cserélődik
  `SawDispatchSevenSaw`-ra. A filter, Drive, latency-adapter, kalibráció,
  envelope/MIDI/moduláció, SUB, FX és state ugyanaz marad.
- A két motor külön namespace-ben fordul. A jelölt könyvtárat a páros
  tesztexecutable és kizárólag az explicit Candidate VST3-target használja.
  A production Classic és a már létező Premium Preview identitása/hangútja megmarad.
- A közös `oscillator_dispatch_engine.cpp` fixture külön fordítási definícióval
  választja ezt a párt. Az eredeti lookup/dispatch kutatási fixture megmarad.
  Nem keverjük a régi célgépes forrás/hashjegyzékkel az új tesztforrást.

## Numerikus szerződés

Mind a 480 eset megmarad: négy hullámforma, öt ráta (48/96/176,4/192/384 kHz),
négy filtermód, Poly/Mono/Legato, FX ki/be. Mintánkénti bitazonosság a
kimeneten és a pre-FX ponton; véges, védett kimenet; moduláció, Drive-,
filtermód-/waveformváltás és dry/wet átmenet; MIDI ownership, voice steal,
sustain, release/tail, reset és negatív módosított-jel kontroll.
Hullámformánként 120 eset; hiányos mátrix nem sikeres.

Helyi ellenőrzés: GNU 13.3.0, Linux, Release; a kalibrált pár 480/480 esete
sikeres, a négy hullámforma naplója együtt 480 egyedi kombinációt őriz,
minden kimeneti/pre-FX hibamérték nulla. A meglévő Preview identity/32 mintás
dry latency/négymódú smoke is sikeres. Ez nem macOS/Windows natív futás,
nem CPU-időmérés, nem validator- vagy REAPER-eredmény.

## CI és futtatás

A külön `Calibrated Preview engine pair qualification` workflow Windows x64
és macOS runneren fordítja/futtatja a numerikus párt, plugintelepítés nélkül.
A Reference és Candidate buildkapcsoló külön mátrixdimenzió, saját smoke-kal
és role/executable/log hashokat tartalmazó manifesttel. Ez Windows alatt sem VST3-/host-próba.
A normál Universal Preview workflow a valódi Preview VST3 építése mellett
mindkét arm64/x86_64 szeleten futtatja a páros ellenőrzést is.
A külön workflow manifestje megőrzi a pontos checkout SHA-t, executable-
és naplóhashokat, siker/hiba állapotot; minden eredmény kísérleti, a
`native_host_acceptance` és `production_promotion_allowed` flag false marad.
Új head minden releváns CI-jének tényleges sikerére várni kell.

```sh
cmake -S . -B build-pair -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DSAWSTAR_PREMIUM_PREVIEW=ON -DSAWSTAR_BUILD_PLUGIN=OFF
cmake --build build-pair --config Release --target sawstar_premium_native_pair sawstar_premium_preview_smoke --parallel 2
```

A létrejött pair executable `--waveform 0`, `1`, `2`, `3` argumentummal fut,
argumentum nélkül mind a 480 esetet végrehajtja. A futtatási útvonal a
CMake-generátortól függ (Windows esetén jellemzően `Release/` almappa és `.exe`).
BUILD_TESTING=ON esetén négy külön `premium_native_pair_waveform_*` CTest is
regisztrált. Ez kvalifikáció, nem új lezárt M1/Windows mérési kampány.

## Külön natív identitás és macOS tesztcsomag

| Szerep | REAPER név | Bundle / wrapper osztály | Plugin-ID | Presetmappa |
| --- | --- | --- | --- | --- |
| Production | SAWSTAR | `SAWSTAR` | `SwSt` | `SAWSTAR` |
| Reference | SAWSTAR Premium Preview | `SAWSTARPremiumPreview` | `SwPP` | `SAWSTAR Premium Preview` |
| Candidate | SAWSTAR Premium Candidate | `SAWSTARPremiumCandidate` | `SwPC` | `SAWSTAR Premium Candidate` |

A Candidate két kapcsolót igényel: `SAWSTAR_PREMIUM_PREVIEW=ON` és
`SAWSTAR_PREMIUM_NATIVE_CANDIDATE=ON`; mindkettő alapból OFF. Preview nélkül
Candidate-et konfigurálni hiba. A VST3-adapter kizárólag ebben a külön
identitásban használ `PremiumNativeCandidateSynth`-et. A 32 mintás latency,
paraméterek/state, GUI-scale rekord, wet +3 dB / dry unity kalibráció és
factory trim közös. Az About verzió `-premium-candidate` utótagot kap.
Nincs új felhasználói Classic/Premium modellválasztó.

A `macOS Premium Preview` workflow két külön mátrixjobban, ugyanarról a
pontos checkout SHA-ról, ugyanazon Release/Universal beállításokkal készít
Reference és Candidate buildet. Mindkét job mindkét szeletén lefut a 480
esetes motorpár és a **kiválasztott saját motor** identity/audio/latency smoke.
A factory headroom-executable továbbra a kalibrált Reference/meterezett
Reference 144 esetét méri; nem címkézzük át Candidate mérésnek.
Mindkét tényleges VST3-bundle saját validator-próbát kap arm64 és Rosetta
x86_64 alatt. Ez utóbbi nem natív Intel REAPER-bizonyíték.

Csak sikeres, azonos SHA-jú jobok artifactjai használhatók párként:

- Reference: `SAWSTAR-Premium-Preview-macOS-Universal`.
- Candidate: `SAWSTAR-Premium-Candidate-macOS-Universal`.

Az artifacton belüli ZIP és `.sha256` ellenőrzendő. A `PACKAGE-MANIFEST.json`
megadja a forráscommitot, a `pair_role`-t, külön oscillator/ID-t, a tényleges
Universal executable SHA-256 hashát és minden csomagfájl hashát. A csomagoló
explicit `--role candidate` nélkül visszautasítja a Candidate-et; a bundle
átnevezése önmagában nem írhatja felül a plist-identitás kapuját.
`native_host_acceptance=false`, `production_promotion_allowed=false` marad.
A CI ad hoc aláírást használ, nincs Developer ID/notarizálás vagy publikálás.

Telepítési smoke a Mac célgépen, REAPER bezárása után:

1. A két azonos forrású ZIP teljes `.vst3` bundle-jét másold
   `~/Library/Audio/Plug-Ins/VST3/` alá. A Classic maradhat mellettük.
2. REAPER rescan után a fenti **két külön FX-név** legyen elérhető;
   About-ban ellenőrizd a forrásazonosítót és a két eltérő verzióutótagot.
3. Külön példányban ugyanazt a `.sawstar` hangszínt/MIDI-t/automatizálást töltsd
   be; ne a régebbi Classic vagy korábbi Preview-letöltést kezeld az új párként.
4. Ellenőrizd a hangot, GUI-t, 75/100/125% projektmentést/újranyitást és a
   két külön presetmappát. Jegyezd fel a pontos csomag-/binárishashokat.
5. A CPU A/B-próbában egyszerre csak a mért hangszer legyen online;
   a másik legyen offline. A két hangszer együttes szólása külön tartalékteszt.

A fejlesztői környezetre vonatkozó korábbi Linux-megjegyzés történeti adat.
A CI bundle/validator eredménye és a célgépes REAPER-telepítési/host-próba
külön bizonyíték; ebből a dokumentumból nem következik natív elfogadás.

## Windows x64 tesztpár

A [Windows útmutató](PREMIUM_PREVIEW_WINDOWS.md) leírja az azonos forrású két
csomag letöltését, hash-/factory-CID ellenőrzését és a telepítési smoke-ot.
A `Windows Premium Preview pair` workflow két explicit szerepet épít, nem
production installert. A numerikus pár, saját smoke, PE/statikus runtime audit,
validator és binárishashhoz kötött SDK factory-eredetjegyzék külön kapuk.
Windows ARM64 támogatást és tényleges REAPER-/ASIO-elfogadást ez nem ad.

## Hátralévő M1 munka

A [macOS mérési csomag előkészítője](NATIVE_HOST_KIT.md) ellenőrzi a két ZIP
azonosságát és üres, 112 ablakos adatlapot készít. Ez nem natív mérés vagy PASS.

1. A külön Candidate VST3 macOS pontos-head platform-CI-je és két kész
   csomag/binárishash ellenőrzése, tényleges célgépes telepítési smoke.
2. Az implementált Windows x64 csomagolás friss CI-/validator-/hash-kapuja,
   utána tényleges célgépes telepítési smoke. A Linux csomagolási fixture nem natív bizonyíték.
3. Ezután a [valós REAPER-mátrix](NEXT_RELEASE_DEVELOPMENT_PLAN.md) végrehajtása,
   a [natív adatlap](NATIVE_REAPER_CPU_ACCEPTANCE.md) kitöltésével.

A régi p99/túllépés-eredmények és a 0,44 dB RMS-korrekció elfogadása megmarad.
Nincs automatikus production-integráció, CPU-/natív elfogadás, release vagy publikálás.

