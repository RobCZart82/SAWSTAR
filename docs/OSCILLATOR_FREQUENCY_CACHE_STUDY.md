# Oszcillátor-frekvencia cache — kutatási jelölt

A #86 beolvadt a `645cab8` main-ba, mind a 35 exact-head ellenőrzés
sikeres. A FIR-kibontás nem hozott igazolt teljesmotoros CPU-nyereséget.
A komponensprofil szerint az OSC1/OSC2 oszcillátorbank költsége is érdemi,
különösen magas rátán. Ez a következő, izolált optimalizálási hipotézis.

## Változás és referencia

A külön `experimental_oscillator::CachedSevenSaw` a main `SevenSaw`
algoritmusát használja. A végső `held-saw-tuning-v2` csak a SAW-frekvencia
frissítését hagyja el változatlan pitch és detune-ráták mellett. Mozgó
pitch vagy változó detune esetén az eredeti setter fut, az alternatív
waveformok setterei minden aktív mintán változatlanul futnak. A rögzített DaisySP
commit `599511b740f8f3a9b8db72a0642aa45b8a23c3a3`: a setter frekvenciát
és fázislépést ír, nem indítja újra a fázist. A cache csak az ismételt,
azonos hangolás újraszámítását kerüli el; nincs közelítő összehasonlítás
vagy vezérlésritkítás. A pitch és az egy mintával korábbi detune-ráta
byte-összehasonlítása signed zero esetén is pontos. Init, SetFreq és
Snap külön invalidálja a hangolást; ezért a közvetlen Snap nem maradhat
észrevétlen, akkor sem, ha az utána következő simítás már nem mozdul.

A detune, pitch, Nyquist-clamp, waveform súlyok, háromszög-integrátor,
pan és normalizálás változatlan. Inaktív hullámformák nem kezdenek
folyamatosan futni; a saját fáziselőzményük megmarad. Másoláskor az előző
pitch, dirty flag és oszcillátorállapot együtt másolódik. A korábbi 28
floatos lane-cache helyett egy float és egy bool a tartós plusz állapot.

Az eredeti `src/dsp/SevenSaw.*` és a production Synth változatlan.
Referencia a main `645cab8` SevenSaw-forrása; a riport mindkét saját
implementáció és a rögzített DaisySP-fájlok byte-hash-eit rögzíti.
Ez még nem teljesmotoros integráció vagy realtime elfogadás.

## Numerikus kapu

A `oscillator_cache_bit_identity` 72 jelenetet, összesen 596 736 sztereó
frame-et vizsgál: hat rátát (8–384 kHz), négy induló hullámformát és
három gerjesztést. A statikus kontroll mellett hangmagasság/detune/mix/
waveform váltás, Nyquist-clamp, nulla és signed zero, subnormál frekvencia,
NaN/Inf vezérlők utáni helyreállás, Snap, reinit és élő állapot másolása
szerepel. Az output float-bytejai egyeznek; a jelenetek nem lehetnek
csendesek. A negatív kontroll szándékos hangeltérést utasít el.

Helyi MSVC 19.44 x64 Release, a rögzített valódi DaisySP-forrásokkal:
a numerikus kapu sikeres. Négy Python riportteszt sikeres, beleértve
az aszimmetrikus párok mediánarányát, hiányzó/duplikált grid, rossz
sorrend/bank/frame, nem véges mérés és eltérő páros energia elutasítását.
Windows/macOS Debug/Release és sanitizer kvalifikáció friss CI-kapu.

## Páros költségmérés

A benchmark 32 SevenSaw példányt használ, azaz 16 voice OSC1+OSC2
bankját modellezi. Nem tartalmaz envelope-ot, SUB/noise-t, filtert, FX-et
vagy host callbacket. Három ráta × négy waveform × statikus/modulált
pitch × négy váltakozó pár × két út: 192 nyers sor és 24 összesítés.

Mindkét út frissen indul; Init és 0,25 s warmup a timeren kívül van.
8192 frame-et mérünk, bankfeldolgozás és energiadiagnosztika belül;
modulált esetben audio-rate szinuszos pitch-setter is belül. Az ismétlési
sorrend páronként váltakozik. A riport a study/reference időarányok
mediánját számolja; nem a két független medián hányadosát.

Az `oscillator-cache.yml` a numerikus kapuk után méri a két platformot.
CSV, compiler-leírás, kontraktusnapló, JSON és Markdown artifact készül;
a JSON a CI-naplóban is megjelenik. Nincs CPU pass/fail küszöb.
Jobb izolált idő nem bizonyít teljesmotoros nyereséget; a modulált
kontrollban regresszió is lehetséges a cache-ellenőrzés többletköltsége miatt.

## Első, teljes waveform-cache helyi eredmény — történeti v1

MSVC 19.44 x64 Release, a végleges byte-összehasonlító cache-sel.
Ez a `a44dfcb` v1 forrás eredménye, nem a szűkített v2 mérési eredménye.
Az alábbi feltáró összevonás négy hullámforma × négy pár időarányának
mediánja; a 24 külön waveform/workload cella és 192 nyers sor is megmarad.

| Ráta | Statikus pitch | Modulált pitch |
| --- | ---: | ---: |
| 48 kHz | 1.013395 | 1.012916 |
| 96 kHz | 1.025308 | 0.994427 |
| 192 kHz | 1.009815 | 0.994977 |

Nincs minden hullámformára érvényes helyi CPU-nyereség; a statikus
összevonások kb. 1–2,5% lassulást mutatnak. A kisebb modulált különbségből
nem következik stabil gyorsulás. A többletágazás vagy kódelrendezés szerepe hipotézis, nem
profilozással bizonyított ok. Ezeket az eredményeket nem cseréljük
kedvezőbb kiválasztott mintákra, és nem indokolnak production aktiválást.

A külön SAW (waveform=0) statikus cellák ugyanebben a teljes gridben
0.860448 / 0.863677 / 0.857075 arányt adtak 48/96/192 kHz-en: kb.
14% kisebb izolált bankidő. Modulált SAW: 0.992635 / 0.986464 / 0.980758.
Ezzel szemben több más hullámforma lassult, például a 192 kHz-es modulált
triangle 1.119223 és sine 1.099469 arányt adott. Ezért a pozitív SAW
eredmény célzott következő hipotézis, nem a négy mód közös elfogadása:
előbb platformismétlés, majd szükség esetén szűkített cache-scope,
waveformváltási kontroll és teljesmotoros SAW-próba következzen.

[Nyers helyi mérés](../experiments/oscillator/measurements/2026-10-08-cache-local-windows.csv)
és [eredetjegyzék](../experiments/oscillator/measurements/2026-10-08-cache-local-windows-provenance.json):
forráscommit, raw/source/compiler/executable hash-ek, helyi buildleírás
és a 24 cella mind a négy páros aránya. A mérés izolált helyi bankpróba,
nem azonos workload a korábbi teljesmotor-profillal.

## V1 platformeredmények és a scope szűkítése

Mért forrás: `4ef343ac54b92ded1aac11a51a824f71e0625dbe`, még a teljes
waveform-cache. A két sikeres job gépi riportja megőrzött:
[Windows](../experiments/oscillator/measurements/2026-10-08-cache-v1-ci-windows.json),
[macOS](../experiments/oscillator/measurements/2026-10-08-cache-v1-ci-macos.json).
Ezek naplóból kiemelt, 24 cellás összesítések a négy nyers páros aránnyal;
a 192 soros nyers CSV-t nem töltöttük le és nem hash-eltük újra helyben.

| Platform | SAW 48 kHz statikus / modulált | SAW 96 kHz statikus / modulált | SAW 192 kHz statikus / modulált |
| --- | --- | --- | --- |
| Windows | 1.000529 / 1.019698 | 0.913434 / 1.039793 | 0.930488 / 1.055580 |
| macOS arm64 | 0.963924 / 1.086571 | 0.731199 / 1.110797 | 0.818055 / 0.999967 |

Windows alatt statikus SAW 96/192 kHz-en gyorsult, de a modulált SAW
2–5,6% lassulást mutatott. Más waveformok többsége is lassult. macOS-en
a párok szórása nagy (pl. 96 kHz statikus SAW: 0.572041–1.210578);
ezért a medián önmagában nem általános platform-nyereség bizonyítéka.

A v2 ezért elhagyja az alternatív waveformok cache-ét és a per-lane
frekvencia-cache-t. A már elvégzett detune-simítás változását ellenőrzi,
valamint egyszer, bankonként az előző pitch értékét. Változó pitch esetén
a detune-összehasonlítás rövidzáras, a frekvenciasetter eredeti útja fut.
Változatlan hangolásnál a SAW frekvenciaszorzás/clamp/setter is elmarad.
Ez új hipotézis: a v1 időarányai nem vihetők át rá.

Az új CSV kötelező compiled `variant=held-saw-tuning-v2` mezőt tartalmaz;
a reporter rossz/hiányzó variánst elutasít és a metadata-ban rögzíti.
A helyi 72 jelenet / 596 736 frame újra bitazonos; mind a négy riportteszt
sikeres a variánsazonosítás negatív kontrolljával is. A v2 friss platformos
és teljesmotoros CPU-kapu marad; production aktiválás nincs.

### V2 első helyi mérés — a modulációs regresszió megmaradt

Mért forrás: `55c42f68b40cd2485fcf6abfd35fed7848fce1c6`, MSVC x64 Release.
A 192 soros grid sikeresen validált, a páros energiák pontosan egyeznek.

| Ráta | SAW statikus | SAW modulált |
| --- | ---: | ---: |
| 48 kHz | 0.972938 | 1.135337 |
| 96 kHz | 0.963177 | 1.121958 |
| 192 kHz | 0.960207 | 1.118610 |

A statikus SAW 3–4% javulási jele mellett a modulált SAW 11,9–13,5%
lassulása megmaradt. Más waveformok modulált cellái is kb. 1–3,5%
lassulást mutatnak. A v1 és v2 helyi futása külön kampány, ezért ezek
arányaiból nem állítunk kontrollált v1/v2 gyorsulást. Ez negatív v2
kvalifikációs jel: jelen állás szerint teljesmotoros/production aktiválás
nem indokolt. A cache-gate többletköltsége feltételezés, további
profilozás nélkül nem bizonyított magyarázat.

[V2 nyers adatok](../experiments/oscillator/measurements/2026-10-08-cache-v2-local-windows.csv)
és [eredetjegyzék](../experiments/oscillator/measurements/2026-10-08-cache-v2-local-windows-provenance.json)
megőrzi mind a 24 cellát, a párokat és a raw/source/compiler/executable
hash-eket. A friss Windows/macOS CI a funkcionális és platformos
vizsgálatot végzi; zöld job nem írja felül a lassulási megfigyelést.

Következő döntés: platformeredmények értékelése, majd megfelelő eredmény
esetén külön teljesmotoros lookup-kontroll és modulált deadline mérés.
Production aktiválás, magasrátás policy és natív REAPER továbbra is
külön, nyitott kapuk; nem következnek a zöld izolált tesztből.

## V2 platformeredmények és döntés — 2026-10-08

A `674f66bdb7ccb321f9e61dc49562812ec58b972a` PR-head minden ellenőrzése
sikeres. Ez funkcionális és build-kapu, nem CPU-elfogadás.
A két izolált platformmérés naplóból kiemelt, változatlan schema-1 JSON-ja:
[Windows](../experiments/oscillator/measurements/2026-10-08-cache-v2-ci-windows.json),
[macOS](../experiments/oscillator/measurements/2026-10-08-cache-v2-ci-macos.json).
A nyers CSV-ket nem töltöttük le újra; az összes páros arány és a CI által
közölt raw/source/compiler hash megmarad az összesítésekben.

| Platform | SAW 48 kHz statikus / modulált | SAW 96 kHz statikus / modulált | SAW 192 kHz statikus / modulált |
| --- | --- | --- | --- |
| Windows | 0.931259 / 1.045345 | 0.938845 / 1.050838 | 0.936462 / 1.056853 |
| macOS arm64 | 0.833299 / 0.892990 | 0.784961 / 0.887875 | 0.927653 / 0.966164 |

Windows alatt a statikus SAW 6–7% javulási jele mellett a modulált SAW
4,5–5,7% lassult. A 192 kHz-es modulált SAW mind a négy párban lassabb.
macOS-en több cella nagy szórású: a 192 kHz statikus SAW páros arányai
0.518–1.693 közé esnek. Az eltérő platform- és helyi eredményekből
nem következik igazolt, általános teljesmotoros nyereség.

Döntés: a held-saw-tuning-v2 production bevezetését elhalasztjuk;
a kísérleti referencia, tesztek és negatív eredmények megőrzendők.
Nincs teljesmotoros cache-integráció vagy új magasrátás CPU-policy.
A következő hipotézis a mintán belül ismételt frekvenciaszámítás
megosztása legyen, cache-állapot és változásfigyelő ágak nélkül.
Előbb ugyanilyen bitazonossági/platformos kontroll, majd igazolt
izolált eredménynél teljesmotoros modulált deadline mérés szükséges.

A reporter schema-2 kimenete külön `qualification` mezőben rögzíti a
medián szerint lassabb cellákat és az összes párban lassabb cellákat.
Ezek leíró megfigyelések, nem statisztikai szignifikancia vagy CPU-küszöb.
Kedvező időarányok esetén is `cpu_acceptance=not-established` és
`production_promotion_allowed=false`: az izolált mérés nem jogosít
production bevezetésre. A Markdown/CI összesítő ezt kifejezetten jelzi.
Két új negatív kontroll különíti el az összes párban és csak a mediánban
látható lassulást, és ellenőrzi, hogy a kedvező grid sem engedélyez aktiválást.
