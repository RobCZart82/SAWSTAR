# Lookup motor komponensprofil — 2026-10-08

A következő CPU-optimalizálás előfeltétele: az aktuális, rátafüggő
SIMD/reciprok/lookup kutatási motor költségének feltérképezése Windows és
macOS alatt. Ez offline diagnosztika; nincs production aktiválás vagy natív
REAPER-elfogadás. A blokkidő p99/túllépési mérését nem helyettesíti.

## Mérési szerződés

`sawstar_premium_lookup_components --components`: 48/96/192 kHz, négy
szűrőmód, 16 voice kontextus, 20 dB Drive, három független ismétlés.
Minden eset új állapotot kap; 0,25 s warmup után 16 384 frame időzített.
Az Init, memóriafoglalás és warmup kívül, az energiadiagnosztika és a ciklus/
indexelés belül van a timeren. Pontosan 360 nyers sorból 120 összesítő sor készül.

| Stage | Tényleges terhelés |
| --- | --- |
| oscillators | 16 bank, két SevenSaw és félfrekvenciás SUB; eltérő hangmagasság, állandó mix. Nincs noise/envelope/pitch-routing. |
| modulation | Két globális LFO, egy matrix.Process és 16 Evaluate eltérő velocityvel. Nincs teljes voice-envelope vagy controller-simítás. |
| fx | Egy globális Chorus → Delay → Reverb lánc. A CSV 16 voice mezője kontextus, nem 16 FX-példány. |
| drive | 16 aktuális lookup Drive, 4×/2× routinggal. |
| filter | 16 PremiumLowPass önmagában, Drive nélkül. |
| adapter | 16 teljes Drive/filter adapter, 100% wet. |
| engine-no-fx / engine-fx | Teljes külön kutatási motor, 16 tartott Poly voice. |
| production-no-fx / production-fx | Teljes shipping motor ugyanazokkal a fixture-beállításokkal; külön költségkontroll. |

Az izolált hangút 16 384 frame-es, körbeforgó 440/660 Hz szinusz/koszinusz
bemenetet használ. A körbehajtás nem garantál fázisfolytonosságot. A teljes
motor saját oszcillátorait futtatja. A szűrőmódok alatt ismételt oscillator/
modulation/FX sorok kontrollok, a mód nem változtatja e külön kernel működését.

**A komponensidők nem összeadható CPU-részarányok.** A külön workloadok,
cache, hívások és energiakiértékelés miatt a motor-FX és motor-no-FX különbsége
sem bizonyított attribúció. A riport audioidőhöz viszonyított százalékot,
három ismétlés mediánját/minimumát/maximumát mutatja. Ezek nem blokk-p99 értékek.
Új optimalizálás csak a teljesmotoros numerikus/minőségi és deadline-próbákkal
együtt fogadható el; a rosszabb tail és célgépes eredményt is meg kell őrizni.

## A korábbi profil javítása

A közös offline Setup régi Delay tone=60 Hz és Reverb decay=40 s,
damping=50 Hz értékei natív egységekkel értendők, ezért limitre futottak.
Az új fixture 2000 Hz / 2 s / 6000 Hz értékeket használ, mint a jelenlegi
deadline-próba. A warmup is 4096 frame-ről 0,25 s-ra változott. Ez minden,
e mérőforrást használó kutatási targetre vonatkozik. A régi mérési adatok
érvényes korábbi workloadok maradnak; nem közvetlen új/előző CPU-arányok.
A shipping hangút és presetek ezen módosítás által nem változnak.

## Ellenőrzés és eredet

A riport elutasítja a hiányzó/dupla/idegen esetet, nem véges vagy nem pozitív
energiát/időt, hibás audioidőarányt, backend/routing rekordot és hiányzó
compiler/contract eredetet. A self-test 360 érvényes esetet és 20 elutasító
kontrollt fed le. A metadata a teljes forráscommitot, forrás- és nyers byte-
hashokat rögzíti; a Windows/macOS CI artifact 90 napig elérhető.

```sh
cmake -S . -B build-components -DBUILD_TESTING=ON -DSAWSTAR_CHECK_DAISYSP=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-components --config Release --target sawstar_premium_lookup_components
python scripts/report-premium-components.py --self-test
```

Az új teljesmotor-célpont fordítását és az első platformadatokat a friss CI
igazolja. A riport helyi ellenőrzése nem teljesmotor- vagy natív CPU-elfogadás.
