# Dispatch CPU-kampány: Windows/macOS forrásbyte-egyezés

## Javítás lezárása — 2026-10-17-i állapot

A sorvégjavítás beolvadt #102-vel; az új teljes kampány `38735317728`
mindkét gyűjtője és Linuxos nyers összevetése sikeres volt.
A #102/#103/#104/#106/#105 sorozat beolvadt. Az alábbi „Következő kapu”
a javítás előtti történeti követelmény, mára teljesült; nem kér új kampányt.
A régi hibás kampány és hashai megmaradnak, nincs utólagos normalizálás.
Aktuális CPU-/natív feladat: [release terv](NEXT_RELEASE_DEVELOPMENT_PLAN.md).


## Feltárt hiba — 2026-10-15

A #102 `29bfe4a2a2587d3461c70ba3c5aaa4f4a238dd36` headjén a
`38699679057` teljes kampány mindkét gyűjtője sikeres lett, de az összevetés
helyesen elutasította az eltérő `source_file_sha256` értékeket.
A két platform nyers adatai megmaradnak; ez nem CPU-regresszió, gyorsulás
vagy elfogadás megállapítása.

A letöltött ZIP-ek SHA-256 értéke egyezik az Actions artifact digestjével:

| Platform | Artifact ID | ZIP SHA-256 |
| --- | --- | --- |
| Windows | 11964756961 | fb23c65aed4815041674c8699af4f86758fa430ba2a6fa1e9c04eedddbf2a65f |
| macOS | 11963011517 | 887011518b8ef5ea8881acdec1068732b51656bcb4fb8cc51188381bf03f1cab |

Mind a 20 mért forrásfájl esetében a Git blob eredeti LF byte-jainak
SHA-256 értéke egyezik a macOS archívumával. Ugyanezen blob kizárólagos
LF → CRLF átalakítása pontosan a Windows archívumában szereplő hash-t adja.
A Git blob SHA-1 azonosítóit is ellenőriztük.
[Útvonalankénti bizonyíték](measurements/dispatch-checkout-bytes-2026-10-15.json).

A hiba oka így a checkout sorvégkonverziója. Az összevető szigorú nyers
hash-egyezési feltétele továbbra is érvényes; a régi archívumok utólag nem
kapnak átírt vagy normalizált hash-eket.

## Javítás és kontrollok

- A `.gitattributes` a C++ fejléceket/forrásokat és a `CMakeLists.txt`
  fájlt LF checkouttal rögzíti. A korábbi archivált mérési bizonyítékok
  későbbi, konkrét `-text` szabályai megmaradnak, így CRLF byte-jaik sem változnak.
- A `tests/dispatch_checkout_bytes.py` valós ideiglenes Git repositoryban,
  `core.autocrlf=true` és `core.eol=crlf` beállítással ellenőrzi a változatlan
  LF forrásbyte-okat és a változatlan CRLF archív bizonyítékot.
- A negatív kontroll forrás-szabályok nélkül reprodukálja az eltérő
  checkout-byte-okat és hash-eket. A teszt nem módosít felhasználói Git-beállítást.
- A két kontroll a teljes kampányban még a függőség/build/időmérés előtt fut.
- A hibás összevetés jobb diagnosztikájáért nem lazítunk mérési,
  p99/deadline-, numerikus vagy forrásazonossági küszöböt.

## Következő kapu

A javított checkouttal **új teljes Windows/macOS kampány** szükséges,
mindkét körrel, külön referenciaismétléssel/jelölttel és minden nyers griddel.
A régi mérés nem minősül automatikusan elfogadottnak a sorvég-ok feltárásától.
A javított PR-head összes ellenőrzése és az új összevetés sikere után
következhet #102 beolvasztása, a main CI ellenőrzése, majd #103 → #104 → #105.
A külön hangerőkorrekciós Preview letölthetősége nem kerül e sor elé.

Ez mérési reprodukálhatósági javítás; a szintézisalgoritmus és a Premium
hangkaraktere nem változik. A dedikált Windows/M1, natív host és CPU-elfogadás
továbbra is külön kapu.

