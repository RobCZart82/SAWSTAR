# A felhasználói hibajegyzék ellenőrzése

Vizsgált alap: `1a46fd458b963338825476acf23f6554cbb0b2f9`.
Forrás: a felhasználó JAVÍTÁSOK-fejlesztések.txt jegyzete, 13 számozott észrevétellel.
Ez az aktuális kód ellenőrzése, nem bizonyíték arra, hogy egy korábbi verzióban mi
szerepelt. A jegyzetben használt sorszámok helyett az alábbi függvények az irányadók.

| # | Eredmény | Indok és kódhely |
|---|---|---|
| 1 | Nem igazolt; a jelenlegi sor teljes | `Synth::SetVoiceMode` beállítja mindkét oszcillátor frekvenciáját. A `v.fun[...]` nincs a fájlban; valószínűleg csonkolt megjelenítésből származott. A csonkolt C++ egyébként fordítási hibát jelentene, nem önmagában futásidejű szegmentálhibát. |
| 2 | Nem áll fenn a leírt túlindexelés | `Synth::Midi` a note/value értékeket ellenőrzi; `MonoMidi` privát és ezen keresztül kap adatot. `status & 15` 0–15 csatornát ad. `channel*128+note` ezért 0–2047. `SelectMono` a 0–2047 tartományból választ, ebből származtatja a csatornát. |
| 3 | Nem hiba | `voices_` nem üres, fix 16 elemű `std::array`; a `min_element` ezért nem ad `end()`-et. |
| 4 | A megjelölt drift nem igazolt | `monoPitch_`, cél és lépés double. A glide végén a kód pontosan `monoTarget_`-re állítja a pitch-et; legfeljebb 2000 ms a glide. Lebegőpontos kerekítés létezik, de a jegyzet nem mutat hallható intonációs hibát vagy tartós driftet. |
| 5 | A megjelölt drive-instabilitás nem igazolt | `LowPass::Process`: drive 0–24 dB, így a gain legalább 1, a nemnulla drive nevezője `tanh(gain)` legalább kb. 0,76. Van nullára igazítás és kis szűrőállapotok törlése. A teljes DSP minden lehetséges denormálisának hiányát ez nem bizonyítja, de a leírt ok nem áll fenn. |
| 6 | A jelenlegi paramétereknél nem hiba | A nevező `log(max/min)`, nem `log(min)`: a minimum=1 önmagában szabályos. A specifikációkban max>min és minden logaritmikus minimum pozitív. Hibás, kívülről összeállított specifikáció ellen külön védelem lehetne, de a függvény szerződése a `kParameters` elemeire vonatkozik. |
| 7 | Téves hibaleírás | `Lfo::Process` a hullámot az előző lépésben már körbeforgatott fázisból számolja, majd növel és ugyanabban a hívásban alkalmazza a floor-t. Ez helyes modulo-1 művelet. A tempószinkron BPM-követés; nem host-idővonalhoz igazított fáziszárás, ahogy a kód megjegyzése is jelzi. |
| 8 | Téves hibaleírás | `w=1..3` pontosan az `alternatives_[0..2]` sorokat éri el; minden sor 7 oszcillátora inicializálódik. A 3-as hullámforma engedélyezett, a súlya közelít 1-hez. A 2-re korlátozás eltüntetné a szinuszt. A két square inicializálás közül az egyikből külön integrálás készít háromszöget. |
| 9 | A jelenlegi kód már tartalmazza a védelmet | `Delay::Init`: közvetlenül a puffer létrehozása után `write_=valid_=0`. A hiányzó resetre épülő példa nem alkalmazható. |
| 10 | A jelenlegi kód már tartalmazza a védelmet | `Reverb::Init`: közvetlenül a puffer létrehozása után `write_=valid_=0`. |
| 11 | Tervezett korlátozás, nem igazolt numerikus instabilitás | Négy korlátozott út összeadódik és a célparaméter tartományára korlátozódik. A slew nem hoz létre korlátlan növekedést. A javasolt állandó 1/4 szorzó egyetlen út mélységét is negyedére csökkentené; ez hangtervezési változás lenne, nem általános hibajavítás. |
| 12 | Részben helyes amplitúdóészrevétel, hibás típushiba-következtetés | A 16 sor és az extra fehér komponens 1/8 skálázással elméletileg ±2,125 körüli belső tartományt enged. Nincs ±1 csúcsgarancia. A double összeg szándékosan pontosabb, a float operandus double-lé lép elő; a visszaalakítás kerekít, nem egészre csonkol. Belső ±1 fölötti float nem önmagában clipping: a synth további skálázást és kimeneti védelmet alkalmaz. Az itt javasolt hard clamp megváltoztatná a zaj spektrumát. Külön hangerő-/statisztikai döntésként érdemes kezelni, nem bizonyított torzításként. |
| 13 | Téves hibaleírás | Ha a maradék kisebb a lépésnél, a clamp pontosan `target_-extra_` értéket ad. Kikapcsoláskor ez `-extra_`, tehát a következő összeadás pontosan nullát eredményez. Ez véges rámpa, nem aszimptotikus exponenciális közelítés. A `master_width` teszt ellenőrzi a pontos bypass-t. |

Kód: [Synth](../src/engine/Synth.cpp), [Synth adattagok](../src/engine/Synth.h),
[LowPass](../src/dsp/LowPass.cpp), [Parameters](../src/plugin/Parameters.cpp),
[LFO](../src/dsp/Lfo.h), [SevenSaw](../src/dsp/SevenSaw.cpp),
[Delay](../src/dsp/Effects/Delay.h), [Reverb](../src/dsp/Effects/Reverb.h),
[Modulation](../src/dsp/Modulation.h), [PinkNoise](../src/dsp/PinkNoise.h),
[Width](../src/dsp/Effects/Width.h).

## Preset-kiterjesztés

A felsorolt rövidítések névválasztási javaslatok, nem hibák. Jelenleg `.sawstar` a
kiterjesztés. Ez a változtatás nem nevez át meglévő fájlokat és nem vezet be bankot.
A külön preset/bank végződésről akkor célszerű dönteni, ha bankformátum is készül.

## Save As

A felhasználói észrevétel helytálló: eddig a panel névmezőjéből közvetlenül mentett.
Most a natív mentési ablak nyílik meg, előre kitöltött névvel és User könyvtárral.
Ott a név és hely szerkeszthető; Mentés véglegesít, Mégse nem ír fájlt és nem cserél
hangot. A LOAD meglévő működése változatlan. A meglévő fájl felülírását a fájlkezelő
réteg továbbra sem engedi; új név szükséges. A jegyzet alapján DSP-algoritmust nem
változtattunk meg, mert a megjelölt kritikus hibák nem igazolódtak.

## A Save As módosítás ellenőrzése

Kód: `69cc028fe980d7a30007d91a667f72512cb9344e`.

- [macOS CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34264700816)
- [Windows CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34264700757)

A natív mentési ablakot ebben az ellenőrzésben nem próbáltuk ki kézzel REAPER-ben.
A mentési codec és a fájlkezelési regressziós tesztek a build részei.
