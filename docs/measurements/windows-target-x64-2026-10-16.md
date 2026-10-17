# SAWSTAR Windows/M1 célgépes mérés – kiértékelés

Ellenőrzés: 2026-10-17. Mindkét teljes archívum nyers ellenőrzése és az azonos forrású összevetés exit 0 eredménnyel lefutott. Új mérés nem történt.

## Eredmény

| Mérőszám | Windows i7-6600U | Mac M1 |
| --- | --- | --- |
| Saw p50 változás, 1. kör | −4,84…−2,78% | −7,51…−4,13% |
| Saw p50 változás, 2. kör | −4,75…−2,95% | −7,33…−3,97% |
| Legrosszabb jelölt p99 változás, 1. kör | +5,44% | +7,54% |
| Legrosszabb jelölt p99 változás, 2. kör | +7,76% | +5,01% |

A változások minden gépen a saját referenciaútjához képest értendők. Ez a Premium lookup teljesmotor-referencia és a Saw-dispatch jelölt vizsgálata, nem Classic/Premium összehasonlítás és nem REAPER CPU%. A p50/p99 arányok a rögzített páros eljárás eredményei; körök összevonása nem történt.

Windowson a 96 és 192 kHz-es jelölt-összevetések minden cellájában mind a referencia, mind a jelölt 4096/4096 blokkja túllépte az időkeretet (körönként 48/72 cella). 48 kHz-en is vannak túllépések. Az első kör legrosszabb p99 cellája: statikus Triangle, 48 kHz/64, referencia 70, jelölt 139 túllépés. A második köré statikus Sine, 192 kHz/32, mindkét út 4096 túllépés. Az eredmény nem jelent mért DAW-hangkiesést, azt natív hostban külön kell megvizsgálni.

A referenciaismétlés p99 változása Windowson az 1. körben −13,83…+15,18%, a 2. körben −6,09…+4,63%. Ez változékonyságot mutat; zajlevonás vagy a jelölt romlásainak zajként való elkönyvelése nem történt.

## Eredet és megőrzés

- Forrás mindkét gépen: `2f401939db95c4da51e80d041e8911d0258f3b39`; a 20 forráshash egyezik.
- DaisySP: `599511b740f8f3a9b8db72a0642aa45b8a23c3a3`.
- Windows: Intel Core i7-6600U, 2 mag/4 szál, 8 395 759 616 byte RAM; Windows 10 Pro 10.0.19045; MSVC 19.44.35229; Release x64, Python 3.15.0; kiegyensúlyozott energiaséma. AC-állapotot a gépadatfájl nem rögzített; a háttérterhelés nem igazolt.
- Windows futás: 2026-10-16 21:46:06–23:32:53 UTC (Budapest: október 16. 23:46:06 – október 17. 01:32:53); 1 óra 46 perc 48 másodperc.
- Windowson 32/32 grid, 288 páros cella, 2 359 296 nyers blokk. Mindkét kör és minden rosszabb eredmény megmaradt.
- Windows ZIP: `SAWSTAR-Windows-target-2026-10-16.zip`, 14 207 491 byte; SHA-256: `abf2bf2f55f80349661e4dcea21c86cfac4dc9dc367b4688e342f0a96d0656d1`.
- Windows manifest SHA-256: `97aae25880010fa32510676f6b80c0d745c613407f5541da9b431786dd38f47b`.
- Mac ZIP SHA-256: `98aaf722af0f28da906a4c11d3f9014b584134ff5373b798314765c4f199f215`.
- A Windows ZIP bejegyzésneveiben levő Windows könyvtárelválasztókat csak a Linuxos kibontás útvonalain értelmeztük; fájltartalom, raw hash és eredeti ZIP nem módosult.
- A feltöltött review.md mellett függetlenül újrafutott a pinned reviewer --verify-raw, majd a pinned compare-dispatch-engine-targets.py mindkét nyers archívumon. Hash, nyers blokksorrend, percentilisek, páros eredmények és túllépésszámok ellenőrizve.

## Következő kapuk

A Saw p50 javulása mindkét platformon megismétlődik. A p99-romlások és túllépések miatt CPU-elfogadás és production-promóció továbbra sincs. A natív REAPER/dropout vizsgálat, a támogatott ráta/puffer szabály és a production-integráció nyitott. A GUI-fix natív elfogadása külön feladat. A hangerőkorrekció elfogadott; production Classic, Premium Preview külön opt-in, alapból OFF.

`background_load_controlled=false`, `native_host_acceptance=false`, `production_promotion_allowed=false`.

## Géppel feldolgozható bizonyíték

[Windows eredetjegyzék és mind a 288 cella](windows-target-x64-2026-10-16.json).
[Mac eredetjegyzék és mind a 288 cella](mac-target-arm64-2026-10-16.json).
A nyers Windows ZIP Robert helyi példányában és a feltöltött privát másolatban megőrizve; nem nyilvános release-artifact. A nagy raw CSV-k nem kerülnek Gitbe.
