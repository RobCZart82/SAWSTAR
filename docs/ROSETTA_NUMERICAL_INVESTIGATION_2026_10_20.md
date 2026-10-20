# #116 — megismétlődött Rosetta-biteltérés vizsgálata, 2026-10-20

**Állapot: ismét bizonyított CI-eltérés, az ok és a javítás továbbra nyitott.**
Kapcsolódó [K1 munkacsomag](BUGFIX_DEVELOPMENT_PLAN_2026_10_20.md#4-k1--a-116-rosetta-biteltérése).
Ez a `fb3114eb829442223083e103929a15ad25d3efeb` head új negatív bizonyítéka;
nem az eredeti `05b730d` napló helyettesítése vagy annak hibaméretének visszaszámítása.

## Megőrzött forrás és első eltérés

- [macOS Preview workflow 39250177170](https://github.com/RobCZart82/SAWSTAR/actions/runs/39250177170):
  Universal/reference sikeres; [Universal/candidate job 118051765492](https://github.com/RobCZart82/SAWSTAR/actions/runs/39250177170/job/118051765492) hibás.
- A build elkészült. A numerikus kvalifikáció x86_64 Triangle cold-run-0 folyamata
  exit 1; a későbbi három Triangle-folyamat exit 0, de a deklarált sikeres baseline
  hiánya miatt helyesen nem válthatják ki az eredeti hibát.
- Hibás artifact ID **12186151118**, ZIP SHA-256
  `247fe73cec4300ff828cd529993fdcb9572def3edb9388c9f0918165814fda6c`.
- A ZIP hashát, a manifestben megadott executable/cache és mind a 14 log hashát
  helyileg ellenőriztük. Az eredeti hibás `x86_64-waveform-2.txt` SHA-256:
  `d893ac670bfe10981e3ae55c34593c433363ec7f38ecfd34cd875908c24f2c09`.

Az első diagnosztizált eltérés:

| Mező | Érték |
| --- | --- |
| Waveform / rate | Triangle / 48000 Hz |
| Filter / voice / FX | HP12 / Poly / bekapcsolva |
| Szakasz / minta / mérési pont | main / 197 (nullától indexelt) / output, bal csatorna |
| Candidate | `0.0008501199190504849`, bits `0x3a5eda95` |
| Reference | `0.00085012189811095595`, bits `0x3a5edab7` |
| Candidate − Reference | `-1.9790604710578918e-09` |
| FP-diagnosztika | `fp_rounding=0`, `mxcsr=0x1fb3` |

A teljes eredeti hibasor:

```text
Dispatch engine bit identity waveform=2 rate=48000 filter_mode=2 voice_mode=0 fx=1 phase=main frame=197 point=output channel=left candidate_bits=0x3a5eda95 reference_bits=0x3a5edab7 candidate=0.0008501199190504849 reference=0.00085012189811095595 delta=-1.9790604710578918e-09 fp_rounding=0 mxcsr=0x1fb3
```

Ez az **első eltérés**, nem a teljes render maximális hibája: a teszt itt megszakad.
Az `output` ellenőrzés megelőzi a `pre_fx` ellenőrzést, ezért a sor nem bizonyítja,
hogy maga az FX okozza a hibát. A delta kicsisége önmagában nem oldja fel a
bitazonossági követelményt és nem bizonyítja a teljes eltérés hallhatatlanságát.

## Sikeres kontroll és helyi próbák

A sikeres Reference-job numerikus artifactja **12185332231**, külső ZIP SHA-256
`cdcbb17305ddec26ae15174062fc19627e068240df35b37d08836b7355877a72`.
Ennek ZIP- és loghashai is ellenőrizve. A hibás és sikeres jobban ugyanaz az
Universal tesztexecutable szerepel, SHA-256:
`3228d4d226e7e5f6b213773831406b326a74a260340ed738e012efa1009cccd0`.
Mindkét job executable-je a két motort együtt hasonlítja össze; a job szerepneve
nem jelenti, hogy csak az egyik motort teszteli.

A megőrzött bináris egyetlen helyi macOS arm64/Rosetta Triangle-futtatása
120/120 esetet teljesített. Egy külön, előre kijelölt memória-inicializálási
hipotézispróba `MallocPreScribble=1 MallocScribble=1` környezettel szintén
120/120 esetet teljesített. Mindkettő teljes CSV-je egyezett a sikeres CI
Triangle CSV-jével; helyi logjuk SHA-256-ja azonos:
`05a07296c6c434c9d2a86b482974203af744acff4dd914258ffb0b77ee0e19d4`.
Ez nem sikerig ismétlés, és nem zárja ki általánosan az inicializálási hibát.
Nem az eredeti CI-gépen/OS-verzión végzett próba; a futásfüggő jelenség oka
ebből még nem nevezhető Rosetta-, compiler- vagy könyvtárhibának.

## Kapcsolat az új audit találataival

| Találat | Összevetés e konkrét esettel |
| --- | --- |
| B1 mixer/CC1/pressure szubnormális lecsengés | A 197. mintáig a mixer céljai 70/50/20/5, a CC1/pressure kezdő és célértéke nulla; az első nem nulla vezérlő csak az 512. mintánál érkezik. A B1 reprodukciójának nullára lecsengő állapotai itt nem állnak elő. Más upstream FP-jelenséget ez nem zár ki. |
| B2 SUB 20 kHz-plafon | MIDI 48–63, SUB octave −1, a bend kezdetben középen; a cél messze a plafon alatt van. Nem magyarázza ezt az eltérést. |
| R1 preset-lock | Ez közvetlen, egyszálú Synth-API teszt, presetimport/GUI-művelet nélkül; ez az útvonal nem vesz részt benne. |
| Dokumentáció/CMake/dátum | Nem a numerikus mintaeltérés javításai; a build sikeres volt. |

## Következő célzott diagnosztika és javítási döntés

1. A következő diagnosztikai mód őrizze meg az eredeti gridet, eseménysorrendet,
   mintaszámot, bitpontos/negatív kontrollt és az összes eredményt.
2. **Még az exception előtt** rögzítse ugyanannak a hibás mintának mindkét
   outputját és pre-FX értékét. Korlátos, csak eltérésnél kiírt jelút-trace kell:
   oscillator bankok/sub/noise, voice-filter, voice-sum/pre-FX, chorus/delay/reverb,
   gain/width/guard. A két motor első eltérő belső pontját keressük.
3. A releváns kezdeti DSP-állapotok, szűrő/FIR együtthatók és FP-policy rögzítése
   különítse el az inicializálás, fordított aritmetika és futási környezet hipotéziseit.
   Egy állapothash csak a definiált mezőket tartalmazza, paddinget/pointercímet ne.
4. A megőrzött bináris/hash és a hibás log maradjon meg. A diagnosztikai fordítás
   új bináris, ezért annak sikere nem bizonyítja a régi bináris javulását.
5. Csak az első belső eltéréshez igazolt ok után válasszunk javítást és célzott
   regressziót. Nincs bizonyítékot helyettesítő epsilon-emelés, általános FP-flag
   váltás vagy a B1/B2 javítás automatikus K1-megoldásként kezelése.

**Eredmény:** lényegesen pontosabb kiváltó eset és megőrzött hibás bináris áll
rendelkezésre, de igazolt kódjavítás még nincs. #116 numerikus kapuja nyitott.
