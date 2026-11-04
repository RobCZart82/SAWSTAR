# Chorus körpuffer-határ: Velvet Orbit — 2026-11-04

A #135 bankbővítés Windows Debug tesztje három felosztási változatban is
időkorlátba ütközött. Az utolsó két headen a hiba egyértelműen a Velvet Orbit
25. indexéhez tartozik; a másik 29 hang és Release/VST3 munkák sikeresek.
Az időkorlát emelése, a hang módosítása vagy a render rövidítése nem javítás.

## Bizonyított címzési hiba

A `Chorus::Read` a negatív olvasási pozícióhoz hozzáadta a 8192-es
körpufferhosszt. Float kerekítés miatt egy apró negatív szám ekkor pontosan
8192-re kerekedhet. Az első interpolációs cím nem volt körbeforgatva,
csak a második: az első cím ilyenkor érvénytelen 8192, a legnagyobb érvényes 8191.

Determinista helyi negatív kontroll: 48 kHz, write=576,
`milliseconds=nextafter(12.f,+infinity)` = 12.0000009537;
delay=576.00012207, körbeforgatott float pozíció=8192. Bal puffer végig +0,25,
jobb végig -0,75: a régi bal olvasás -0,75 volt +0,25 helyett. Azaz a régi kód
átolvasott a következő csatornába; a jobb csatornán az olvasás a pufferek utánra esik.
Ez tényleges határon túli hozzáférés, nem zajküszöb vagy lassú mérés.

A valódi Velvet Orbit chorusütemezés (.2 Hz, 30% depth, 44,1 kHz) külön
előrejelzője a 565729. mintánál, 12,828322 másodpercnél mutatott érvénytelen
címet: tap2, write481, index8192. A Windows-napló single2s + release10s
feldolgozásig 2,7167 s alatt eljutott, a release11s checkpoint már hiányzott.
A pontos időegyezés összhangban van a Windows Debug bounds-ellenőrzésénél
megálló hozzáféréssel; a javított Windows-futás külön ellenőrzés.
[Negatív mérések és pontos workflow/job azonosítók](measurements/chorus-ring-boundary-2026-11-04.json).

## Javítás és regresszió

Az eredeti egész pozícióból számított törtrész megmarad, de az első cím is
`whole % kSize` szerint körbefordul. Így 8192 → 0, az interpoláció mindkét
szomszédja ugyanannak a csatornának érvényes eleme. Más pozíciónál az eredeti
számítás bitazonos marad. Nincs új allokáció, FP-módváltás vagy presetváltoztatás.

Az existing `stereo_chorus` tesztben név szerinti friend-hozzáférés teszi
ellenőrizhetővé a privát olvasót; ez nem plugin-API vagy új futásidejű állapot.
A regresszió a javítás előtti tényleges `Read` kóddal megbukott:
`rounded wrap must read index zero in the same channel`.
További ellenőrzések: 262144 normál interpoláció bitazonossága (4 ráta,
8192 write-pozíció, 4 delay, 2 csatorna), valamint 13 másodperc tényleges
Velvet Orbit chorusütemezés, megkülönböztetett konstans csatornákkal.

A presetek jóváhagyott értékei megmaradnak. A DSP-változás a hibás
puffercímet javítja; a későbbi exact-head CI, RC-csomag és natív kapuk külön
állapotok. A K1 korábbi, pre-FX numerikus eltérése ettől nem minősül javítottnak.


## Helyi javítás utáni eredmény

- Teljes Classic Release CTest: **173/173 sikeres** (91,31 s).
- Debug: a chorus regresszió és a 30 factory feladat **31/31 sikeres** (120,54 s).
- Premium production state/identity: **2/2 sikeres**.
- Premium limiter előtti headroom: **252/252 sikeres**, worst **-6.51292972 dBFS**;
  a teljes CSV byte-egyezik a korábbi commitolt rácséval.
- Külön AddressSanitizer/UBSan boundary-probe: sikeres, a korábbi -0,75 helyett
  a helyes +0,25 bal csatorna. A régi feldolgozási kód negatív kontrollja megmarad.
- Valódi arm64 VST3 újrafordult; nincs automatikus install/publikálás.

A távoli Windows Debug és valamennyi pontos-head CI/RC bizonyíték még függő.
A korábbi K1 negatív külön nyitott, a chorus-javítás nem K1-javítás.
