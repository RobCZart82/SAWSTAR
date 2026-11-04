# Hat új performance factory preset — 2026-11-03

Robert kérésére a 24 meglévő hang mellé hat új preset kerül. Az eredeti
kulcsok, indexek és paraméterértékek megmaradnak; a bank vége bővül 30-ra.
A hangok Classicban is elérhetők, hangszíntervezési céljuk a Premium RC.
A presetbővítés nem változtat paraméter-ID-t, presetformátumot vagy kiadási kaput.
A Windows-teszt közben feltárt chorus körpuffer-címzési hiba külön, szükséges
DSP-javításként került ugyanebbe a PR-be; a hangértékek megmaradtak.

| Név | Játék és karakter | Fő képességek |
| --- | --- | --- |
| Wheelfire (use modwheel) | Összekötött lead, kerékkel nyitható filter és oscillator edge | LP24, Drive, legato/glide, finom állandó vibrato, pressure → cutoff, chorus, pontozott ping-pong delay, reverb |
| Velvet Orbit (use modwheel) | Kitartott, lassan fejlődő térpad | BP12, két oszcillátor, pink noise, filterburkoló, két LFO: cutoff/pan, velocity → cutoff, chorus/delay/reverb, width |
| Neon Current (hold chord) | Kétoktávos fel/le tizenhatod arpeggio | LP12, rövid filterburkoló, szinkron háromszög-LFO, kis swing, pontozott ping-pong delay |
| Glass Circuit (hold chord) | Háromoktávos lefelé haladó harmincketted üvegpattern | HP12, triangle + oktáv sine, szinkron pan, chorus, ping-pong delay/reverb |
| Copper Pulse (use modwheel) | Mély hangokon pulzáló mono basszus | LP24, Drive, square + saw + triangle sub, legato/glide, szinkron cutoff-LFO, LFO2 → oscillator color |
| Polar Keys (use modwheel) | Velocity-érzékeny, lecsengő térkeys | BP12, oktávos réteg, velocity → cutoff, lassú pan, chorus, ping-pong delay/reverb |

A `(use modwheel)` jelzés a MIDI CC1-re utal; nincs szükség új vezérlőre.
Minden hang működik alaphelyzeti kerékkel is. Az arp hangoknál tarts le akkordot;
az automatikus HOLD minden factory presetben kikapcsolt. A meglévő beépített
Mod Depth út cutoffot nyit; a mátrixban LFO1/LFO2/wheel/velocity/pressure
meglévő forrásai dolgoznak. A címek és a browser lesson szövegek is segítik a játékot.

## Ellenőrzési terv és elkülönített állapot

- Implementált: hat hozzáfűzött hang, Premium output-only trimek, 30-as bank,
  játékutasítások és friss magyar/angol RC-kézikönyv.
- Helyi ellenőrzés: factory paraméterhatárok, egyedi kulcsok, state/host-state
  visszaolvasás, presetválasztás és lecsengés; CC1 hangra gyakorolt tényleges
  hatása a Premium motoron. Eredmények a fejlesztési naplóban és lentebb rögzítve.
- Headroom: a meglévő limiter előtti -6 dBFS határ változatlan. 180 alaphelyzeti
  eset (30 × 44,1/48 kHz × 1/8/16 MIDI-gyökér), továbbá az új hat hang 72
  teljes/mozgatott wheel+pressure esete. CSV `performance`: 0=alaphelyzet,
  1=CC1 és channel pressure 127, 2=kétmásodperces ismétlődő 0..126 rámpák.
  A maximális controller-értéket külön az 1-es eset vizsgálja.
- Távoli CI: külön, pontos PR-headhez kötött kapu; a korábbi #134 csomagok
  a régi, 24-es bank bizonyítékai, nem tartalmazzák ezeket az új hangokat.
- Natív/zenei elfogadás: Robert a külön exportált hat új hangot meghallgatta
  és jóváhagyta („meghallgattam jól szólnak”). Ez a hat snapshot zenei elfogadása,
  nem a későbbi VST3 csomag teljes natív elfogadása. A korábbi Windows/macOS alapműködési visszajelzés a korábbi RC-re
  vonatkozik. Ez a bővítés nem zárja le a CPU/dropout-, lifecycle- vagy K1-kaput.

Nincs release/tag/publikálás vagy automatikus plugintelepítés.

## Helyi eredmény (Apple Silicon arm64, Release)

A végleges bank 252/252 esete sikeres; a teljes bank legnagyobb csúcsa
**-6.51292972 dBFS** (a korábbi bankból). A régi 144 alapmérési sor szövegesen
is egyezik a változatlan régi bank azonos helyi mérésével.
[Teljes, limiter előtti CSV](measurements/expressive-factory-2026-11-03-arm64.csv).
Az új hangok maximumai mind a 18 ráta/hangszám/controller eseten:

| Kulcs | Maximum dBFS |
| --- | ---: |
| wheelfire | -14.00477160 |
| velvet_orbit | -11.87197540 |
| neon_current | -14.53620090 |
| glass_circuit | -10.32475960 |
| copper_pulse | -12.59536030 |
| polar_keys | -7.77149579 |

Az első új hangok szándékosan halkabb induló beállítása után a kimeneti
szinteket felfelé hangoltuk, és a Polar Keys BP-cutoffját 1200 Hz-re állítottuk.
A teljes végleges rácsot ezért új forrással ellenőriztük; küszöböt/védelmet
nem lazítottunk, kedvezőbb eredményért változatlan mérést nem ismételtünk.
A hat új hang Output Gain -6 dB, további Premium trimjük a mérés alapján 0 dB;
a régi 24 hang összes értéke és trimje megmaradt.

Ez offline helyi eredmény, nem Windows/Rosetta CI, natív REAPER vagy zenei PASS.

A hat hang külön `.sawstar` tesztcsomagként is exportált, mind a hat fájl
a végleges factory snapshotra pontosan visszaolvasható; a ZIP-hashok ellenőrzöttek.
A korábbi Premium rc1-ben PRESETS → Import → User útvonalon is kipróbálhatók.
