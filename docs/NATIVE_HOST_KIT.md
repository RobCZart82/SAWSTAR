# Natív mérési csomag előkészítése — macOS

Állapot: **implementált előkészítő eszköz, nincs natív mérési eredmény**.
A #127-ben összesített K1-kontrollok után a következő prioritás a külön
Reference/Candidate pár tényleges host-/CPU-vizsgálata. K1 oka továbbra ismeretlen.

## Letöltés és előellenőrzés

1. Válassz egy pontos checkout SHA-jú, sikeres `macOS Premium Preview` futást.
   Mindkét szerep build-, páros numerikus-, saját smoke- és validator-jobja
   legyen sikeres. Rögzítsd a futás URL-jét és a teljes 40 karakteres SHA-t.
2. Töltsd le a `SAWSTAR-Premium-Preview-macOS-Universal` és
   `SAWSTAR-Premium-Candidate-macOS-Universal` artifactokat ugyanerről a headről.
3. A GitHub artifact külső ZIP-jéből külön könyvtárba bontsd ki a belső
   plugin-csomag ZIP-et és a mellette levő `.zip.sha256` fájlt.
   Az eszköz bemenete a **belső csomag**, nem az artifact külső ZIP-je.
4. Válaszd ki előre a P1 hangszínt és annak filtermódját. Ne a futási eredmény
   alapján válassz kedvezőbb módot. Az alábbi példa HP12-je csak példa.

```sh
python3 scripts/prepare-native-host-kit.py \
  --reference /path/to/reference-package.zip \
  --candidate /path/to/candidate-package.zip \
  --expected-source FULL_40_CHARACTER_CHECKOUT_SHA \
  --fx-mode HP12 \
  --output /path/to/new-measurement-directory
```

A script kizárólag olvas és új adatlapkönyvtárat ír. Nem bont ki plugint,
nem telepít, nem indít REAPER-t, nem módosít projektet és nem játszik le hangot.
Meglévő kimeneti könyvtárat — üreset sem — nem ír felül.

Ellenőrzések: külső SHA-256 és fájlnév; manifest teljes fájljegyzéke és minden
tag hash-a; azonos várt forráscommit; role/product/oscillator/ID/plist;
közös engine/latency/level policy és kísérleti státusz; executable hash és
jogosultság; Universal arm64/x86_64 fat-tábla és CPU-fejlécek határai.
Duplikált/case-colliding útvonal, symlink, nem reguláris vagy manifestből
hiányzó tag elutasított. A vizsgálati méretkorlát tagonként 128 MiB,
összesen 512 MiB és 10 000 tag.

Ez **konzisztenciaellenőrzés**: az azonos csomagban tárolt hash nem hitelesítő
aláírás. A CPU-fejléc vizsgálata nem teljes Mach-O-validátor és nem bizonyítja,
hogy a host tényleg natívan fut. A script nem kérdezi le a GitHub CI-t,
nem igazolja a codesign/notarizálás állapotát és nem ad telepítési elfogadást.
A workflow sikerét és eredetét külön kell ellenőrizni és feljegyezni.

## Kimenet és végrehajtás

| Fájl | Tartalom |
| --- | --- |
| `provenance.json` | Mindkét teljes manifest, ZIP-/manifest-/binárishash, forrás és előre kiválasztott P1-mód; `not_run`, elfogadási flagjei false. |
| `measurements.csv` | 28 C/V/G/F eset × 4 ablak = 112 sor; 10 s bemelegítés + 60 s megfigyelés, Reference/Candidate/Candidate/Reference sorrend. Mérési adatok üresek. |
| `environment.json` | Kitöltendő környezet, tényleges hostarchitektúra, eszköz/driver, preset/projekt/MIDI/automatizálás hash, mérőmódszer és elfogadási scope. |
| `README.md` | Végrehajtási korlátok és rövid ellenőrzőlista. |

A CSV első négy esete a C /128 AUTO előszűrés: 16 ablak, nettó 18 perc
40 másodperc. A teljes terv nettó 130 perc 40 másodperc/gép, setup és tail nélkül.
/128 megelőzi /64-et; minden eset megtartja a kétirányú AB/BA kontrollt.
Az előszűrés sorai a teljes mátrix részei, nem további ismétlések.

Telepítési smoke előtt a REAPER legyen bezárva; kövesd a
[két külön plugin telepítési útját](NATIVE_PREVIEW_PAIR_QUALIFICATION.md).
Utána a **telepített** executable hashait is hasonlítsd össze a provenance-szal.
Apple Siliconon natív arm64 REAPER, Intelen natív x86_64 REAPER kell;
az aktuális hostfolyamat architektúráját is rögzítsd. Rosetta-kontroll külön próba.

Mérés előtt töltsd ki a környezetadatokat, mentsd az azonos P0/P1 presetet,
MIDI-t és automatizálást. A script nem talál ki helyetted paramétereket vagy
hangszínt. P0 FX nélkül, P1 előre rögzített FX-szel; egyszerre egy hangszer online,
a másik offline. Headroom és a már elfogadott 0,44 dB policy változatlan.

Minden megfigyelést őrizz meg. Elérhetetlen számláló `N/A`, nem nulla;
nem futott sor `not_run`, nem PASS. Szemmel olvasott CPU-tartományból nincs p99,
FX CPU-ból nincs RT-tartalék. Hibás ablakot ne írj felül: külön ismétlésazonosító
és ok kell. A tail/idle, stressz, state/GUI és magas ráta/kis puffer külön adatsor.

A [natív adatlap](NATIVE_REAPER_CPU_ACCEPTANCE.md) és a
[release terv 4–7. pontja](NEXT_RELEASE_DEVELOPMENT_PLAN.md) marad a mérvadó.
Robert konkrét build/gép/tartomány szerinti elfogadása, a Windows x64 célgépes
mérés és a végső production RC újraellenőrzése külön feladat. Ez az első
eszközverzió csak macOS csomagot fogad; Windows-csomaghoz nem ad hamis sikert.

## Fejlesztői ellenőrzés

```sh
python3 tests/premium_preview_package.py
python3 tests/native_host_kit.py
```

A fixture a valódi csomagolóval készített szintetikus Universal csomagot használ;
nem valódi plugin/validator/REAPER-próba. A külön `Native host kit preflight`
workflow ugyanazokat a pozitív és elutasítási kontrollokat futtatja Linuxon.
A natív pár útmutatójának módosítása a két platform Preview-csomagoló CI-jét
is elindítja, így friss, azonos forrású Reference/Candidate artifactok készülhetnek.
