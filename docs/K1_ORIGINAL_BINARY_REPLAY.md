# K1 — Az eredeti bináris kontrollált CI-visszajátszása

Állapot: a `5f859452` CI-kampánya sikeres, K1 nem reprodukált; **Nem K1-javítás és nem production-elfogadás.**
Alap: `9383d46d45bd1569616f0f5452137e2ef2e55646`, a #123 beolvasztása után.
A #123 összesen 19 workflow-ja / 59 jobja sikeres volt; a megelőző main négy
workflow-ja is sikeres. Az új main saját ellenőrzése külön kapu.

## Legfrissebb kontrollok

A #126-ban az eredeti executable natív Intel macOS-en és újabb
Apple Silicon/Rosetta kampányban is hibamentesen futott. A kalibrált
sanitizer-gridek is sikeresek; minden csomag helyileg újraellenőrzött.
[Összesített eredmény és értelmezés](K1_NATIVE_SANITIZER_RESULTS_2026_10_20.md).
Az alábbi #124-kampány történeti bizonyítéka változatlanul megmarad.

## Elvégzett #124-kampány — 2026-10-20

A [39463485911-es workflow](https://github.com/RobCZart82/SAWSTAR/actions/runs/39463485911)
a `5f859452cfdf48d27757b40d18692c8f804a001d` headen sikeres.
Mindkét eredeti archívum ellenőrzése és 14–14 kijelölt folyamata lefutott,
minden exit 0, a teljes CSV- és Triangle-ujjlenyomat-ellenőrzésekkel együtt.
Ez **28 folyamat / 3360 esetmegfigyelés**, K1 most nem reprodukált.

Mindkét runner macOS 14.8.9 / macos-14-arm64, image 20260831.0302.1.
Az új artifactok:

| Eredet | Artifact ID | Új ZIP SHA256 |
| --- | --- | --- |
| Candidate | 12284685877 | `17b8690facbfbd6b8f9c6ec358d77486a62d3d7436d26b89a6f7c4633f28e90a` |
| Reference | 12283578783 | `ca3bff35d4c193ef39714f14dad9db1698261aa9761af6bb265bfdbcd8a1d952` |

[Összesítő és mind a 28 megőrzött folyamat eredménye](measurements/k1-original-replay-ci-2026-10-20.json).
A fenti összesítés a befejezett GitHub-jobnaplókra és artifact-metaadatokra
támaszkodik. Az archívumok/bináris/cache/logok ellenőrzését a verziózott
CI-runner végezte; ebben a munkalépésben nem történt külön helyi ZIP-újraaudit.

A #124 mind az öt PR-workflow-ja / 14 jobja sikeres; a megelőző main négy
workflow-ja / 11 jobja is sikeres. #124 beolvadt:
`16f024e3bad4a84cd21f22362694f711e18fdf07`.
Az új main és a következő PR saját CI-je külön kapu.
Ezt követte a [setup alatti IE műveletszintű lokalizálása](K1_SETUP_FP_LOCALIZATION.md).
Ez a sikeres replay nem írja felül az eredeti negatív bizonyítékot.

## Miért ez a következő lépés?

A [passzív FP-mérés](K1_FP_OBSERVATION.md) új binárisában a korábbi
`mxcsr=0x1fb3` állapot audiohiba nélkül is előállt. Az új próbák nem magyarázzák
az [eredeti eltérést](ROSETTA_NUMERICAL_INVESTIGATION_2026_10_20.md), ezért most
a ténylegesen hibázott executable-t vizsgáljuk, újrafordítás nélkül.

Az eredeti forrás és qualifier ellenőrzése egy fontos részletet pontosított:
a hibás cold-run-0 parancsa **`--diagnostic-waveform 2`**, nem `--waveform 2`.
Az előbbi a bitazonossági ellenőrzés mellett FNV-jelujjlenyomatokat is számol.
A sima kapcsoló ugyanazon executable más futási útja; nem kezelhető teljesen
azonos kontrollként. A #123 FP-kampánya új binárist használt és más célt mért.

Az eredeti hiba előtt a Triangle-folyamatban 13 teljes grid-eset futott le.
Nem egyetlen HP12/Poly/FX ON esetet indítunk külön. A teljes eredeti,
120 esetes Triangle-grid rögzített sorrendje megmarad. Ezenfelül az eredeti
qualifier 14 folyamatának sorrendje is megmarad.

## Megőrzött, előre rögzített bizonyíték

| Forrás | Artifact ID | ZIP SHA256 |
| --- | --- | --- |
| Korábban hibás Candidate-job | 12186151118 | `247fe73cec4300ff828cd529993fdcb9572def3edb9388c9f0918165814fda6c` |
| Korábban sikeres Reference-job | 12185332231 | `cdcbb17305ddec26ae15174062fc19627e068240df35b37d08836b7355877a72` |

Mindkettő a 39250177170-es workflow / `fb3114eb829442223083e103929a15ad25d3efeb`
forráshoz tartozik. Mindkettőben ugyanaz a Universal tesztexecutable várható:
`3228d4d226e7e5f6b213773831406b326a74a260340ed738e012efa1009cccd0`.
A szerepnév az archívum eredetét jelöli; mindkét executable mindkét motort
egy folyamaton belül hasonlítja össze.

A runner mindkét eredetet külön macOS 14 / Apple Silicon jobban vizsgálja.
A ZIP-hash, a manifest forrása/állapota/executable-hasha, az executable és
cache tényleges hashai, a 14 eredeti log hashai, valamint az eredeti
folyamatsorrend ellenőrzése megelőzi az első végrehajtást.
A Candidate cold-run-0 hibás logjának ismert hashát külön is ellenőrizzük.
Nem történik rebuild, újraszignózás, FP-módváltás vagy státuszbit-törlés.
A letöltéshez használt GitHub-token nincs átadva a tesztexecutable-nek.

## Rögzített kampány és döntés

A [workflow](../.github/workflows/k1-original-replay.yml) először Linuxon
futtatja az evidenciaparser elutasítási kontrolljait; csak utána indul a két
független macOS-job. A [runner script](../scripts/replay-k1-original.py)
eredetenként az alábbi változatlan tervet hajtja végre:

1. Waveform 0, 1, 2, 3 sorrendben mindegyikhez arm64, majd x86_64 folyamat.
2. Három további Triangle-pár: arm64, majd x86_64, cold-run-1/2/3.
3. A Triangle mindig `--diagnostic-waveform 2`; a többi waveform sima
   `--waveform N`. Eredetenként 14 folyamat, összesen 28;
   teljes siker esetén 3360 grid-esetmegfigyelés.
4. Folyamatonként 120 másodperces határ, minden kimenet megőrzött.
   Numerikus hiba után is végigmegy a kijelölt terv; nincs sikerig ismétlés.
   Hiányzó/lejárt artifact, hibás hash vagy eredetellenőrzés esetén
   a végrehajtás előtt megáll, nem épít helyettesítő executable-t.

A sikeres folyamatok teljes, sorrendhelyes 120 esetes CSV-je, nulla
hibametrikái és a Triangle 13 oszlopos jelujjlenyomat-táblája külön ellenőrzött.
A két motor ujjlenyomatainak egyezése és a későbbi Triangle-folyamatoknak a
saját architektúrájuk előre kijelölt cold-run-0 alapjához való egyezése kötelező.
Hibás alapot későbbi siker nem helyettesíthet. A táblák SHA256-ja FNV-táblára
vonatkozik, nem nyers audio SHA256-ra.

A manifest megőrzi az eredeti és a kampányrunner forrásazonosítóját, az
OS/runner image adatokat, a bináris/cache/loghashokat, parancsokat, kilépéseket
és validációs hibákat. Az eredeti ZIP, manifest, executable, cache és logok is
az új eredménycsomagba kerülnek. Létező kimenetkönyvtár nem írható felül.

## Értelmezési korlátok és következő döntés

A „cold” új folyamatot jelent, nem garantáltan üres Rosetta-transzlációs cache-t.
A macOS-verziót és runner image-et rögzítjük; a felhős gép, ütemezés, rendszer-
és könyvtárkörnyezet teljes azonossága nem garantált. Az artifact végrehajtási
útvonala, a hívó Python-verzió és az eredeti plugin-buildet megelőző munkák is
eltérhetnek. Ez hashazonos executable-kontroll, nem az eredeti gép teljes másolata.

- Numerikus hiba esetén az új napló önálló negatív bizonyíték. Az első eltérés
  konfigurációját/mintáját/bitmintáit az eredetivel össze kell vetni; más
  eltérés nem nevezhető automatikusan ugyanannak a K1-esetnek.
- Timeout/letöltési/validációs hiba nem automatikusan numerikus reprodukció.
- Minden folyamat sikere csak a rögzített kampány eredménye; az eredeti
  negatív bizonyítékot nem törli és K1-et nem zárja le.
- Új audioeltérés nélkül a setup alatti IE pontos Reset/setter/MIDI
  lokalizálása külön, új instrumentált binárisban folytatható. A régi
  executable nem kap utólagos belső trace-t.

A [parser kontrolljai](../tests/k1_original_replay.py) hiányzó/duplikált/átrendezett
esetet, hamis hibametrikát, eltérő vagy hibás ujjlenyomatot, hibás fejlécet és
ismeretlen archívumot utasítanak el. Ezek nem az eredeti DSP-hiba reprodukciói.
