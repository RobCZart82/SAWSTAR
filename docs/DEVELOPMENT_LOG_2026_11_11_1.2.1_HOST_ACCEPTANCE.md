# SAWSTAR 1.2.1-rc1 — gépi elfogadási visszajelzés — 2026-11-11

## Aktuális forrás és CI

- Ellenőrzött main commit: `406e76dbe0f763e9fe3d5cfaa1150748f5939d52`.
- A négy kötelező főági workflow és mind a 11 job sikeres.
- Az első CI-próbálkozás négy workflow-szintű GitHub belső szerverhibát
  jelzett, jobok nélkül. A következő próbálkozás teljesen zöld lett; a sikeres
  próbálkozás runjai: lint `42673617265`, Windows foundation `42673617273`,
  macOS foundation `42673617393`, Code quality `42673617687`.
- A kiadás-előkészítő workflow az `rc1` státusz miatt kihagyta a publikálást.
  Nincs 1.2.1 stable tag vagy publikált release.

## A pontos main commit csomagjai

| Csomag | Actions artifact | Külső artifact SHA-256 |
| --- | --- | --- |
| macOS Universal | [13700168199](https://github.com/RobCZart82/SAWSTAR/actions/runs/42673617393/artifacts/13700168199) | `bf6f9fc5174e0dfdda9da8d6dd98405cb2805d055f0f81fcad73ace762d41b00` |
| Windows x64 | [13701321074](https://github.com/RobCZart82/SAWSTAR/actions/runs/42673617273/artifacts/13701321074) | `0ad6c116f8ea1eefed8705abe5953c73fa5d3c338c06ad3027185c50ef2f7291` |
| Windows ARM64 | [13699454696](https://github.com/RobCZart82/SAWSTAR/actions/runs/42673617273/artifacts/13699454696) | `c86949f2c335b33ee767e58e2917f38ac700614dbcef696b326fe6bd2e44233d` |

Ezek a `1.2.1-rc1` tesztcsomagok az adott main commitból, nem stabil kiadási
fájlok. Az artifact-ellenőrzés CI-állapotát a 11 sikeres job fedi; a hostpróba
alábbi eredménye felhasználói visszajelzés.

## Felhasználói hostpróba

RobCZart82 2026-11-11-én jelezte, hogy az SAWSTAR 1.2.1 Windows és macOS alatt is
jól működik; elvégezte a hangzáspróbát, a betöltést és a mentést is, és nem
talált hibát. Ez a jelölt általános működésére vonatkozó pozitív felhasználói
visszajelzés. Nem adott meg operációs rendszer- vagy REAPER-verziót, Windows
architektúrát, konkrét mérést, illetve teljes CPU/dropout vagy lifecycle
tesztmátrixot; ezeket nem állítjuk ellenőrzöttnek. Különösen nem következik
belőle Windows ARM64 natív hostelfogadás.

## Nyitva maradó kiadási kapuk

- `1.2.1-rc1` marad a verzió; a stabil metadata, tag és publikálás nem történt
  meg. Ehhez RobCZart82 külön, kifejezett 1.2.1 kiadási/publikálási döntése kell.
- K1 numerikus eltérése továbbra is ismert, megőrzött és megoldatlan. A
  sikeres hostpróba és a korábbi kontrollok nem bizonyítják a K1 gyökérokát
  vagy javítását; a végleges kiadási megjegyzésekben pontosan fel kell tüntetni.
- Részletes CPU/dropout lefedettség, Windows ARM64 natív hostpróba és a
  támogatott minimum operációs rendszerek teljes minősítése nincs ezzel a
  visszajelzéssel lezárva.

Nincs itt dokumentálva új kódváltozás, hangmotor-javítás vagy K1-elfogadás.
