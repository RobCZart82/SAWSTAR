# Premium RC factory bank — helyi műszaki ellenőrzés, 2026-11-03

Állapot: 144/144 limiter előtti headroom-eset sikeres; zenei elfogadás nyitott.
A meglévő, elfogadott Premium preset-trimek változatlanok. 24 preset,
44,1/48 kHz, 1/8/16 kitartott MIDI-gyökér, velocity 127, 4 s tartás + 2 s tail.
Az ARP az adott preset saját beállításait használja. A 6 dB-es küszöb változatlan.
A legrosszabb csúcs −6,51293 dBFS. A limitert kizárólag a külön offline
mérőpéldány kerüli meg; a VST3-ban minden védelem megmarad.

Helyi macOS arm64 / AppleClang 21 Release; nem REAPER, nem Windows,
nem CPU-mérés. [Nyers CSV](measurements/premium-production-rc1-factory-local-arm64.csv).
CSV SHA-256: `c66789ab4445891b02338ac387008bbc96168c89567c7d36cc329aef69c0b9e7`.
A forrás még munkapéldány volt a méréskor; a CSV nem állít végleges RC-binárisazonosságot.
A pontos új PR-head távoli CI-je és a csomagok validátora külön kapu.

| Preset | Eset | Legnagyobb limiter előtti csúcs (dBFS) | Műszaki headroom | Hallgatás |
| --- | ---: | ---: | --- | --- |
| init | 6 | -6.660 | PASS | nyitott |
| wide_lead | 6 | -7.146 | PASS | nyitott |
| soft_pad | 6 | -6.964 | PASS | nyitott |
| saw_pluck | 6 | -6.690 | PASS | nyitott |
| deep_bass | 6 | -11.597 | PASS | nyitott |
| bright_keys | 6 | -7.216 | PASS | nyitott |
| supersaw_one | 6 | -6.513 | PASS | nyitott |
| soft_pad_one | 6 | -7.427 | PASS | nyitott |
| pad_init | 6 | -7.019 | PASS | nyitott |
| lead_init | 6 | -6.669 | PASS | nyitott |
| pluck_init | 6 | -6.668 | PASS | nyitott |
| bass_init | 6 | -9.862 | PASS | nyitott |
| silk_lead | 6 | -7.584 | PASS | nyitott |
| warm_horizon | 6 | -7.287 | PASS | nyitott |
| glass_pluck | 6 | -7.486 | PASS | nyitott |
| simple_steps | 6 | -11.231 | PASS | nyitott |
| keys_init | 6 | -6.679 | PASS | nyitott |
| arp_init | 6 | -6.974 | PASS | nyitott |
| solid_saw | 6 | -7.144 | PASS | nyitott |
| velvet_pad | 6 | -7.239 | PASS | nyitott |
| trance_pluck | 6 | -6.797 | PASS | nyitott |
| round_sub | 6 | -11.855 | PASS | nyitott |
| octave_steps | 6 | -13.244 | PASS | nyitott |
| air_keys | 6 | -6.916 | PASS | nyitott |

A külön production-state próba minden factory preset mentését, sound/host
state visszatöltését, 125%-os GUI-scale rekordját és két példány visszahívott
renderének egyezését ellenőrizte. Ez nem hostablak/GUI-próba.
A két production smoke/state regresszió helyben 2/2 sikeres.
A zenei koherencia, presetenkénti meghallgatás, végső natív clipping/CPU/tail
és pontos RC telepítési próba még szükséges. Nem minősítünk késznek hallgatási
feladatot numerikus mérés alapján.
