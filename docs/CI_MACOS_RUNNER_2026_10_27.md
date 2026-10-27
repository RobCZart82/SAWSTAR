# macOS foundation CI — futtatókörnyezet frissítése, 2026-10-27

A `Build macos foundation` workflow Debug/Release és Universal VST3 jobjai
rögzített `macos-15` futtatógépre állnak át. A pontos új head teljes
CI-eredménye a beolvasztás előfeltétele.

## Mi indokolja?

A [GitHub hivatalos közleménye](https://github.blog/changelog/2026-10-01-github-actions-macos-14-runner-image-retirement/)
szerint a `macos-14` képet 2026-11-02-án kivezetik. A közlemény külön
jelzi a lehetséges kapacitáscsökkentést és hosszabb sorban állást, és a
`macos-15` arm64 címkét támogatott alternatívaként nevezi meg.

A #131 `bda5a276063ba364e816cd964a176f70df5bd707` headjén:
- [40529650148 futás](https://github.com/RobCZart82/SAWSTAR/actions/runs/40529650148),
  első kísérlet: Universal job `122278638295` cancelled, steps üres,
  runner ID 0 / név üres. 15:18:24–15:33:25 UTC között nem indult teszt.
- A Debug és Release foundation jobok sikeresek, ahogy a többi PR-workflow is.
- A célzott újraindítás második kísérlete létrejött; a Universal
  `122306506719` job az ellenőrzéskor queued, runner nélkül maradt.
  Ugyanekkor a repositoryban más in-progress workflow nem volt.

A konkrét job elakadásának közvetlen oka nem bizonyított: a csökkentett
`macos-14` kapacitás lehetséges magyarázat. A mai várakozás nem nevezhető
a közlemény előző napra kijelölt brownout-időablakának.
Nem jelent meg fordítási, numerikus vagy K1-hibanapló; a régi megszakadt
futás megmarad, eredményét nem írjuk át sikeresre.

## A módosítás és kapuja

A fő macOS buildworkflow két `runs-on` értéke változik; a foundation
kötelező checkneve visszafelé kompatibilis marad. A `macos-latest` mozgó alias helyett explicit `macos-15`
szerepel. A Debug/Release tesztmátrix, összes teszt, benchmarkküszöb,
időkeret és függőségpin megmarad. A Universal VST3 továbbra arm64+x86_64,
11.0 deployment targettel, validatorral, csomag-/installerpróbával készül.

Ez CI-gép-/OS-/toolchain-váltás: új futás, külön eredet. A régi `macos-14`
mérési bizonyíték nem válik `macos-15` mérésnek, és a termék minimum OS-e
nem emelkedik ettől 15-re. A CI-siker nem sajátgépes REAPER- vagy natív
Intel-próba, és nem oldja fel K1-et vagy a production/RC/publikálási kaput.
A többi, külön macOS workflow kivezetési felülvizsgálata külön feladat;
azok régi diagnosztikai eredményeit ez a változás nem írja át.

## Kötelező checknevek és igazolt futás

A `c45b66228851cfc2721f44d27f6ed1458532c129` head mind a négy workflow-ja /
11 jobja sikeresen befejeződött, köztük a macOS 15 Debug/Release és Universal
VST3/validator/installer ellenőrzése. Ez a kör már nem félkész.

A beolvasztás ellenőrzésekor a [main ruleset 23025854](https://github.com/RobCZart82/SAWSTAR/rules/23025854)
még `macos-14 / Debug` és `macos-14 / Release` nevű kötelező checkeket várt.
A runnercsere melletti névcsere ezért a sikeres tesztek ellenére blokkolta a
merge-et. A workflow visszaállítja a két stabil checknevet; ugyanazokban a
teljes jobokban továbbra ténylegesen `macos-15` gép és változatlan tesztek futnak.
A megjelenő `macos-14` szöveg ebben a két checknévben örökölt azonosító,
nem a tényleges OS állítása; a `runs-on`, runneradatok és setupnapló mutatják
az új környezetet. A workflow kommentje ezt külön jelzi.

A védelmi szabály nem változik, nincs bypass, helyettesítő sikerjelzés vagy
teszt nélküli bridge-job. Az új head összes checkjének és a teljes előző
mainnek ismét sikeresnek kell lennie. A c45b662 kör eredménye nem másolható
át az új headre. Későbbi, összehangolt ruleset-átnevezésnél a megjelenített
checknév is modernizálható.
