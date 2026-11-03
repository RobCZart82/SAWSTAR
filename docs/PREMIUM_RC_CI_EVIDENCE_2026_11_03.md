# Premium RC: távoli bizonyíték és kézikönyvek, 2026-11-03

## Beolvasztás és CI

#133 végső head `5c9861c61e46097284152bd726d0b7f070abae18`:
22 workflow / 67 job sikeres, review-szál nem maradt. Squash merge:
`b9f35574a72758790e23774cd0afa3e1110f1fe8`.
Az új main mind az öt workflow-ja és mind a 12 jobja sikeres:

- [macOS foundation](https://github.com/RobCZart82/SAWSTAR/actions/runs/41587327756)
- [Windows foundation](https://github.com/RobCZart82/SAWSTAR/actions/runs/41587327757)
- [Code quality](https://github.com/RobCZart82/SAWSTAR/actions/runs/41587327785)
- [Workflow-validálás](https://github.com/RobCZart82/SAWSTAR/actions/runs/41587327763)
- [Release-védelem](https://github.com/RobCZart82/SAWSTAR/actions/runs/41587327777):
  a napló szerint `Release preparation skipped: development candidate rc1`.
  Nem történt release-előkészítés, tag vagy publikálás.

## Tényleges RC-csomagok

[RC workflow](https://github.com/RobCZart82/SAWSTAR/actions/runs/41581455905),
pontos forrása az előbbi PR-head, nem a későbbi merge vagy dokumentációs head.
A ZIP-eket letöltöttük; az archívumhash és minden csomagolt payload ellenőrzött.

| Platform | Artifact | Payload | ZIP SHA-256 |
| --- | --- | ---: | --- |
| macOS Universal | 13207490682 | 32 | `1c77f2571889526d891df9c2bd7c75167cd0865868d6921653aa796d00192b00` |
| Windows x64 | 13205458082 | 30 | `7626b139dbc42cce7348dc3f3f62628614c0f36e6a8e585a5abaaa4f12d21e10` |

Mindkét manifest: validator_passed=true, source_dirty=true;
native_host_acceptance=false, factory_listening_acceptance=false,
publication_allowed=false. A három eredeti CSV (Mac arm64, fordított Intel,
Windows x64) külön-külön 144 egyedi cellát tartalmaz: 24 preset × 2 ráta ×
3 gyökérhangszám. Mindhárom legrosszabb limiter előtti csúcsa -6,51292972 dBFS;
a változatlan -6 dBFS kapu sikeres. Rosetta nem natív Intel host-elfogadás.

[Tartós nyers CSV-k, eredeti manifestek és hashjegyzék](measurements/premium-rc-5c9861c-ci-evidence.json).
Az előző, `3e0037dd` Windows-futás exit 127 hibája és üres CSV-artifactja nem
sikeres csomag; a későbbi zöld nem bizonyítja visszamenőleg az eredeti hibakód
pontos okát. A két régi production CPU-negatív eredmény is megmarad.

## A dirty állapot értelmezése és új ellenőrzés

A bootstrap rögzített iPlug2 gitlinket használ, majd két fájlt javít a
repository-ban verziózott patch-szkripttel. Az SDK placeholder README helyére
a rögzített SDK saját README-je kerül. Ezek tényleges, szándékos változások;
a helyi függőségekben mindhárom eltérés látható és ellenőrzött.
A régi CI-manifest nem tartalmaz teljes dirty-diagnosztikát, ezért a régi
csomagokat utólag nem minősítjük tisztának és nem zárunk ki bizonyíték nélkül
minden más korabeli eltérést.

Az új `rc_provenance.py` a csomagolás előtt tilt minden váratlan projekt- vagy
függőségmódosítást. A javított iPlug2 teljes fájljait ideiglenes könyvtárban,
a gitlinkhez tartozó eredeti fájlokból állítja elő ugyanazzal a patch-szkripttel,
és teljes tartalmukat veti össze. Az SDK és négy gyermekrepo pinjét, tiszta
állapotát, az SDK README-jét és a DaisySP pin/tiszta állapotát is ellenőrzi.
A normalizálás kizárólag CRLF/LF; a nyers és normalizált hashok megmaradnak.
Az eredmény külön `source_provenance`; a raw source_dirty mező nem lesz
hamisan false. A factory CSV-k a figyelmen kívül hagyott buildkönyvtárba
kerülnek, így az ellenőrzés nem engedélyez általánosan ismeretlen fájlokat.
Helyben a teljes elvárt függőségpróba sikeres; a még módosított projekt
csomagolását az ellenőrzés helyesen elutasította.

## RC-kézikönyvek

A magyar és angol PDF most 1.1.0-rc1, 2026-11-03 kiadásjelöléssel, 15-15
oldalon. A két első oldal egyértelmű RC-hatókört és telepítést ad, a 15.
fejezet a Premium motort, projektmigrációt, 32 mintás latencyt, factory
ellenőrzést és a nyitott kapukat foglalja össze. A képernyőképek jelölt
1.0.3 felvételek. Mind a 30 oldal renderelt és vizuálisan ellenőrzött;
a generátor és a PDF-ek együtt verziózottak. Ez nem végleges kiadási jóváhagyás.

## Ellenőrzési státusz

Implementált: dokumentáció/PDF és szigorúbb csomag-provenance.
Helyileg: öt csomag/provenance regresszió, elvárt dependency teljesfájl-próba,
módosított projekt negatív kontroll, actionlint, szintaxis, PDF render/text/layout
és hivatkozás/diff ellenőrzés. E lépcső új pontos-head CI-je még szükséges.
A korábbi 22/67 és main 5/12 kizárólag az azonosított régi commitokra érvényes.
Natív és zenei elfogadás: továbbra nyitott; K1 és a rate/buffer döntés szintén.

## #134 review: nem követett iPlug2 fájlok

Az automatizált P2 review felvetését valós, ideiglenes iPlug2 headerrel
reprodukáltuk: az eredeti ellenőrzés a nem követett fájlt elfogadta, mert
a git diff csak a követett változásokat leltározta. A korrekció külön
`git ls-files --others --exclude-standard` leltárt készít, és kizárólag
a később külön ellenőrzött SDK könyvtárat engedi meg. A váratlan header
elutasított; törlése után az elvárt függőségfa sikeres. Új, ideiglenes
Git-repón végrehajtott regresszió őrzi a hibát; öt teszt és a CTest-cél sikeres.
A teszt saját fájlját minden esetben eltávolította. A futó régi CI-t nem
szakítjuk meg: e korrekció annak lezárása után kerül ugyanabba a PR-be.
