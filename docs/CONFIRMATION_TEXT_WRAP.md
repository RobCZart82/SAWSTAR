# Megerősítő szövegek tördelése — 2026-10-13

## Hiba és javítás

A Save megerősítése a preset nevét a szövegbe illeszti. A megengedett,
legfeljebb 80 karakteres név egy hosszú, szóköz nélküli szó is lehet.
A korábbi ConfirmAction csak szóköznél tört sort; az ilyen név ezért
kifuthatott az 522 pixeles szövegterületből. A fix 230 pixeles ablak
nem biztosított helyet a hosszabb, több sorra tördelendő üzeneteknek.

Az új közös WrapText a tényleges IGraphics betűszélesség-mérést használja,
szóköznél tördel, és hosszú szónál UTF-8-kódponthatáron is vág. Nem vágja
ketté a többbyte-os karaktert; megőrzi az explicit sortöréseket. Ez nem
általános Unicode grapheme-cluster algoritmus. Egy önmagában túl széles
kódpontot külön sorba tesz, így mindig előrehalad.

A sorok a párbeszéd megnyitásakor készülnek el, nem minden Draw alatt.
A rövid üzenetek ablaka továbbra is 230 pixel magas; a hosszabbak több
helyet kapnak. A Cancel és a megerősítő gomb ugyanazon dinamikus dobozból
számítódik. A műveleti callback és billentyűkezelés nem változik.

## Ellenőrzés és korlátok

A confirmation_text_wrap CTest 80 karakteres szóköz nélküli névvel,
magyar kétbyte-os és négybyte-os Unicode karakterekkel, szóhatárral,
explicit/üres sorokkal és szűk szélességgel dolgozik. Ellenőrzi a megőrzött
szöveget és a megerősítő gombok előtti szabad helyet.

A régi algoritmus reprodukciója a determinista mérőfixture-ben 830 pixel
széles sort hagyott az 522 pixeles területre. Ez szintetikus mérőfixture,
nem natív fontmérés vagy REAPER-képernyőkép. A helper GCC Release próbája
sikeres; Windows/macOS plugin-fordítás és natív, skálázott GUI-ellenőrzés
külön kapu. A teljes preset-életciklus elfogadását nem zárjuk le.

Nincs DSP-, state-, parameter ID-, factory preset- vagy verzióváltozás.
A prémium motor célgépes CPU-elfogadása továbbra is nyitott.
