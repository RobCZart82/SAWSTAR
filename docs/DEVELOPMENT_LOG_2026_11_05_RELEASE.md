# Fejlesztési napló — 1.2.0 előkészítés, 2026-11-05

Robert új kiadási célja 1.2.0. Friss GitHub-lekérés: main ec110a3 (#137),
4 workflow / 11 job sikeres, nincs nyitott PR. A #136 működési visszajelzését
és #137 ARP bypass javítását az új kiadás tartalmazza.

Az előkészítő ág az új mainből indul. 1.2.0 rc1 metaadat, config/plist,
README, kiadási leírás és aktuális terv készül; a régi dátumozott
1.1.0 RC bizonyítékok változatlan történeti források maradnak.
Az index/terv elavult állítását a hat új exportált hang elfogadásáról javítjuk.
A két kézikönyv az új candidate verzióból generálódik.

[Kiadási végrehajtási sorrend](RELEASE_1.2.0_CHECKLIST.md).
DSP, presetérték, paraméter-ID, FP-mód, mérési küszöb és alapmotor nem változik.
Implementáció, helyi ellenőrzés, távoli CI és natív elfogadás külön státusz;
a tényleges helyi/CI eredmény az előkészítő PR-ben kerül véglegesítésre.
Publikálás nem történt; a pontos RC elfogadása és végső kiadási döntés nyitott.

Helyi ellenőrzés: metadata sync --check sikeres; release_guard 21/21,
Premium RC packaging/provenance 5/5, Preview packaging 10/10, build display
metadata mind a hét release/dev/RC esete sikeres. Mindkét PDF 15 oldalas,
1.2.0 jelölésű; 30 oldal renderelve, kontaktlap vizuálisan és minden szó
oldalhatáron belül ellenőrizve. git diff --check sikeres. Távoli CI még függő.
