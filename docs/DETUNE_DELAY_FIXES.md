# Detune és Delay célzott javítások

2026-09-19 — helyi, ki nem adott fejlesztés.

## Detune

A SetFreq() korábban a simított unison-arányokat azonnal a célértékükre
állította. Ez akkor is megszakította a folyamatban levő detune-automatizálást,
ha a hívás változatlan MIDI-alaphangot állított be. A hiba szóló hang
újraindításakor vagy újrafelhasználásakor is releváns; nem minden mintán
érkező hostautomatizálásról van szó.

A SetFreq() most megőrzi a detune-simító állapotát. Az alaphang továbbra is
azonnal változik, új Glide vagy hangváltási fade nem került a motorba.
Az üres szólamok kezdő beállítását a meglévő SnapToTargets() végzi.

A detune_continuity teszt régi kódon elbukott (44,1 kHz saw: maximális
stereo eltérés 1,09218); javítva mind a négy hullámformán és 44,1/48/96 kHz-en
bitazonos a redundáns SetFreq-hívásos és az azt nem tartalmazó változat.
Külön ellenőrzés igazolja, hogy egy valódi oktávváltás azonnal végbemegy,
és az idle inicializálás továbbra is használható.

## Delay

Init után a Delay 350 ms-ból simította az olvasási távolságot a frissen
beállított értékre. A 10 ms-os impulzuskontroll első visszhangja régi kóddal
44,1 kHz-en a 1997. mintán (45,28 ms) jelent meg a várt 441. minta helyett.

Az első Process az addig beérkezett utolsó késleltetési időt veszi át.
Ezután a megszokott automatizálási simítás működik. A mix, feedback, tone,
ping-pong és effekttörténet szabályai nem változtak. A Clear nem indítja
újra az inicializálási logikát; Init igen.

A delay_initial_time teszt szabad 10 ms-os és tempóhoz kötött 125 ms-os
időt ellenőriz 44,1/48/96 kHz-en. Minden eset a várt mintapozícióban,
legfeljebb egy mintányi interpolációs eltéréssel szólal meg. A Clear után
régi jeltörténet nem szivárog vissza.

## Ellenőrzés és korlátok

- 9 tesztprogram sikeres: detune_continuity, delay_initial_time, delay,
  output_gain, parameter_transitions, idle_start, glide_history,
  release_order, idle_voice_reuse.
- A meglévő delay teszt 192 kHz-et, élő időautomatizálást, ping-pongot,
  bypass-t, magas feedbacket és érvénytelen paramétereket is ellenőriz.
- 27 normál hangzáskontroll hash-e bitazonos a korábbi fejlesztési alappal.
  Ez nem állítás minden preset és automatizálás azonosságáról: a javított
  detune-átmenet és Delay-indulás szándékosan eltér a hibás működéstől.
- A regressziók CMake-regisztrációja elkészült; most közvetlen helyi C++
  fordítással futottak. Új VST3-build, telepítés és hostteszt nem történt.
- A GUI és presetformátum változatlan. Nincs GitHub-push vagy release.

A lezárt Mono-pukkanáskutatás nem nyílt újra. Ezek önállóan igazolt
paraméterkezelési és inicializálási javítások.
