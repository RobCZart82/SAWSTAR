# K1 — Az első külső AI-jelentés értékelése

Forrásalap: main `289e6db86b4c80e9f6e2679fdfb5057b171ce149` (#121 után).
Állapot: **hipotézisek pontosítva; igazolt gyökérok/javítás nincs**.
Kapcsolódó [eredeti bizonyíték](ROSETTA_NUMERICAL_INVESTIGATION_2026_10_20.md)
és [belső trace](K1_INTERNAL_TRACE.md).

## MXCSR: hasznos nyom, nem oksági bizonyíték

`0x1fb3` státuszjelzői: IE, DE, UE, PE (bitek 0, 1, 4, 5).
FTZ=0, DAZ=0, RC=00; a hat kivételmaszk (7–12) mind 1.
A kikapcsolt FTZ/DAZ tehát nem hiányzó kivételmaszkolás.
A státuszjelzők törlésig megmaradnak: nem azonosítanak konkrét mintát vagy motort.
Elsődleges forrás: [Intel SDM Volume 1, 10.2.3](https://cdrdv2-public.intel.com/671436/253665-sdm-vol-1.pdf).

Új, ellenőrzött körülmény: a hibás CSV-ben **13 teljes eset** előzi meg a
filter=2 / voice=0 / FX=1 esetet. A rögzített `fb3114e` fixture nem törölte
esetenként a státuszjelzőket. Az MXCSR-t a hibánál a szövegformázás előtt olvasta.
Nem tudjuk, mikor és melyik motorban billentek be a jelzők; származhatnak korábbi
esetből is. A DE/UE így nem bizonyítja, hogy a 197. minta eltérését denormális
művelet okozta. A hibás output-érték és a delta önmagában sem szubnormális float.

## A kódra vonatkozó kérdések

- **SIMD van:** a két kalibrált motor közös `RateScaledLookupPremiumDrive`
  útja `VectorFir=true`; x86_64-en a `FirLanes4` SSE2 `_mm_mul_pd`/`_mm_add_pd`
  műveleteket használ, explicit nullázott akkumulátorokkal. Nem kizárólag a
  Candidate optimalizációja, és nem ARM64 Reference ↔ x86 Candidate összevetés:
  mindkét motor ugyanabban az x86_64 folyamatban fut Rosetta alatt.
- **HP12 és Triangle:** a filter állapotai inicializáltak és Clear() alatt
  nullázódnak. A Triangle saját `triangleHistory_` tömbje Init() alatt nullázott;
  a DaisySP triangle ágának `last_out_` mezőjét nem használja. Vannak osztások és
  tömb-/FIR-indexelések, de konkrét inicializálatlan függést e vizsgálat nem talált.
  Ez nem teljes memóriahelyességi bizonyítás és nem zárja ki más komponens hibáját.
- A fenti PremiumLowPass/PremiumDrive/FirLanes4/RateScaledPremiumDrive/SevenSaw.cpp
  fájlok szövege a megőrzött `fb3114e` headen és a helyi jelenlegi változatban egyezett.
- Az eredeti Universal executable `otool -arch x86_64 -tvV` listájában a keresett
  FMA és x87 load/store/aritmetikai mnemonicokból nulla találat volt. Ez a saját
  executable ellenőrzése; dinamikus rendszerkönyvtárakat és Rosetta fordított
  kódját nem minősíti. Az FMA/x87/context-switch magyarázat továbbra spekuláció.

## Átvett vizsgálati ötletek, javításként el nem fogadott állítások

1. **FP-környezet-profil:** először passzívan, formázás előtt rögzített MXCSR
   mindkét motor előtt/után, reset/setup és esethatárok körül. Külön diagnosztikai
   kontrollban csak a státuszjelzők törlése lokalizálhatja keletkezésüket, az eredeti
   kontrollbitek megőrzésével. Az x87 állapot külön mérhető, ha szükséges.
2. **FTZ/DAZ OFF/ON kontroll:** előre kijelölt, mindkét motorra azonos policy,
   külön folyamatok; az eredeti OFF alappróba megőrzésével. Egy ON/PASS nem
   bizonyít okot és nem indokol automatikus production-policy-váltást.
3. **Memóriainicializálás:** célzott állapotaudit és megfelelő sanitizer/
   stack-inicializálási kontroll. A heap-scribble korábbi PASS nem zárta ki a
   stackhibát, de az állapotok vak, renderenkénti nullázása hibás javítás lenne.
4. **Fordítási kontroll:** csak támogatott, ellenőrzött toolchain-flagekkel,
   mindkét motorra azonos feltételekkel, külön binárisként; nem csak a Candidate
   aritmetikájának átállításával. A bitazonossági küszöb változatlan marad.

A jelentés kategorikus „legvalószínűbb ok” állítását nem fogadjuk el bizonyítottnak.
A hasznos rész a mérhető hipotézis: mikor változik az FP-környezet és melyik
állapot/művelet tér el először. Nincs e jelentés alapján elvégzett DSP-, FTZ/DAZ-
vagy fordítási policy-módosítás. K1 továbbra nyitott.
