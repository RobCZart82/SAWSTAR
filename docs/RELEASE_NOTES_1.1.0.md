# SAWSTAR 1.1.0 rc1 — kiadási leírás tervezete

- Fejlesztés alatt: hat új performance factory preset, 30-as bank, modwheel/akkord játékutasításokkal. [Részletek és elfogadási kapuk](EXPRESSIVE_FACTORY_PRESETS_2026_11_03.md).


Nem publikált végleges kiadás. A nyilvános stabil kiadás továbbra 1.0.4.

- Premium LP12/LP24/HP12/BP12 filter integrációs build a SAWSTAR wrapperben.
- Megőrzött pluginazonosság, paraméter-ID-k, GUI és state/preset formátum.
- A Premium build jelentett késleltetése 32 minta, a dry filterút is késleltetett.
- Elfogadott dry unity / wet +3 dB kalibráció és presetenkénti headroom-trimek.
- Korábbi mixer/MIDI/LFO underflow-, SUB-frekvenciaplafon-, preset-zárolási
  javítások és a változatlan Drive-érték normalizálásának újrafelhasználása.

A Classic projektek/presetek betölthetők, de a Premium filter megváltoztatja
hangzásukat. A régi pluginverziót és projekteket a migráció előtt őrizd meg.
A Classic és a Premium production RC ugyanazzal a pluginazonossággal működik;
a build nem telepít automatikusan. A külön Preview/Candidate identitás megmarad.

[RC használat, ellenőrzési hatókör és nyitott kiadási kapuk](PREMIUM_PRODUCTION_RC.md).
A natív CPU/Windows/hallgatási elfogadás és K1 nyitott. A támogatott végleges
ráták/pufferek, RC-csomagok és publikálási dátum még nincsenek elfogadva.
Az eredeti negatív CPU-bizonyítékokat a későbbi zöld CI nem törli.
