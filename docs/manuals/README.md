# User manuals

- [English PDF](SAWSTAR-User-Manual-EN.pdf)
- [Magyar PDF](SAWSTAR-User-Manual-HU.pdf)

These guides are generated from the repository release metadata. In this
branch they describe SAWSTAR 1.2.2, published on 13 November 2026.
The edition date is build metadata; publication is recorded in the
[release checklist](https://github.com/RobCZart82/SAWSTAR/blob/main/docs/RELEASE_1.2.2_CHECKLIST.md).
The bank has 30 built-in factory presets. Chapter 15 covers the Premium filter,
32-sample latency, Classic project migration, user feedback and the known K1
numerical issue, plus the block-based parameter-automation limitation.
The K1 section summarizes three preserved negative cases and
does not claim a root cause or fix. Screenshots show 1.0.3 build 8040390 and
illustrate the layout. The six new preset snapshots have listening approval;
the 1.2.1 RC has reported Windows/macOS listening, loading and saving feedback.
This does not establish exact-final-build acceptance, a complete CPU/dropout
matrix or native ARM64 host coverage. This preparation does not replace the
already published release assets.

Rebuild with python scripts/build-manuals.py, ReportLab and Pillow.
SAWSTAR_MANUAL_FONTS selects the DejaVuSans font folder. Both languages were
rendered and visually checked, with text bounds verified on all 30 pages.
