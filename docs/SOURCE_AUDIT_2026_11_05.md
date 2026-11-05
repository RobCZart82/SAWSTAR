# Forrásaudit és ARP bypass javítás — 2026-11-05

## Kiindulás és hatókör

Vizsgált main: `e50218a41fc4329327c9dab67da987b49ec37898`.
A helyi másolat GitHub-forráslekérésekből készült, nem teljes submodule checkout.
A felülvizsgálat az adapter MIDI/state útját, a Classic engine note/controller
kezelését, az ARP/queue/editor tulajdonlást, a preset fájlműveleteket,
az ADSR/LFO/SUB/filter/effect kód kiválasztott részeit és a src README-ket érintette.
Ez részleges forrásaudit, nem a teljes repository hibamentességének igazolása.

## Igazolt és javított találatok

| Prioritás | Találat | Javítás |
| --- | --- | --- |
| P2, teljesítmény | Az ARP bypass útja note eseményenként végigolvasta a 2048 gyökérhelyet, kibontotta az oktávokat és rendezte a mintát, amelyet kikapcsolt állapotban nem fogyaszt a Process. Ez az audio callbackben felesleges munka. | Rebuild bypassban üres számlálóval azonnal visszatér. A fizikai key/forwarded számlálók és a közvetlen MIDI-továbbítás megmaradnak. Off→On váltáskor Set újraépít a megtartott gyökerekből. |
| P3, dokumentáció | src/plugin/README még néma shellt és be nem kötött motort írt le. | A tényleges render/MIDI/state/engine-választás leírása. |
| P3, dokumentáció | src/midi és src/dsp README-k az elkészült ARP/DSP funkciókat jövőbeli tervként mutatták. | Jelenlegi működés és felelősségek. |
| P3, dokumentáció | src/dsp/Effects szerint még nem volt effekt. | Chorus→Delay→Reverb→Width út és bypass/tail viselkedés. |
| P3, dokumentáció | src/engine szerint a voice reuse crossfade még halasztott volt; a kijelzőt sounding voice-ként írta le. | A 3 ms poly output-correction ramp és a fizikailag tartott MIDI-pitch snapshot pontosítása. |
| P3, dokumentáció | src/presets még 24 factory hangot írt. | 30 hang: 7 Template és 23 további hang. |
| P3, dokumentáció | src/gui és Controls az induló shellt vagy csak fenntartott helyet írták le. | A megvalósult három oldal, vezérlők és szálhatárok. |

A bypass optimalizálása nem numerikus DSP-javítás, nem K1 gyökérok, és nem
bizonyítja, hogy korábban hallható dropout keletkezett.

## Ellenőrzés

- Új `tests/arpeggiator_bypass.cpp`, DaisySP nélkül is futó CTest-cél:
  változatlan bypass eseményadat/sorrend, ismételt pitch tulajdonlás,
  csatorna/velocity megtartás, elengedett/CC123-mal törölt gyökerek hiánya,
  bypassban módosított mode/octave érvényesítése bekapcsoláskor.
- Ugyanez a funkcionális teszt az eredeti változattal is sikeres: a javítás
  teljesítményt javít, nem korábbi hangfunkciós teszthibát javít.
- Az új teszt GCC AddressSanitizer + UBSan mellett is sikeres.
  LeakSanitizer a felügyelt/ptrace környezetben nem működik, ezért a sikeres
  ismétlés `ASAN_OPTIONS=detect_leaks=0` mellett történt; ez nem leak-ellenőrzés.
- Külön eredeti/javított ARP fordítás összevetése: 4 seed × 10 000 lépés,
  paraméterváltások, note/pedal/controller/bend és Process hívások;
  **76 532 kimeneti MIDI-esemény teljes sorozata bájtról bájtra azonos**.
- 18 meglévő teszt közvetlen GCC C++17 fordítással sikeres:
  parameters, state, state_roundtrip, host_gui_state, user_presets,
  preset_library, preset_reliability, strict_preset_files, preset_import_index,
  preset_import_job, preset_canonicalization, lfo_smoothing, sub_oscillator,
  scope, meter, confirmation_wrap, display_state, editor_wheel_gesture.
- CMake nem telepített ebben a környezetben. A DaisySP-függő motor/ARP/queue
  integrációs tesztek, illetve a generált gui_scale_hooks.inc-et igénylő teszt
  itt nem fordultak le. Ezek környezeti korlátok, nem igazolt termékhibák.
  A teljes CMake, natív build és integrációs teszt a PR CI feladata.

### Megismételhető lokális mikromérés

`experiments/midi/benchmark_arp_bypass.cpp`:
128 tartott gyökér, Played/4 oktáv beállítás, bypassban 20 000 további
note-on/off pár, majd bekapcsolás és a megtartott minta fogyasztása.
Azonos GCC `-std=c++17 -O2`, azonos gép; csak a bypass párhurok időmérése.

| Változat | Három ismétlés (ms) |
| --- | --- |
| Eredeti | 533.483 / 556.284 / 528.631 |
| Javított | 0.162555 / 0.162555 / 0.241003 |

Mindkettő 41 859 eseményt, azonos `5338532675975936500` ellenőrzőösszeget adott.
A fordítótól és géptől függő mikromérés nem általános CPU-százalék, nem natív
host deadline/xrun elfogadás, és nem a teljes plugin sebességnövekedése.

Fordítás a checkout gyökeréből:

```sh
g++ -std=c++17 -O2 -Isrc experiments/midi/benchmark_arp_bypass.cpp -o /tmp/arp-bench
/tmp/arp-bench
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -Isrc tests/arpeggiator_bypass.cpp -o /tmp/arp-test
ASAN_OPTIONS=detect_leaks=0 /tmp/arp-test
```

Összehasonlításhoz azonos benchmarkot kell fordítani a szülő és a javított
Arpeggiator.h-val; az eredeti revisionben az új benchmark még nincs jelen.

## Nyitott vizsgálati irányok, nem igazolt új hanghibák

1. **Aktív ARP sűrű MIDI-terhelése.** Bekapcsolva továbbra is minden gyökérváltozás
   újraépít/rendez, akár 8192 mintahellyel. Ez véges, de egy callback 1024 eseménye
   jelentős költséget jelenthet. Mérendő blokkhatár/deadline mellett, több csatornával,
   ritka és sűrű akkordváltásokkal. A mintaválasztás időrendjét megőrző megoldás
   kell; az események egyszerű blokk-végi összevonása megváltoztathatja a hangot.
2. **UI fájlműveletek.** A háttérimport mellett Refresh/Save/Rename/Favorites
   útvonalakon marad szinkron fájlrendszer-munka, és az interprocess lock akár
   két másodpercig várhat. Ez meglévő részleges R1/GUI-folyamatossági feladat;
   nem bizonyított audio-thread blokk. Lassú/távoli meghajtóval külön mérendő.
3. **MeterMailbox RT-felső korlát.** A Publish compare_exchange_weak ciklusa nem
   rendelkezik kódszintű próbálkozási limittel. Egy producer/consumer és normál
   GUI-frissítés mellett nem találtunk hibát; szigorú callback-idejű auditban
   külön vizsgálandó, hogyan őrizhető meg a peak/timestamp szemantika véges
   próbálkozással. Ez nem lock vagy igazolt dropout.

## Következő fejlesztési lépések

1. PR teljes natív CI-je a pontos head commiton; a CMake-cél bekötésének ellenőrzése.
2. Aktív ARP deadline-profilozása; csak méréssel igazolt további optimalizálás.
3. A konkrét kiadásra szánt RC CPU/dropout/GUI/state/preset elfogadása
   **(sajátgépes reaper)**.
4. K1 továbbra is ismeretlen eredetű, lezárt negatív reprodukciós kampánnyal;
   ez a változás nem zárja le és nem írja át a korábbi bizonyítékokat.
5. A publikálás továbbra is külön jóváhagyás. Nincs release, tag vagy
   Premium-default átállítás ebben a változásban.
