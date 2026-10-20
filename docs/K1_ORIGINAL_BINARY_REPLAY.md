# K1 — Az eredeti bináris kontrollált CI-visszajátszása

Állapot: új diagnosztikai kampány implementálva; eredménye a saját pontos-head
CI-jétől függ. **Nem K1-javítás és nem production-elfogadás.**
Alap: `9383d46d45bd1569616f0f5452137e2ef2e55646`, a #123 beolvasztása után.
A #123 összesen 19 workflow-ja / 59 jobja sikeres volt; a megelőző main négy
workflow-ja is sikeres. Az új main saját ellenőrzése külön kapu.

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
