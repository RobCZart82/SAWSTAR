# 1.0.0 RC1 package record

Candidate source: `c84be79f96877de3b565dd543be14b484f426936`.
Prepared 2026-09-11. No v1.0.0 tag or public GitHub Release was created.
The binaries identify themselves as **1.0.0 Pre Release** and show a Build date.

## Packages

| File | SHA-256 |
| --- | --- |
| SAWSTAR-1.0.0-rc1-macos-arm64.zip | 7438e7cb4114c3b2f2ed764a24bf0b6d7511d80b3f2c8cbc62d570daa0ae63f8 |
| SAWSTAR-1.0.0-rc1-windows-x64.zip | c75a328982f66e7ee4ba6c66ce90d202e209f3af455cb1c32f4813b7a49c7924 |

The Actions artifacts contain the same inner ZIP bytes under the shorter names
SAWSTAR-macos.zip / SAWSTAR-windows.zip. Renaming the outer file does not change
its checksum. Artifact IDs: macOS 10188382879; Windows 10188435099.

- [macOS build](https://github.com/RobCZart82/SAWSTAR/actions/runs/34572520005)
- [Windows build](https://github.com/RobCZart82/SAWSTAR/actions/runs/34572519935)

## Verified

Both VST3 Release jobs passed 34 tests and 47 Steinberg validator tests. Both
foundation Debug and Release jobs also passed on macOS and Windows. All six
jobs completed successfully.

Downloaded artifact SHA-256 digests match GitHub metadata. Inner ZIP checksums,
ZIP integrity, all per-file manifest hashes, included EN/HU manuals, notices and
source commit match. macOS plist reports 1.0.0; both binaries contain the 1.0.0
Pre Release/About build identifier. Windows is PE x64; macOS is Mach-O arm64.
A packaging fixture also verifies preservation of Unix executable permissions.

macOS signature inspection reports an ad-hoc linker signature, no TeamIdentifier
and no sealed resources. This is not Developer ID signing/notarization. The Windows PE certificate directory is empty (no embedded Authenticode
signature). Installed user plugins were not replaced.

## User-reported Windows acceptance

On 2026-09-11, the user reported "Windows teszt ok" following delivery of RC1.
Record this as a successful user-reported Windows manual test for this candidate.
The Windows version, host version and individual test cases were not specified;
this does not establish a clean-install result or a minimum supported OS version.

## Still required for publication

- Final macOS candidate acceptance and clean-install coverage. Windows manual
  acceptance was reported above; detailed test coverage remains unspecified.
- Supported minimum OS versions and signing/distribution policy.
- Final manual edition metadata and screenshots (current PDFs are the explicitly
  labelled 0.1.0 development edition; their function descriptions cover this
  unchanged-feature candidate).
- Actual release date, a rebuild with SAWSTAR_RELEASE_DATE, final package hashes,
  then v1.0.0 tag and release publication. Do not rename RC1 as a final release.
