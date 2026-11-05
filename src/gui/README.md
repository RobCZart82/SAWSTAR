# GUI

Layout.h builds the MAIN, ADVANCED and PRESETS control groups; Pages.h names
their purposes. SAWSTAR.cpp supplies parameter, factory-preset and state callbacks.

MAIN exposes sound design and signal flow. ADVANCED contains performance,
arpeggiator and modulation controls. PRESETS provides factory/user browsing,
search, favorites, import, save/overwrite, rename and recovery-backed deletion.
Controls/ contains the shared graphics controls, keyboard, performance wheels,
scope, preset browser and confirmation overlay.

PresetImportJob performs import and library enumeration on a worker. Other
preset actions can still perform synchronous filesystem work on the UI thread;
this does not establish a fully asynchronous editor. GUI/state/preset lifecycle
and native rendering acceptance are tracked in the development documentation.
