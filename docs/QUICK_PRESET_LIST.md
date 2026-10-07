# Quick preset selector

The top-bar selector presents factory and User presets together in
case-insensitive name order. Each menu label includes its category, e.g.
`Air Keys [Keys]` or `My Sound [User]`. Sorting uses the existing `Fold`
normalization; it is not locale-specific dictionary collation.

The left/right arrows follow the same order as the popup and wrap at the ends.
From Custom, Init, or an external preset outside the User Library, right selects
the first listed sound and left selects the last. `INIT PRESET` remains a
separate confirmed action after a separator, outside arrow navigation.

The menu retains the original factory index or exact User file path. Display
sorting does not change parameter IDs, saved state, factory sound identity,
or file contents. Equal folded names have a deterministic category/name/source
tie-break, so a User sound named after a factory preset remains distinguishable.

`preset_library_import` checks sorted order, category labels, factory IDs and
User paths after sorting, duplicate/case-variant names, reversed input order,
unknown selection, wraparound, and complete arrow traversal. The VST3 CI builds
compile the production selector. Native GUI acceptance should verify popup
selection, arrows, User loading and the Init confirmation in REAPER.
