# SAWSTAR — fejlesztési terv az 1.0.4 után

Frissítve: 2026-10-02. Kiinduló main: `8e4c18e1de08f6b3018cb859c90b8186ffabddce`.
A main Windows, macOS és Code quality workflow-ja sikeres. A #36 PR lezárta
az ARP transport-stop utáni editor-tulajdonlás, a VST3 host MIDI-kontrollerpontok
és a jövőbeli editor-offsetek overflow-helyreállításának javítását.

## Aktuális munkacsomag: presetimport

Az importot a GUI közvetlenül hívja. A korábbi implementáció minden új fájlnál
újra listázta a könyvtárat, a duplikációkereséshez újraolvasta a preseteket,
és a mentés előtt ismét elvégezte a névütközés-ellenőrzést.

A javítás a közös `PresetMutationLock` alatt, egyetlen importhívás idejére
név- és tartalomindexet készít. A tartalomindex fokozatosan épül fel, és korai
egyezésnél megáll; a következő bemenet a rendezett keresést onnan folytatja.
Minden meglévő preset legfeljebb egyszer kerül beolvasásra egy batchben.
Hibás vagy üres bemenet önmagában nem olvastatja be a könyvtárat.
Sikeres import után az új név és hang is bekerül az indexbe. Sikertelen mentés
esetén a foglalás visszavonódik. A közönséges Save útvonal névellenőrzése megmarad.

Megőrzött követelmények:

- Unicode/case-insensitive névütközés; meglévő fájl nem íródik felül.
- Pontos, kanonikus Snapshot-egyezés; közel azonos hangok nem olvadnak össze.
- A korábbi könyvtárban az első rendezett egyező név kerül a jelentésbe.
- Hibás meglévő preset nem akadályozhatja a jó fájlok importját, de a neve foglalt.
- Azonos hangok külön néven csak a kifejezett másolatimporttal kerülnek be.
- A pluginpéldányok és folyamatok műveleteit a meglévő közös zár sorosítja.
- A fájl létrehozása továbbra is kizáró: külsőleg létrehozott cél sem írható felül.

Az index az import idején készített könyvtárpillanatképre épül. A zárat nem
használó külső program ne módosítsa közben a könyvtárat; egy következő importhívás
már új indexet készít. A GUI-művelet továbbra is szinkron, és a zárra várakozás
sem szűnik meg. A háttérmunka külön következő feladat.

### Helyi mérés

Windows, MSVC 19.44, Release; ugyanaz a mérőprogram és adatkészlet. A mérés az
`ImportPresets` hívást tartalmazza, a fixture-előkészítést nem. A mentés/flush
ideje benne van. Egy-egy helyi futás, nem platformfüggetlen teljesítményígéret.

| Meglévő + új preset | Korábbi main | Indexelt import |
|---|---:|---:|
| 100 + 100 | 2634,85 ms | 185,77 ms |
| 200 + 200 | 9859,56 ms | 377,46 ms |
| 400 + 400 | 37825,70 ms | 788,17 ms |

A manuális mérőprogram: `sawstar_preset_import_benchmark`. Egy parancssori
argumentuma a generált mérési alkönyvtárak szülőkönyvtára. A cél nem CTest:
közös CI-gépek sebességére nem állítunk merev időkorlátot. A funkcionális
`preset_import_index` teszt vegyes batch, pontos duplikáció, eltérő kis/nagybetűs
nevek, sérült fájlok, másolatimport és sikertelen mentés utáni folytatás esetét fedi.

## Presetbetöltési koherencia — követelmény a javítás előtt

A GUI presetalkalmazás jelenleg 93 külön paramétergesztust küld. A forrásból
látható a mezőnkénti átadás; ez önmagában nem bizonyít új hallható regressziót.
Teljes hangállapot átadását csak a következő szerződéssel szabad bevezetni:

1. Rögzíteni kell az editor presetváltás és közben érkező host-automatizálás
   sorrendjét: melyik esemény melyik blokkban érvényesül, és ki nyer ütközéskor.
2. A teljes motorparaméterkészlet és az ARP/editor reset ugyanahhoz az elfogadott
   állapothoz tartozzon. A host paraméterértesítése, Undo és GUI-visszajelzés megmarad.
3. A host-state visszatöltés, SerializeState, gyors egymás utáni presetek, zero-frame
   hívások és reset közbeni átadás is kapjon külön tesztet.
4. Az audio callback nem várhat GUI-mutexre, nem allokálhat és nem használhat
   korlátlan újrapróbálkozást. Egy audio-oldali snapshot önmagában nem rendezi
   a többi paraméteríróval való együttélést.
5. Determinisztikus interleaving-harness igazolja a részleges állapotot, majd a
   kiválasztott javítást. A meglévő állapotformátum és parameter ID-k megmaradnak.

## Következő sorrend

1. Importindex regressziói, platform-CI, sanitizer és mérés.
2. Import/mentés GUI-késésének profilozása; háttérmunka csak megmaradó késés esetén,
   megszakítással és az editor élettartamát tiszteletben tartó eredményátadással.
3. Presetkoherencia reprodukció és a fenti sorrendi szerződés.
4. Hard link nélküli átnevezés, kizáró célfoglalással és hibainjektált helyreállítással.
5. Backup/kedvencek helyreállítása, írási és zárolási hibák, többpéldányos próbák.
6. A hagyományos paraméterautomatizálás blokkfüggésének mérése. A #36 MIDI-pontjavítása
   nem vezette be a hagyományos paraméterek sample-accurate automatizálását.
7. Wrapper/motor profilozás; hangkarakter-változtatás csak külön mérés és döntés után.

Natív REAPER-mátrix: host/editor azonos hang, overflow, késői release, ARP HOLD,
sustain, stop/start, editor close, bypass, deaktiválás, reset, presetváltás és
offline tail. A korábbi 1.0.4 általános felhasználói elfogadás nem azonos az új
build minden kombinációjának külön dokumentált elfogadásával.

A következő release verzióját és RC-jét a tényleges kiadási kör rögzítse.
Minden PR pontos headje legyen zöld; publikálás előtt a végleges commit,
csomagok, natív elfogadás és dokumentáció kapui is teljesüljenek.
Filterkarakter, 32 voice és a lezárt click/pop kutatás külön munkacsomag marad.
