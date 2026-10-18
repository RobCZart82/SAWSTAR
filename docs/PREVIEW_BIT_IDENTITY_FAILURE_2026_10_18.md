# Kalibrált Preview-pár — nyitott bitazonossági hiba, 2026-10-18

**Állapot: az ok nem azonosított; a #116 beolvasztási sora megállt.**
A diagnosztikai módosítás nem DSP-javítás. Egy későbbi zöld futás önmagában
nem magyarázza meg és nem törli az itt rögzített negatív bizonyítékot.
Nincs CPU-/natív elfogadás vagy production-promóció.

## Ellenőrzött tények

- PR-head: `05b730d58dd2f23fe08676b6114b436524923c93`.
- [Workflow 39126567559](https://github.com/RobCZart82/SAWSTAR/actions/runs/39126567559),
  [hibás Universal / reference job 117672833349](https://github.com/RobCZart82/SAWSTAR/actions/runs/39126567559/job/117672833349).
- A main `547f84d4` négy fő workflow-ja / 11 jobja sikeres; a PR 18 jobjából
  17 sikeres, a külön Windows-pár és a Candidate Universal-job is sikeres.
- A Reference-jobban a numerikus motorpár x86_64/Rosetta futása
  `Dispatch engine bit identity` kivétellel, exit 1-gyel állt meg.
  Ez nem timeout, nem CPU-küszöb, nem csomagolási vagy validator-hiba.
- A naplóban a Triangle (`waveform=2`) natív arm64 teljes 120 esete még
  nulla eltéréssel befejeződött. Az utána következő x86_64 stream utolsó
  sikeres sora `2,96000,0,2,1,0,0,0,0`; a következő tervezett kombináció
  96 kHz / LP24 / Poly / FX OFF. Az eredeti kód nem írta ki az első hibás
  mintát, pontot, csatornát és bitmintát; **a konkrét eltérés mérete/oka ismeretlen**.
- A #116 addigi változása előkészítő eszköz és dokumentáció, nem DSP-módosítás.
  Ettől az észlelt hangmintaeltérés még valós blokkoló bizonyíték: nem
  címkézzük bizonyíték nélkül teszthibának, zajnak vagy ártalmatlan eltérésnek.

## Tartós eredeti napló

A GitHub eszköz által visszaadott teljes dekódolt joblog UTF-8 byte-jai,
a kapott sorvégek megtartásával, változtatás nélkül tömörítve:
[preview-reference-rosetta-2026-10-18.log.gz](ci-evidence/preview-reference-rosetta-2026-10-18.log.gz).
Ez a dekódolt joblog archiválása, nem az eredeti HTTP-transzport ZIP digestje.

- Dekódolt UTF-8: 66 606 byte, SHA-256
  `3186fac93a02e53efa06f168b17386173bd6c8b8eaee66624b790cce4f204e59`.
- GZIP: 11 363 byte, SHA-256
  `0cd11d2757876d765b956540042e40e9cd1b00866e2af1f837af3b9a1dc4cefb`.

A korábbi célgépes CPU-kampányok, hashok, küszöbök és az ellenőrzött
`c55ca8f` csomagpár jegyzéke változatlanul megmaradnak. Azok nem javítják
meg automatikusan ezt a később észlelt numerikus hibát.

## Mostani diagnosztikai fejlesztés

1. Az eredeti 480 eset, események, mintaszámok, bitpontos `memcmp`, véges-jel,
   kimeneti védelem, MIDI/lifecycle és negatív módosított-jel kontroll megmarad.
2. Az első biteltérés most kiírja a waveform/rate/filter/voice/FX kombinációt,
   main/tail szakaszt, mintaszámot, output/pre-FX pontot, bal/jobb csatornát,
   két float értéket, 32 bites hex bitmintájukat és a numerikus különbséget.
   A szándékos negatív kontroll a diagnosztika ismert bitmintáit is ellenőrzi.
3. A `089445cb` diagnosztikai head mind a 9 workflow-ja és 26 jobja sikeres
   ([Universal futás](https://github.com/RobCZart82/SAWSTAR/actions/runs/39141889969)).
   Ez nem reprodukálta a hibát, nem bizonyítja az okát és nem oldja fel a merge-kaput.
4. A következő, előre rögzített reprodukciós próba szerepenként **14 stream**:
   az eredeti nyolc waveform × architektúra stream, majd három további Triangle
   hidegfolyamat mindkét architektúrán. Így Triangle esetén pontosan **4 külön
   folyamat / architektúra** fut, minden kijelölt sorszám egyszer. Nincs
   sikerig ismétlés, kedvező eredmény kiválasztása vagy CPU-kampány.
5. Az opt-in `--diagnostic-waveform` az eredeti 120 esetet futtatja, és a végső
   output/pre-FX minták Reference/Candidate ujjlenyomatát is kiírja.
   FNV-1a-64 diagnosztikai ujjlenyomatot használ, explicit little-endian float
   byte-sorrenddel; **ez nem kriptográfiai nyershang-hash**. A kanonikus
   ujjlenyomattábla és az eredeti log/executable/cache eredetigazolása SHA-256.
   Azonos architektúra külön folyamatait hasonlítjuk össze, nem arm64-et x86_64-gyel.
   Az eredeti alapértelmezett és `--waveform` CSV 9 oszlopa változatlan.
6. A 0. sorszámú Triangle-folyamat az előre kijelölt alap. Ha hibás vagy hiányos,
   későbbi siker nem helyettesítheti. A teljes 120-as grid, nulla biteltérés,
   páron belüli ujjlenyomat-egyezés és a külön folyamatok közötti egyezés is kapu.
   A folyamatok közötti eltérés akkor is blokkol, ha mindegyik exitkód 0.
7. A tényleges Universal tesztexecutable és CMakeCache is megmarad az artifactban.
   Pontos checkout-/executable-/cache-/loghash, architektúra/Rosetta-jelölés,
   hidegfolyamat-sorszám, exitkód és ellenőrzési hiba kerül a manifestbe.
   Hiba esetén a többi előre kijelölt stream is megmarad; bármelyik hibája az
   egész jobot failed állapotban tartja, és nem készül tesztplugin-csomag.
   Az első biteltérés az FP-kerekítési módot, x86-on az MXCSR-t is rögzíti,
   még a lebegőpontos diagnosztikai formázás előtt.
8. Az új naplók és manifest 90 napos artifactot kapnak a workflow-ban;
   az eredeti hibás joblog a repositoryban tartósan megmarad.

## Helyi ellenőrzés és korlátai

Linux GNU 13.3 Release: a kalibrált pár teljes 480 esete sikeres. A két külön
Triangle-folyamat 120/120 esete és azonos architektúrás ujjlenyomattáblája egyezik;
a kutatási motorpár diagnosztikai 120 esete is sikeres. Az FP-állapot rögzítésének
formázás elé helyezése után mindkét target újrafordult; a végső kalibrált Triangle
ellenőrzés 120/120 esete sikeres, ujjlenyomattáblája a korábbi kettővel egyezik. Az eredeti negatív módosított-jel
kontroll és az ismert float-ujjlenyomat kontroll megmarad.

A reprodukciós orkesztráció 7 regressziós tesztje ellenőrzi a teljes 14 streamet,
az összes 0 exitkód ellenére változó jelet, a hibás alap megőrzését, hiányos gridet,
páron belüli eltérést, menet közben módosuló executable-t, a meglévő bizonyíték
felülírásának és a nem natív futtatásnak a tiltását. A meglévő natív tesztkészlet
7 regressziója is sikeres. Ezek mockolt vezérlési kontrollok, nem natív hostmérések.

Egy inicializálási hipotézist a konstrukciós lánc és a valódi, nem aggregate
`SevenSaw` osztállyal végzett értékinicializálási próba nem igazolt: a `Voice{}`
útvonalon a vizsgált Daisy-oszcillátor előzőkimenet-tagja nullázódott.
Ezen az alapon nincs DSP-módosítás, és ez nem bizonyítja az eredeti hiba okát.

Következő: a friss pontos-head, véges reprodukciós CI kiértékelése; csak
bizonyított ok/javítás után zárható le a numerikus hiba. Egy véges próba sikere
önmagában nem bizonyít hibamentességet. Nincs küszöblazítás, kedvező számokért
újraindított mérés, CPU-/natív elfogadás, production-promóció vagy vak merge.
