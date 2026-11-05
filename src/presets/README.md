# Presets

FactoryPresets.cpp embeds 30 complete parameter snapshots with stable keys,
learning descriptions and searchable tags. Seven dry Templates and 23 finished
sounds cover the factory categories, including the six performance sounds added
in November 2026. Factory sounds need no companion bank.

Library.h combines factory/user sounds, category/tag search and favorites.
QuickPresetList.h builds the sorted navigation list, with Init kept as an explicit
confirmed action. UserPresets.cpp implements exclusive creation, canonicalized
duplicate checks, guarded overwrite, rename, archive and favorites replacement.
ImportJob.h runs batch import and resulting library enumeration on a worker.

Preset files share the sound-state record format but use stricter file validation
than legacy host project migration. See [factory library](../../docs/FACTORY_LIBRARY.md)
and [user presets](../../docs/USER_PRESETS.md) for behavior and limits.
