# Output Volume / Boost átmeneti kilengés

2026-09-15 — helyi, ki nem adott fejlesztés.

## Igazolt hiba

Azonos összerősítésű (Volume 0 dB, Boost 0 dB) és (Volume −24 dB,
Boost +24 dB) beállítás közötti, egy audio-minta előtt végrehajtott
váltáskor a külön simított lineáris erősítések szorzata átmeneti többletet
hozott létre. A régi kódon a regresszió elbukott: 44,1 kHz-en a referencia
kimenetéhez mért maximum arány 4,478, körülbelül +13,02 dB.
A teszt alacsony forrásszinttel kizárja a limiter beavatkozását.

## Javítás

Az összesített Volume × Boost célértéket egyetlen, az eddigi 5 ms-os
időállandójú simító követi. A két felhasználói paraméter, tartományuk,
mentési formátumuk és végső erősítésük változatlan. A GUI, oszcillátor,
szűrő, burkoló és effektfeldolgozás nem változott. Ez nem a lezárt
Mono-pukkanáskutatás javítása.

## Ellenőrzés

- output_gain: kompenzált váltás mindkét irányban 44,1/48/96 kHz-en;
  maximális arány 1 a megadott tolerancián belül.
- Önálló Volume és Boost automatizálás: monoton, túllövésmentes beállás.
- parameter_transitions, idle_start, glide_history, release_order,
  idle_voice_reuse: sikeres offline motoros tesztek.
- 27 hangzáskontroll (3 mintavételi frekvencia × 3 hangmód × 3 hullámforma)
  kimeneti hash-e bitazonos a javítás előtti helyi fejlesztési állapottal.
  Ez nem teljes presetbankra vagy minden automatizálásra vonatkozó állítás.

A regresszió a CMake tesztlistájában szerepel, de ebben a körben közvetlen
fordítóhívással futottak a motoros tesztek. Új VST3, telepítés, Windows-host
ellenőrzés és publikálás nem történt. A megváltozott automatizálási átmenetet
később a tényleges VST3 adapteren keresztül is ellenőrizni kell.
