# K1 — Natív Intel- és sanitizer-eredmények, 2026-10-20

**Állapot: a kijelölt kampányok sikeresek; K1 nem reprodukált, oka és javítása
továbbra ismeretlen.** Nem minősítjük bizonyított SAWSTAR-, teszt-, Rosetta-
vagy hardverhibának. A megőrzött eredeti negatív bizonyíték változatlan.

Vizsgált pontos PR-head: `7480154d7c6f4596925992df1a7bd61940d6468d`.
[#126](https://github.com/RobCZart82/SAWSTAR/pull/126) beolvadt:
`e2b27054c9777c43ce989dc1cf7e858917c0edb0`.
Mind a hét PR-workflow / 22 job sikeres; a megelőző main négy workflow-ja is
sikeres volt. Az új main és e dokumentáció saját CI-je külön kapu.

## Mért eredmények

| Kampány | Környezet és bináris | Eredmény |
| --- | --- | --- |
| Eredeti bináris natív Intel-kontrollja | macOS 15.7.9; Intel Core i7-8700B; a megőrzött `3228d4d2…` Universal executable x86_64 szelete | Két archívumeredet, 7–7 folyamat, **1680 eset**, nulla numerikus eltérés |
| Eredeti bináris újabb Apple Silicon-kontrollja | macOS 14.8.9; ugyanazon executable arm64 és x86_64/Rosetta szelete | Két archívumeredet, 14–14 folyamat, **3360 eset**, nulla numerikus eltérés |
| Kalibrált Preview ASan/UBSan | Eredeti `fb3114eb…` és aktuális `7480154d…` forrásból új build; natív ARM64/macOS 14.8.9 és Intel/macOS 15.7.9 | Négy konfiguráció, 4–4 teljes waveform-grid, **1920 eset**, motorbeli sanitizer-jelentés és numerikus eltérés nélkül |

Az esetek kampányonként megismételt megfigyelések, nem ennyi különböző
tesztszituáció. A sanitizer build más kódgenerálású executable, nem az
eredeti hibás bináris hashazonos újrafuttatása.

A natív Intel-futtató metaadata `GenuineIntel`, `X64`,
`Intel(R) Core(TM) i7-8700B CPU @ 3.20GHz`. A translation probe üres
eredménye megmaradt, nem fordítjuk hamis Rosetta-jelzéssé. Ez GitHub által
hosztolt natív Intel macOS-kontroll, nem REAPER/audiointerface-elfogadás.

## Az x86_64 Triangle-ujjlenyomat egyezése

A megőrzött executable nyolc natív Intel és nyolc Rosetta Triangle-folyamatában
ugyanaz a teljes diagnosztikai ujjlenyomat-tábla SHA256 érték adódott:

`998b01c921eb167ffbeba5f60a8245d902b949a158e20fc3e3132e2d61ac37c1`

A táblák az egyes esetek output/pre-FX Reference/Candidate FNV-1a
jelujjlenyomatait tartalmazzák. Ez **FNV-tábla SHA256**, nem nyers audio SHA256.
A bitazonossági guard minden folyamaton belül külön is érvényesült.

Az ARM64 szelet nyolc Triangle-folyamatának saját táblája:
`ca1b8aa3bdb0ea55f18b5e3e8a3821171030eeb305235f4e282ed89b91d424a2`.
Az architektúrák között nem volt előírva bitazonosság; külön fordított
gépi kódról van szó. A két motor ARM64-en is egyezett a saját folyamatában.

## Sanitizer-kontrollok és lefedettség

Mind a négy konfigurációban a külön, szándékosan hibás kontroll-executable:

- ASan heap-buffer-overflow jelentést adott és hibával kilépett;
- UBSan signed-integer-overflow jelentést adott és hibával kilépett.

Ez összesen nyolc **elvárt negatív kontrolljelentés**. Nem motorhibák és nem
K1-reprodukciók. A valódi motorgridekben nem volt sanitizer-jelentés.

A fordítási adatbázis valamennyi egységének ASan/UBSan/O1 kapcsolói,
a kalibrált Reference/Candidate forrásegységek jelenléte, az executable
runtime-szimbólumai és a sanitizer linkkapcsoló is ellenőrzött. Megmaradtak
a dSYM-ek és a linkparancs. A macOS kampány `detect_leaks=0` beállítású:
memóriaszivárgás-elfogadást nem állítunk. A régi Linux quality jobok változatlanok.

Az első próba két teszt-infrastruktúra-hibája javítva és dokumentálva:
az Intel gép hiányzó translation OID-jének túl szigorú kezelése, valamint
az első ASan-kontroll nem jelzett, távolabbi túlindexelése. Ezek a motor
futtatása előtt buktatták a kampányt. A végleges head kontrolljai sikeresek;
egyik korábbi sikertelen kontroll sem tekinthető K1-találatnak.

## Helyi nyersadat-újraellenőrzés

A GitHub-zöld állapot után mind a **nyolc ZIP** letöltve és helyileg ellenőrizve:

- ZIP SHA256 egyezés a GitHub artifact-metaadattal;
- executable/cache/kontrollprogram/fordítási adatbázis és loghashok;
- a megtartott eredeti belső ZIP-ek és a hibás/sikeres eredeti logok;
- a teljes CSV-gridek elemszáma, sorrendje, nulla hibametrikái és ujjlenyomatai;
- sanitizer-kontrollok konkrét jelentései és a natív környezet metaadatai.

A **122 ellenőrzött logfájlpéldány** részben ugyanazon eredeti naplók
külön archívumokban megőrzött másolata. Nem 122 független futást jelent.
A pontos-head 19 replay/CSV/host és öt sanitizer-validator teszt helyileg is
sikeres. A nyers gridek validálásához a verziózott validátorokat, a ZIP-ek
összekapcsolt ellenőrzéséhez külön helyi auditot használtunk; nem állítunk
függetlenül újraírt DSP-referenciát.

[Teljes ellenőrzött manifestek, hashok és összesítők](measurements/k1-native-sanitizer-ci-2026-10-20.json).

## Megőrzött workflow-k és artifactok

| Workflow | Run ID | Artifact ID-k |
| --- | --- | --- |
| [Natív Intel-kontroll](https://github.com/RobCZart82/SAWSTAR/actions/runs/39482712051) | 39482712051 | 12293295054, 12291908749 |
| [Apple Silicon / Rosetta replay](https://github.com/RobCZart82/SAWSTAR/actions/runs/39482711939) | 39482711939 | 12293065802, 12292786431 |
| [Kalibrált ASan/UBSan](https://github.com/RobCZart82/SAWSTAR/actions/runs/39482712146) | 39482712146 | 12292578165, 12292578077, 12292197218, 12291858323 |

A ZIP-hashok és minden artifact saját környezet-/forrás-/logmanifestje a
fenti géppel olvasható összesítőben szerepel. A [módszertan és korlátok](K1_NATIVE_AND_SANITIZER_CONTROLS.md)
az eredménytől külön dokumentumban maradnak.

## Következő fejlesztési döntés

**Ezt az előre kijelölt diagnosztikai kört befejezettnek tekintjük, K1 ügyét
nyitva tartjuk.** A sikeres próbák nem bizonyítják a külső környezet hibáját,
és nem bizonyítják, hogy a ritka belső/tesztállapot-hiba lehetetlen.

1. Megőrizzük az eredeti hibás binárist, naplót és a mostani kontrollokat.
2. Újabb Rosetta-ismétlés vagy mélyebb Reset/IE-instrumentálás csak új negatív
   esethez vagy konkrét, ellenőrizhető hipotézishez indokolt.
3. A végrehajtási prioritás a meglévő natív Reference/Candidate csomagok
   és a [REAPER CPU-/dropout-/host-jegyzőkönyv](NATIVE_REAPER_CPU_ACCEPTANCE.md).
   Az ARM64 és Intel végrehajtás eredményeit külön követjük.
4. Új numerikus vagy sanitizer-találatnál először az első eltérés/stack trace
   lokalizálása következik, majd igazolt okhoz célzott javítás és regresszió.

A numerikus küszöbök, K1 eredeti bizonyítéka és az M1/production/RC/publikálási
elfogadási kapuk változatlanok. E dokumentum nem engedélyez automatikus
production-promóciót, release-t vagy a K1-blokkoló figyelmen kívül hagyását.
