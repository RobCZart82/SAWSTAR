# User manuals

- [English PDF](SAWSTAR-User-Manual-EN.pdf)
- [Magyar PDF](SAWSTAR-User-Manual-HU.pdf)

These 15-page guides describe the unpublished SAWSTAR **1.1.0 rc1** and its
30 embedded factory presets. Chapter 15 covers Premium integration, the shared
SAWSTAR identity, migration, 32-sample latency and open acceptance gates.
The public stable release remains 1.0.4; its published packages are unchanged.
Screenshots show SAWSTAR 1.0.3, capture build 8040390. The displayed preset count includes user-library entries; the factory library contains 30 presets.

See the [installation guide](https://github.com/RobCZart82/SAWSTAR/blob/main/docs/INSTALLATION.md) for installation and bilingual macOS troubleshooting.

Rebuild with `python scripts/build-manuals.py` (ReportLab and Pillow). Set
`SAWSTAR_MANUAL_FONTS` to a directory containing DejaVuSans.ttf and
DejaVuSans-Bold.ttf. Outputs are under build-manuals/output/pdf; visually verify
both languages before copying them here.

RC edition: 2026-11-03. Both PDFs were rendered and visually checked in both
languages. They document a test candidate, not a final-release or native PASS.
Historical fixes are identified separately; screenshots remain from 1.0.3.
