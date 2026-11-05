# GUI controls

These controls implement the keyboard, performance wheels, waveform/noise
selectors, scope, page navigation, preset browsing and confirmation overlay.
Layout.h binds them to stable parameter IDs and plugin callbacks.

Audio rendering belongs to the engine. Meter/scope and held-note updates cross
the audio/UI boundary through their dedicated mailboxes or atomics. Preset file
actions run on the UI/file side; batch import uses PresetImportJob. Modal callbacks
use lifetime guards where the editor may close before completion.

The graphics stub tests cover selected control logic, not native window dispatch
or the complete rendered interface.
