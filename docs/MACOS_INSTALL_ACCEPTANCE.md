# macOS installation acceptance - 2026-09-11

Local host: macOS 26.6.2 (25G83), REAPER 7.79. This is one machine,
not validation of all supported macOS versions or a physical Intel Mac.

## RC2 d515c7c

The user authorized native Installer authentication. Package receipt
`io.github.robczart82.sawstar` exists. The installed executable at
`/Library/Audio/Plug-Ins/VST3/SAWSTAR.vst3/Contents/MacOS/SAWSTAR`
matches the candidate executable SHA256:
`f5de15a08510eb15cc0ff678b8a5b8ca7df31a5191d4312135005b312e1f919a`.
REAPER's open-file list confirms this exact system-wide path is loaded.
MAIN, ADVANCED and PRESETS opened; selecting SuperSaw One changed the loaded
name and parameters. No full audio or project-recall acceptance is claimed here.
The previous user-local plugin was moved to a backup outside plugin scan paths.

## Hover fix e169f3e

Both GitHub workflows passed. Firefox downloaded the official artifact
10193955916 from run 34586630525. ZIP SHA256:
`fad8b6875e7b4907bf3912bb008acefd224619fbe5908f5175e0ecd3e6fdcac4`.
Firefox quarantine was present. Finder opened the ZIP through the configured
Keka extractor; the PKG retained quarantine. Opening it produced Apple's
could-not-verify-malware warning with Done and Move to Trash choices.
No quarantine metadata or system-wide protection was removed.
Per-package approval, installation of this revision and visual hover acceptance
remain pending. The earlier successful RC2 installation used an unquarantined
local file and must not be represented as a browser-download acceptance.

The per-package Open Anyway option was present in Privacy & Security and
selected after user approval. macOS then displayed a second explicit Open Anyway
confirmation. Authentication/installation completion remains pending.
