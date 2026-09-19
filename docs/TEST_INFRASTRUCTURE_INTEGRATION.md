# Friss main integráció és benchmark-adatellenőrzés

2026-09-19. Helyi, ki nem adott fejlesztés.

A fejlesztési ág megtartotta a karaktermegőrző alapot és a 57bb335-ben
rögzített Output/Boost, detune és Delay javításokat. Az origin/main
(e55423b6) frissítéseit Git merge-rel integráltuk. A mainből öt fájl
változásai érkeztek: quality.yml, CMakeLists.txt, benchmark-audio.py,
performance.cpp és state_roundtrip.cpp. Nem került vissza RC7 kutatási DSP.

A benchmark parser négy lebegőpontos metrikája most kötelezően véges.
NaN, +Inf, -Inf és túlcsorduló 1e999 minden metrikában hibát okoz, még a
medián és küszöbérték-összehasonlítás előtt. A main() belépési pont lehetővé
teszi a valódi parser importálását baseline-fordítás indítása nélkül.
A CPU/audio határértékek és a normál futtatás sorrendje nem változott.
AUDIO_QUALITY.md most a tényleges összehasonlítási szabályt írja le.

Ellenőrzés:
- A parser regresszió a végesérték-védelem előtt mind a 16 hibás numerikus
  kombinációt átengedte (16 sikertelen részteszt); utána mindet elutasítja.
- Három Python teszteset sikeres: jó adat és medián, nem véges értékek,
  duplikált/üres eredmény elutasítása. CTest-regisztráció elkészült.
- Az integráció után kilenc releváns C++ motor/DSP teszt újrafordítva és
  futtatva sikeres (a DETUNE_DELAY_FIXES.md listája).
- Ebben a körben nincs teljes CTest-, sanitizer-, platform- vagy valós
  CPU-benchmark futás. Helyben CMake nem található; a motoros ellenőrzések
  közvetlen clang fordítással futottak. Teljes CI és VST3-host ellenőrzés
  szükséges a későbbi beolvasztás előtt.

A jelentések összevont, státuszokkal ellátott terve a munkakönyvtár
outputs/SAWSTAR-development-planning/AUDIT-NOTE-REVIEW-2026-09-19.md
fájljában van. Következő fő feladat: tényleges plugin MIDI-/paraméterút,
majd az ezen keresztüli reset/preset/automatizálási esetek.

GitHub ruleset, kiadott csomag, telepített VST3 és GUI nem változott.
