# Rögzített MIDI-bemenetek a natív P0 CPU-próbához

Állapot: **generálható/ellenőrzött tesztbemenet, REAPER-próba még nincs**.
A kész MIDI-fájlok a Windows gépen Python vagy fordító nélkül importálhatók.
Nem tartalmaznak plugint, presetet, audiót vagy mérési eredményt.

## Beszerzés

Sikeres `Native host kit preflight` workflow után töltsd le a
`native-host-midi-inputs` artifactot. Az artifact az alábbi három fájlt,
`MIDI-INPUTS.json` hash-/paraméterjegyzéket és rövid README-t tartalmaz:

| Fájl | Bemeneti hangok | MIDI note number |
| --- | ---: | --- |
| `midi/P0-held-01-notes.mid` | 1 | 48 |
| `midi/P0-held-08-notes.mid` | 8 | 48–55 |
| `midi/P0-held-16-notes.mid` | 16 | 48–63 |

A hangszám egyidejű MIDI note-okat jelent, **nem bizonyítottan aktív DSP-voice-okat**.
A félhangonként egymás melletti hangok műszaki terhelésre valók, nem zenei
akkordajánlatok. A P1 zenei/FX-próbához külön, előre mentett MIDI szükséges.

Az artifact `generator_source_commit` mezője a generátor pontos forrása;
nem helyettesíti a plugin-csomagok `source_commit` azonosítóját. Egy már
rögzített MIDI-fájl hashazonos másolata használható a két pluginhoz.
Egy későbbi generátorváltozás nem írja felül a korábbi tesztbemenet bizonyítékát.

A teljes [csomagellenőrző/méréselőkészítő](NATIVE_HOST_KIT.md) ugyanezeket a
fájlokat is létrehozza. A C/V/G sorok `midi_fixture` oszlopa a megfelelő fájlra
mutat; a F/P1 soroké üres marad. A provenance rögzíti mindhárom MIDI hashát.

Fejlesztői generálás, új kimeneti könyvtárba:

```sh
python3 scripts/native-host-midi.py --output new-midi-inputs --source FULL_40_CHARACTER_GENERATOR_SHA
```

## Használat (sajátgépes reaper)

1. Külön tesztprojektben állíts **120 BPM** tempót és 1,0 lejátszási sebességet.
   Importáld a megfelelő MIDI-t pontosan a projekt **0. másodpercére**.
   Ne alkalmazz item-stretchet, transzponálást, MIDI FX-et vagy loopolást.
   Ne adj a fájl mellé más MIDI-bemenetet. A note-number az irányadó;
   az oktávnevek hostbeállítástól eltérhetnek.
2. Mindkét pluginban ugyanaz a rögzített P0 állapot legyen: **Poly mód**,
   azonos paraméterek, chorus/delay/reverb kikapcsolva. A hangerőburkolónak
   tartania kell a hangot; nulla sustain vagy Mono/Legato mód nem adja az
   előírt egyidejű terhelést. A tényleges megszólaló hangszámot ellenőrizd.
3. Előre állíts biztonságos, azonos headroomot, majd mentsd a projektet és
   preseteket. A 16 hang összeadódása miatt a gyári preset alapértéke nem
   feltétlenül megfelelő. A mérés közben ne korrigálj hangerőt/limiterrel.
4. Minden mérési ablak előtt várd meg a korábbi release/FX-tail végét, és
   ellenőrizd, hogy nincs beragadt hang. Válaszd az adott szerepet online-nak;
   a másik offline legyen. Indíts a projekt nullpontjáról minden ablakhoz.
5. Note-on: **0 s**; bemelegítés: **0–10 s**; megfigyelés: **10–70 s**;
   note-off: **70 s**. A MIDI markerek `OBSERVE_START` és `OBSERVE_END`
   eseményei ezeket az időpontokat jelölik; a host nem feltétlenül jeleníti
   meg őket projektmarkerként. Az idővonalat külön ellenőrizd.
6. A 70. másodpercben ne állítsd le automatikusan a transzportot: a note-off
   után a teljes tail fusson ki. A MIDI fájl vége nem tail-időkorlát.
   A megfigyelés végén keletkező release nem része a 60 s tartott terhelésnek.
7. AUTO esetben a korábban mentett, mindkét pluginra azonos cutoff-
   automatizálást használd. **A MIDI nem tartalmaz cutoff-CC-t**, és nem
   helyettesíti a mentett hostautomatizálást. STATIC esetben nincs cutoff-görbe.

A terv 28 eset / 112 AB/BA ablak száma változatlan. A MIDI-fájl ismételt,
nullpontról indított lejátszása a négy kijelölt ablak része; hibás eredményt
nem törlünk kedvezőbb ismétlésért. Újrapróbálás külön ID és indok.
44,1 kHz, magas ráta, tail/idle, sustain/voice-steal és P1 zenei próba külön feladat.

## Ellenőrzés és korlát

Standard MIDI File format 0, egy track, PPQ 480, 500 000 µs/negyed (120 BPM),
channel 1, velocity 80. Minden note egyszer indul, és egyszer kap note-offot;
nincs programváltás, CC, sustain, pitch bend, SysEx vagy rejtett pluginvezérlés.
Az eredetjegyzék SHA-256-a az importált fájléval egyezzen.

A teszt külön MIDI-eseményolvasóval ellenőrzi a chunkhatárokat, abszolút tick-
időket, tempót, note-on/off párokat és a megfigyelési szakasz bemeneti polifóniáját.
Ez fájl-/protokollellenőrzés; nem REAPER-import, hangzás-, CPU- vagy natív elfogadás.
Az eltérő projekt-tempó/item-stretch miatti időhibát a fájl hash-a nem mutatja ki.
K1 oka továbbra ismeretlen, e tesztbemenet nem K1-javítás.
