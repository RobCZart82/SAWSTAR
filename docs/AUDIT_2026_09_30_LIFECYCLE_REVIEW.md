# A 2026-09-30-i külső audit ellenőrzése

Kiinduló main: `dce55cc`; ellenőrzés közben #27 beolvadt (`144d20d`).
A jelentés állításai nem automatikusan elfogadott hibák. Az alábbi státuszok
forrásellenőrzést és célzott helyi próbát jelentenek, nem teljes natív hostauditot.
A hivatkozott coverage-százalékokat ebben a körben nem mértem újra.

## Új, elsőbbséget kapó megállapítások

| Téma | Ellenőrzés és döntés |
| --- | --- |
| Zero-frame + editor overflow | Reprodukált integrációs sorrendhiba. A production patchből kiemelt post-audio callback a valódi BlockMidiQueue/EditorMidiTracker/Synth mellett túl korán fogyasztja el a jelzést. A régi kódon bukó új teszt a javítással sikeres. |
| Bypass + MIDI felhalmozódás | A pontosan rögzített upstream `d54f690` adapter igazolja: MIDI-átadás történik, de bypassnál ProcessBuffers helyett PassThroughBuffers fut. A visszatéréskori késői visszajátszás nyitott, külön policy és regresszió szükséges. |
| PITCH kerék bezárás | Forrásszinten alátámasztott hiány: PerformanceWheel a rugós visszaállítást OnMouseUpban küldi, #29 csak Keyboardot kezel. #29-et változatlanul nem olvasztjuk be. Csak aktív GUI pitch-gesztus fejezhető be; MOD latching és hostvezérlés megőrzendő. |
| Tail=0 | A plugin nem állít tailt, az iPlug alapértéke nulla. Javítandó host-szerződés; a konkrét renderlevágás hostfüggő és natív próbát igényel. |

## A mostani célzott javítás határa

A post-audio overflow callback csak pozitív mintaszámú, nem bypassolt feldolgozás
után fogyaszthatja el a jelzőt. Zero-frame és bypass híváskor az eredeti jelző
függőben marad. A következő valódi blokkban a szokásos, offset=0 GUI-események
előbb bekerülnek a trackerbe, majd snapshot készül; a következő blokk célzottan
felengedi az editor hozzájárulását. Nincs új globális panic vagy hangidő-korlát.

A teszt az upstream telített FIFO elfogadott Note On / elveszett Note Off
esetét, a wrapper sorrendjét, a következő valódi blokkot, a felengedést és a
release utáni nulla aktív voice-ot vizsgálja. A FIFO és a wrapper vezérlése
headless modell; a queue/tracker/synth és a callback kódja valódi production
kód. Ez nem tényleges VST3 hostteszt. ASan/UBSan is sikeres.

A bypass alatti MIDI visszajátszása ettől még megmarad. A nem nulla, jövőbeli
editor-offsetek teljes recovery-szerződése szintén külön ellenőrzendő: pozitív
blokk önmagában nem bizonyít minden ilyen esemény feldolgozását.

## Fontos pontosítások az audit következtetéseihez

- A #27 reset FIFO és #28 orphan release célzott, külön javítás. #27 mainben;
  #28 a main-frissítési követelmény miatt új CI-futás alatt. A védelmet nem
  kerüljük meg admin merge-dzsel.
- A PITCH close-nál nem elég minden kerékre meghívni OnMouseUpot: aktív editor
  gesztus nélkül ez felülírhatná a host bend állapotát. A visszajelzett MIDI nem
  bizonyít editor tulajdonjogot.
- A tail becslése nem általában `max(amp, delay, reverb)`. Soros gerjesztésnél az
  amp release késői része még meghajthatja a delayt és reverbet, ezért az effektív
  lánc teljes válaszát kell korlátolni/mérni. Reverb decay sem feltétlenül azonos
  a választott -90/-120 dB küszöbig mért idővel.
- Publish/Clear atomi ütközés önmagában nem memóriahiba. Dokumentált reset-határ
  szükséges; csak ennek meghatározása után dönthető el, kell-e epoch-mechanizmus.
- A latest-value GUI wheel összevonás szándékos, dokumentált viselkedés. Nem
  állítjuk vissza veszteséges FIFO-ba.
- A coverage százaléka nem tanúsítja a natív adapter-életciklust. A 37,7% lefedetlen
  ág nem 37,7% hibás kód; célzott állapotkombinációk szükségesek.

## További, hasznos, de nem mostani P1-javítások

- Diszkrét presetértékek: Sanitize finite/clamp, saved snapshot és iPlug stepped
  paraméter kanonizálása eltérhet. Valós szerződéseltérés; a dirty-marker tünethez
  külön teszt kell. A host-state kompatibilitást nem szabad véletlenül szűkíteni.
- Host-state és standalone preset ugyanazt a DecodeState-ot használja. Szigorú
  file-dekóder és kompatibilis host-dekóder indokolt külön tervezési feladat.
- Preset apply több paraméterfrissítése nem atomi; block-alapú paraméterolvasás
  nem sample-accurate automatizálás. Ismert korlátozások, új hallható hibát ebben
  a körben nem bizonyítottunk.
- MIDI insertion sort legrosszabb esete négyzetes, ApplyEngineControls blokkonként
  teljes leképezés. Profilozás szükséges; ne optimalizáljunk mérés nélkül.
- Fájlrendszer/symlink/backup/rename/fsync és double-click kerékviselkedés:
  későbbi célzott vizsgálat, nem újonnan igazolt súlyos adatvesztési hiba.
- A korábban javított ADSR, glide, detune, delay, release ordering és verziómetaadat
  témák nem nyílnak újra bizonyíték nélkül. A click/pop kutatás lezárt marad.

## Frissített sorrend

1. Zero-frame overflow javítás CI és natív ellenőrzése.
2. #28 frissített ágának ellenőrzése és beolvasztása.
3. #29 PITCH gesture cleanup kiegészítése, MOD/host kontrolltesztekkel.
4. Bypass policy: állapot továbbfuttatása némított kimenettel VAGY definiált
   boundary-reset; előbb specifikáció és wrapper-regresszió, utána implementáció.
5. Deaktiválás/újraaktiválás és reset kombinált hostteszt, ARP/sustain/same-note.
6. Valós DSP-láncon alapuló véges tail-szerződés és offline render teszt.
7. P2: reset epoch, diszkrét kanonizálás, strict import, preset-tranzakció,
   automatizálás blokkfüggése, natív adapter harness.
8. Profilozás, filesystem és csak ezután külön hangminőségi fejlesztés.
