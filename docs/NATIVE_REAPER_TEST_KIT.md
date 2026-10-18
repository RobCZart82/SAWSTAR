# Natív REAPER-tesztkészlet előkészítése

Állapot: **2026-10-18, implementált előkészítő; nem natív mérés vagy elfogadás**.
A #115 beolvadt, a `547f84d4f72a792aaf0b229c9d73b7a8a00fae40` main mind
4 fő workflow-ja / 11 jobja sikeres. A következő tényleges kapu mindkét
célgép telepítési smoke-ja és a REAPER-próba.

Az eszköz nem telepít és nem tölt be plugint, nem indít CPU-kampányt,
nem módosít DSP-t, presetet vagy hangerőt. Előkészíti az azonos bemeneteket
és az üres adatlapot a [release terv](NEXT_RELEASE_DEVELOPMENT_PLAN.md)
28 esetes, 112 ablakos C/V/G/F mátrixához.

## Ellenőrzött csomagpár

A [csomagazonosító jegyzék](test-fixtures/native-preview-pair-2026-10-18.json)
négy csomagja azonos `c55ca8f826c414064c62a3c71961533aeff4fa52` PR-forrásból
készült; ez a #115 ellenőrzött, beolvadt fejlesztési headje, nem a squash SHA.
A Windows és macOS jobok ekkor sikeresek, a tényleges binárisok validáltak voltak.
Az archívumok letöltése után a GitHub artifact-ZIP digestje, a belső csomag-ZIP
`.sha256` fájlja és minden manifest-fájlhash helyben újraellenőrizve.
Ez csomagintegritási bizonyíték; a jelen Linux környezetben nem futott REAPER.

| Platform | Reference artifact | Candidate artifact |
| --- | --- | --- |
| Windows x64 | [12116899452](https://github.com/RobCZart82/SAWSTAR/actions/runs/39077670677/artifacts/12116899452) | [12117229524](https://github.com/RobCZart82/SAWSTAR/actions/runs/39077670677/artifacts/12117229524) |
| macOS Universal | [12118310254](https://github.com/RobCZart82/SAWSTAR/actions/runs/39077670551/artifacts/12118310254) | [12118335254](https://github.com/RobCZart82/SAWSTAR/actions/runs/39077670551/artifacts/12118335254) |

Az artifact lejárhat; a jegyzék lejárati idejét és a pontos forrást őrizd meg.
Ne cseréld le a pár egyik tagját másik forrású/frissebb letöltésre.
Az offline célgépes 32 grides kampányok változatlan történeti bizonyítékok;
a mostani kit nem azok újrafuttatása vagy átnevezése.

## Használat

Python 3 szükséges. A GitHub artifact külső ZIP-jét először bontsd ki;
a scriptnek a benne található **plugin-csomag ZIP-et** add meg. Mellette legyen
az eredeti, azonos nevű `.zip.sha256` fájl.

```sh
python3 scripts/prepare-native-reaper-kit.py --reference SAWSTAR-Premium-Preview-Windows-x64.zip --candidate SAWSTAR-Premium-Candidate-Windows-x64.zip --session-id windows-first-20261018 --output native-session-windows-20261018
```

macOS alatt ugyanígy a két `*-macOS-Universal.zip` fájl a bemenet.
Windows alatt a telepítéstől függően `python` a parancs neve.
Minden platform/sorozat új session ID-t és **új outputkönyvtárat** kap.
Meglévő könyvtárat az eszköz nem ír felül, még ha üres, akkor sem.
Hiba esetén nem próbál újra mérést és nem ad elfogadást.

A script ellenőrzi a külső csomagchecksumot és minden ZIP-member hashát,
a szerepet, platformot, forráscommitot, binárishashot, eltérő pluginazonosságot,
közös filtert, 32 mintás latencyt és elfogadott level policyt. A hiányzó vagy
kevert fájl, szerep-, forrás-, platform- vagy hashhiba blokkol.
Nem ellenőriz újra digitális aláírást vagy GitHub CI-t; az artifact eredetét
és a pontos-head CI-t a fenti jegyzékkel/GitHubbal külön kell azonosítani.

## Mit készít?

- `session.json`: az ellenőrzött csomag-/binárishashok és üres környezeti,
  preset-/projekt-/envelope-azonosítási mezők. Állapota `prepared_not_measured`,
  `native_host_acceptance=false`, `production_promotion_allowed=false`.
- `case-plan.csv`: 28 külön eset, előbb a /128 blokk, utána /64.
  A /128 AUTO négy filtermódja az első, 18 perc 40 s nettó előszűrés.
- `observations.csv`: 112 üres ablak; esetenként Reference→Candidate,
  majd Candidate→Reference. 10 s warmup + 60 s megfigyelés, összesen nettó
  130 perc 40 s/platform. Az üres számláló nem nulla hiba.
- `held-1-notes.mid`, `held-8-notes.mid`, `held-16-notes.mid`: azonos, determinisztikus
  SMF0 MIDI-bemenetek, 480 PPQ / 120 BPM, MIDI channel 1, velocity 100.
  A hangok 0–80 s között tartanak; a fájl vége 82 s. A mérési ablak 10–70 s.
  A 16 hang C-dúr hangkészletű technikai klaszter, MIDI 48–74 számokkal;
  az 1/8 hang az előre rögzített lista első 1/8 hangja. Nincs release-átfedés.
- `automation-plan.csv`: normalizált cutoff-pontok. 0–10 s: 0,2;
  majd 5 s-onként 0,8 / 0,2, 70 s-ig. Ez **terv, nem REAPER-be importált envelope**.
  Egyszer hozd létre lineáris szakaszokkal a hostban, mentsd RPP-be, majd
  pontosan másold a másik szerephez. STATIC esetben állandó 0,5 és AUTO kikapcsolva.
- `READ-ME.txt`: rövid futtatási emlékeztető. A generált fix bemenetek hashai
  a sessionben szerepelnek; az `observations.csv` szándékosan kitölthető.

A 82 s nem teljes tailgarancia: a MIDI-stop utáni teljes release/FX-tail és
60 s idle külön hostpróba. A tartalék-/stressz-/GUI-/lifecycle-/minőségi és
magasráta-kapuk nem lesznek késznek jelölve a 112 üres sor létrejöttétől.

## Hostban még beállítandó

1. Azonos forrású, helyes Reference és Candidate telepítése; About/build-/binárisazonosság
   és külön presetmappák ellenőrzése a [Windows](PREMIUM_PREVIEW_WINDOWS.md),
   illetve [macOS/pár](NATIVE_PREVIEW_PAIR_QUALIFICATION.md) útmutató szerint.
2. Új, mentett projekt 120 BPM-en; MIDI import time zero-ra. Kijelölt tartomány:
   48 kHz /128, majd /64, tényleges driver-puffer visszaellenőrizve.
3. **P0:** egy elmentett Init-alapú kontrollhangszín két bekapcsolt Saw bankkal,
   Poly, ARP OFF, amp sustain >0; azonos detune/unison/mixer/borítékok,
   filter wet 100%, chorus/delay/reverb OFF. Ne maradjon amp-/filter-moduláció
   a nem kijelölt kontrollban. Ellenőrizd, hogy a 10 s warmup után is ténylegesen
   1/8/16 hang szól, nem csak MIDI-note van a fájlban. Pluck/mono/ARP nem helyettesítő.
4. **P1:** előre kijelölt sustained Poly FX-hangszín, például egy mentett
   SuperSaw One kiindulás; filtermód és minden beállítás futás előtt rögzítve.
   A P1 adatai nem automatikusan a P0 vagy egy másik preset adatai.
5. A headroomot a legsűrűbb terhelésen **előre** ellenőrizd és mentsd; szükséges
   tesztpreset-Volume beállítás mindkét szerepnél azonos. Mérés közben nincs
   gainváltás, új limiter/AGC vagy gyári presetcsomag átírása. A 0,44 dB döntés marad.
6. Ugyanaz a mentett preset/MIDI/envelope mindkét szerepnek; GUI zárt/nyitott
   az adatlap szerint. A másik hangszer OFFLINE, a némítás nem elegendő.
7. Töltsd ki a környezetet, rögzítsd a mérő pontos nevét és a tartalékkritériumot;
   FX CPU, RT CPU és saját DSP% külön. Nem hozzáférhető xrun = N/A, nem nulla.

A kit nem mér host-CPU-t, nem számol p99-et szemmel olvasott százalékból és
nem bírálja el Robert helyett a zenei/stabilitási eredményt.
Az elfogadási döntést a [natív jegyzőkönyv](NATIVE_REAPER_CPU_ACCEPTANCE.md)
szerint, pontos buildre, célgépre és tartományra kell rögzíteni.
