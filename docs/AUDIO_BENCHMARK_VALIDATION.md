# Production audio-benchmark CSV-szerződés

A `scripts/benchmark-audio.py` a rögzített production baseline és a jelenlegi
motor nyolc, váltakozó sorrendű mérési párját hasonlítja össze. Ez külön
ellenőrzés a kísérleti prémium filter célgépes kampányától.

A korábbi parser elfogadta a nem üres, véges értékű CSV-részhalmazokat.
Ha mindkét útból ugyanaz a jelenet hiányzott, a körök és a két motor
kulcshalmazának összevetése egyezést mutatott, így hiányos mérés is
sikeresnek tűnhetett. Az új ellenőrzés minden warmup és időzített futásnál
pontosan hat jelenetet követel: `dry_fx` és `full_fx`, mindkettő 1/8/16 voice.
Hiányzó vagy ismeretlen jelenet, duplikáció, hibás fejléc/sorszélesség,
érvénytelen szám, nem véges érték vagy nem pozitív CPU-idő hibát jelent.
A teljes CSV sorainak sorrendje tetszőleges; a metrikák előjele nem korlátozott
a pozitív CPU-idő kivételével (negatív dBFS/DC érvényes).

Nem változik a DSP, baseline, workload, warmup, ismétlésszám vagy a küszöb:
a CPU-regresszió akkor hibás, ha egyszerre több mint +25% és +1,0 realtime
százalékpont. A ±1 dB szint- és ±0,01 DC-korlát is megmarad. Nincs automatikus
újrapróbálás vagy kedvezőbb mérési kör kiválasztása.

A `benchmark_report` CTest a hiányos warmup/időzített CSV-ket és parserhibákat
szintetikus adatokkal ellenőrzi. Ez adatérvényességi javítás, nem CPU-gyorsulás,
natív REAPER-elfogadás vagy a prémium filter CPU-kapujának lezárása.
