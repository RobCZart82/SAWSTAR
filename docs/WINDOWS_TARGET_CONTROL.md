# Windows célgépes kontroll a lezárt M1-mérés mellé

Állapot: **a 2026-10-16–17-i Windows-gyűjtés, nyers ellenőrzés és azonos forrású Mac-összevetés kész; CPU-/natív elfogadás nincs**.
[Eredmény, korlátok és eredetjegyzék](measurements/windows-target-x64-2026-10-16.md).
Az alábbi futtatási útmutató reprodukciós leírás: a már lezárt kampányt nem indítjuk újra. A hosted Windows CI
nem helyettesíti ezt a saját célgépes kontrollt. A Mac-gyűjtést nem ismételjük.

## Rögzített forrás és előfeltételek

A [2026-10-16-i Mac-mérés](measurements/mac-target-arm64-2026-10-16.md)
forrása `2f401939db95c4da51e80d041e8911d0258f3b39`, DaisySP-je
`599511b740f8f3a9b8db72a0642aa45b8a23c3a3`. **A Windows gyűjtőt ebből a
commitból futtasd, ne a későbbi mainből.** A két archívum összevetője azonos
commitot és forrásbyte-okat követel; egy későbbi dokumentációs commit is más SHA.

- Saját Windows x64 gép; az M1-gyűjtéshez illeszkedő két teljes kör.
- 64 bites Python 3, Git, CMake/CTest a PATH-on; Visual Studio 2022 C++ Build
  Tools és Windows SDK. Az alábbi parancsok a VS 2022 x64 generátort használják.
- Hálózat a repository és a pinned DaisySP egyszeri letöltéséhez.
- AC táp, dokumentált energiabeállítás; a DAW lejátszása és egyéb terhelő
  feladatok álljanak. A háttérterhelés hiányát ettől nem tekintjük igazoltnak.
- A teljes gyűjtés több tíz perc vagy több is lehet. Nem telepítünk/állítunk át
  automatikusan eszközt, energiagazdálkodást, prioritást vagy affinitást.

## Mérés előtti, csak olvasó ellenőrzés

A **friss fejlesztési checkout** gyökerében nyiss PowerShellt; innen érhető el
az új előellenőrző és a Mac-összefoglaló. A rögzített mérési checkout külön lesz.
Ez a lépés csak letölt és ellenőriz, időmérést nem indít:

```powershell
$ErrorActionPreference = 'Stop'
$toolRoot = (Get-Location).Path
$controlRoot = Join-Path (Split-Path $toolRoot -Parent) 'sawstar-windows-control-2026-10-16'
if (Test-Path $controlRoot) { throw 'A kontrollmappa már létezik: ne írj felül és ne indíts újra meglévő kampányt.' }
New-Item -ItemType Directory -Path $controlRoot | Out-Null
$sourceDir = Join-Path $controlRoot 'source'
$buildDir = Join-Path $controlRoot 'build'
$resultsDir = Join-Path $controlRoot 'results'
$referenceFile = Join-Path $toolRoot 'docs/measurements/mac-target-arm64-2026-10-16.json'

python -c "import platform,struct,sys; sys.exit(0 if sys.platform == 'win32' and platform.machine().lower() in ('amd64','x86_64') and struct.calcsize('P') == 8 else 1)"
if ($LASTEXITCODE -ne 0) { throw 'Windows x64 és 64 bites Python szükséges.' }
git clone --no-checkout https://github.com/RobCZart82/SAWSTAR.git $sourceDir
if ($LASTEXITCODE -ne 0) { throw 'A forrás letöltése hibás.' }
git -C $sourceDir checkout --detach 2f401939db95c4da51e80d041e8911d0258f3b39
if ($LASTEXITCODE -ne 0) { throw 'A rögzített checkout hibás.' }
git -C $sourceDir submodule update --init third_party/DaisySP
if ($LASTEXITCODE -ne 0) { throw 'A pinned DaisySP inicializálása hibás.' }
python scripts/check-dispatch-target-source.py --source $sourceDir --reference $referenceFile
if ($LASTEXITCODE -ne 0) { throw 'A forrásazonossági kapu nem teljesült; mérés nem indulhat.' }
```

Az előellenőrző a pontos commitot, tiszta forrást és submodule-t, DaisySP
gitlinket és a Macen rögzített 20 forrásfájl tényleges SHA-256-át ellenőrzi.
A CRLF-eltérést akkor is elutasítja, ha Git szerint a checkout tiszta.
Nem javít vagy normalizál fájlt, és nem minősít CPU-t/hardvert/fordítót.
A referencia-JSON belső összehasonlítási alap, nem hitelességi aláírás.

## Egyetlen teljes gyűjtés

Csak sikeres előellenőrzés után, ugyanabban a PowerShellben:

```powershell
$env:CMAKE_GENERATOR = 'Visual Studio 17 2022'
$env:CMAKE_GENERATOR_PLATFORM = 'x64'
python (Join-Path $sourceDir 'scripts/run-dispatch-engine-target.py') --build-dir $buildDir --output-dir $resultsDir --rounds 2
if ($LASTEXITCODE -ne 0) { throw 'A gyűjtés hibás; részadatokat megőrizni, automatikus újramérés nélkül.' }
python scripts/check-dispatch-target-source.py --source $sourceDir --reference $referenceFile
if ($LASTEXITCODE -ne 0) { throw 'A forrásazonosság utóellenőrzése hibás.' }
python (Join-Path $sourceDir 'scripts/review-dispatch-engine-target.py') $resultsDir --verify-raw | Out-File -Encoding utf8 (Join-Path $controlRoot 'review.md')
if ($LASTEXITCODE -ne 0) { throw 'A nyers ellenőrzés hibás; az eredmény nem elfogadott.' }
```

Elvárt: `manifest.state=complete`, 32 grid, két külön kör, 288 összehasonlítási
cella. A gyűjtő friss Release buildet és numerikus kvalifikációt készít.
Hiba esetén a manifest/logok/részadatok megmaradnak; nincs automatikus retry,
kedvező ismétlés kiválasztása, küszöblazítás vagy zajlevonás.

Az eredménymappába ne kerüljön plusz fájl: a reviewer minden fájlt számon tart.
A `controlRoot` alatt, a `results` mellett rögzítsd a CPU pontos típusát,
mag-/szálszámát, RAM-ot, Windows-verziót, táp-/energiabeállítást, fordítót,
a háttérterhelésről ismert korlátokat és a futtatás idejét. Sorozatszám nem kell.
A manifest `background_load_controlled`, `native_host_acceptance` és
`production_promotion_allowed` mezői maradjanak `false` értékűek.

## Visszaellenőrzés és következő döntés

A Windows teljes `results` mappáját és a külön gépadatokat kell átadni,
nem csak a képernyőképet vagy a gyorsabb cellákat. ZIP, SHA-256 és tartós
megőrzés szükséges; a nyers CSV-k tömege ne kerüljön a Gitbe.

Mindkét teljes archívum elérésekor, a rögzített mérési checkout eszközével:

```text
python scripts/compare-dispatch-engine-targets.py WINDOWS_RESULTS MAC_RESULTS > COMPARISON_OUTSIDE_BOTH_ARCHIVES.md
```

Az összevető mindkét oldalt nyers szintig ellenőrzi, megtart minden kört,
kontrollt és rosszabb eredményt. Nem állít abszolút gépsebesség-sorrendet vagy
CPU-elfogadást. Ezután külön [natív REAPER-próba](NATIVE_REAPER_CPU_ACCEPTANCE.md)
és a p99/túllépések, valamint a támogatott ráta-/puffertartomány döntése kell.
