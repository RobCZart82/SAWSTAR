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
3. A Universal workflow szerepenként nyolc külön streamet futtat, mindet
   **egyszer**: négy waveform × arm64/x86_64. A többi előre kijelölt stream
   hiba esetén is naplót ad, a hibás streamet nem futtatjuk újra.
4. A tényleges Universal tesztexecutable és CMakeCache is megmarad az artifactban.
   Pontos checkout-/executable-/cache-/loghash, architektúra/Rosetta-jelölés és exitkód
   kerül a megőrzött manifestbe, az állapot hiánynál incomplete, hibánál failed.
   Egyetlen hibás stream is teljes jobhibát eredményez; nem készül tesztplugin-csomag.
5. Az új naplók és manifest 90 napos artifactot kapnak a workflow-ban;
   a mostani eredeti hibás joblog a repositoryban tartósan megmarad.

Helyi ellenőrzés: Linux GNU 13.3 Release. A kalibrált pár teljes 480 esete
sikeres; az eredeti kutatási pár ugyanazzal a közös fixture-rel szintén
480/480 esetben sikeres. Mindkettő megtartja a diagnosztikai negatív kontrollt.
A workflow-orkesztráció helyi mockolt siker/egyetlen-hiba kontrolljában
mind a nyolc stream egyszer futott, a loghashok helyesek, a hiba failed maradt.
Ez nem Rosetta-reprodukció és nem a hiba okának bizonyítása.

Következő: a friss pontos-head diagnosztikai CI kiértékelése, szükség esetén
célzott reprodukció; csak bizonyított ok/javítás után zárható le a numerikus hiba.
Nincs küszöblazítás, kedvező számokért újraindított mérés vagy vak merge.
