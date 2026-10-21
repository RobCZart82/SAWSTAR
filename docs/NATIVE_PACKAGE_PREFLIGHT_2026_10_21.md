# Natív tesztpár csomagaudit — 2026-10-21

**Eredmény: mind a négy megőrzött CI-tesztcsomag konzisztens; hostpróba még nincs.**
Ez a #128-ban implementált macOS preflight valódi csomagpróbája és a Windows
x64 bővítés első valódi csomagkontrollja. Nem K1-reprodukció vagy okbizonyítás.

## Pontos forrás és CI

- Tesztelt csomagforrás: `92d0cd83e5c255c6592817688a24ad753cc2aeec`, #128 head.
- [macOS Preview run 39596022244](https://github.com/RobCZart82/SAWSTAR/actions/runs/39596022244):
  mindkét szerep sikeres, Universal build, saját smoke, két szelet páros numerikus
  kontrollja és VST3-validátora a workflow része.
- [Windows Preview run 39596022289](https://github.com/RobCZart82/SAWSTAR/actions/runs/39596022289):
  mindkét szerep sikeres, x64 build/saját smoke/páros numerikus kontroll,
  PE/runtime audit és SDK-validator/eredetjegyzék a workflow része.
- A #128 hét workflow-ja / 16 jobja sikeres volt a merge előtt.
  Beolvasztás: `ea57553c75179f0470f026028a64b3cef4618340`; ennek az új mainnek
  mind az öt elindult workflow-ja / 12 jobja is sikeres.

A csomagok **a PR headjéből**, nem a squash merge SHA-jából készültek.
A két azonosító nem cserélhető fel; a csomagok manifestje a headet őrzi.
Az új Windows előkészítő kód saját, későbbi PR-CI-je külön kapu.

## Letöltött artifactok

| Platform / szerep | Artifact ID | Artifact külső ZIP SHA-256 |
| --- | --- | --- |
| macOS Reference | 12344630306 | `b9eccfbe14917c8c44be0a3f6a141c5cd6ae568c6bb26aeffaf38531a07d7006` |
| macOS Candidate | 12343559463 | `2fc97da547f6d1486af9678996decb630c2a93e8578644ba3cac55b2090620e7` |
| Windows Reference | 12344062893 | `18c1fc14ad7c7dc90bccf6e00f03a09c8cbd59c3558d7ba76c48fb847948ce2e` |
| Windows Candidate | 12344510231 | `128f5d1f6726c75e0e73b7674ad41076c4a1d5a76ce1728ef34e3b6e226bf4c6` |

A letöltött külső ZIP-ek helyileg számított SHA-256-a egyezett a GitHub API
artifact-digestjével. A belső csomag ZIP-je és `.zip.sha256` oldalfájlja külön
ellenőrzött; a két ZIP-réteg hashát nem keverjük össze.

| Platform / szerep | Belső csomag SHA-256 | Tényleges executable SHA-256 |
| --- | --- | --- |
| macOS Reference | `23af26495f5408e7f55c47ea7167a836e827e868d07432920a3ef1508c2a063f` | `d3c0853e1775227b656f7be46e740407e5c867fe6f6d08b2d905c328536c5318` |
| macOS Candidate | `afe0b93d7db2ffedba6161b09188e805bf213365a93019e782cd47d222390cfb` | `32e423815ff7e2b3dfd438f6d08f95f8f444aac08d498a52f1da08054257677e` |
| Windows Reference | `aa2f5c170e8177b9590d7e0ff7c0aad6f7f181bd6b40d107f658217da5e83dbd` | `a71cf04ab6e264698dfcc014c89d4f9fe3d83f3999b69ddcb9c3fb7843300eca` |
| Windows Candidate | `7767927c2f51206880acfa0dc76236ee48736b583c5e0208bfb02e06d1ef3cdc` | `633d9f2d1157d9db3c20665a44ef7e7b5e0e4717508b41f550c5de63b9978f3f` |

A géppel olvasható [auditjegyzék](measurements/native-package-preflight-2026-10-21.json)
tartalmazza mind a négy teljes manifestet és fájlonkénti hashjegyzéket.
Az artifactok CI-megőrzési ideje korlátozott; ez a jegyzék nem bináris archívum.

## Végrehajtott ellenőrzések

1. Külső artifact-digest, belső ZIP-checksum és fájlnév egyezése.
2. Manifest teljes tagjegyzéke; minden fájl hash-a egyezik, extra/duplikált
   vagy nem reguláris tag nélkül. macOS csomagonként 28, Windows csomagonként
   27 hashjegyzékbe vett fájl; a manifest maga ezeken felüli tag.
3. Mind a négy csomag forrása a fenti pontos head. Explicit Reference/Candidate
   product, plugin-ID és oscillator; közös filter/32 mintás latency/level policy.
   Kísérleti státusz true, natív elfogadás és production-promóció false.
4. macOS: külön plist-identitások, executable hash és executable jogosultság;
   a Universal táblában és szeletfejlécekben arm64 + x86_64.
5. Windows: PE32+ x64; a Reference class ID `F2AEE70D00DE4F4E5243383253775050`,
   a Candidate-é `F2AEE70D00DE4F4E5243383253775043`. A csomagolt SDK module-info,
   manifest és binárisazonosság egyezik. A validator-eredetjegyzék a forráshoz,
   binárishoz és module-info-hoz kötött; a rögzített SDK-commit egyezik.
6. A két valódi platformpáron a teljes előkészítő parancs is lefutott: külön
   provenance/környezet/README és 28 eset / 112 `not_run` CSV-sor keletkezett.
   A sablon előre kijelölt F-módja LP24; P1 hangszín/paraméter/projekt és minden
   környezet-/mérési érték még kitöltendő. Nincs elvégzett zenei teszt.

Helyi futtatás: macOS arm64, Python 3.9. A Windows DLL-ek nem lettek betöltve
vagy végrehajtva. Az új kód 12 preflight/protokolltesztje és a csomagoló
10 meglévő tesztje sikeres. A Windows csomagoló és preflight közös bytes-alapú
identitásellenőrzőt használ, így a két kapu CID-/SDK-policyja nem külön másolat.

## Mi maradt nyitva?

A checksum és eredetjegyzék konzisztenciát igazol, nem helyettesít aláírást vagy
önálló célgépes validatorfuttatást. A fejlécvizsgálat nem teljes loader-audit.
Nem telepítettünk plugint, nem nyitottunk/módosítottunk REAPER-projektet,
nem mértünk host-CPU-t, dropoutot vagy RT-tartalékot, és nem hallgattunk audiót.
A csomagok nem Developer ID/notarizált production kiadások.

Következő lépés: [telepítési smoke és natív mérés](NATIVE_HOST_KIT.md),
azonos mentett P0/P1 preset/MIDI/automatizálás és tényleges hostarchitektúra
rögzítésével. A hibás vagy nem futott esetek megmaradnak, elfogadást Robert
ad a konkrét build/gép/tartomány alapján. K1 oka továbbra ismeretlen;
a production-integráció, végső RC és publikálás kapui külön maradnak.
