# SAWSTAR 1.2.0

Release date 7 November 2026.

## Premium filters and factory sounds

- Calibrated Premium lookup reference engine with LP12, LP24, HP12 and BP12,
  aligned dry/wet paths and 32 samples of host latency.
- 30 built-in factory presets. Six new performance sounds use modulation,
  effects, velocity, mod wheel, pressure and tempo arpeggios. Playing hints
  appear in preset names. The original 24 preset values and indices are retained.
- Vector creator signature in About, with the existing GYR mark.

## Fixes

- Wrap both chorus interpolation indices at the ring boundary, preventing
  cross-channel and out-of-bounds reads exposed by the Velvet Orbit tail.
- Skip unused ARP pattern expansion and sorting while bypassed, retaining held
  notes for re-enabling the arpeggiator.
- Include earlier control/LFO underflow, SUB frequency ceiling, preset mutation
  contention and unchanged Classic Drive normalization fixes.

## Install and migrate

Choose the Windows installer matching the host architecture, or the macOS
Universal installer. Manual-install ZIPs and SHA256SUMS.txt are also supplied.
Close the DAW and preserve the previous plugin and project copies before updating.
Install and platform details are in the bundled bilingual manuals and
[installation guide](INSTALLATION.md).

The release uses the Premium filter. Earlier Classic projects and presets are
readable through the same SAWSTAR identity, parameter IDs and state format, but
the filter changes their sound. Preserve the earlier version for exact Classic
sound recall. The plugin reports 32 samples of latency to the host.

## Verification and known limitations

Robert reported successful use of the 003b007 Premium RC on macOS and Windows
and approved this release on 7 November 2026. The six new exported preset snapshots
also have his listening approval. The final build promotes that engine and GUI
source with release metadata and distribution documentation updates.
Detailed per-case realtime CPU/dropout data, minimum-OS coverage and native
Windows ARM64 host testing remain incomplete. Builds and installers undergo
platform CI and VST3 validation. Installers are unsigned.

**K1 numerical issue remains open.** A preserved Rosetta x86_64 diagnostic on
source 001847b reported a one-ULP pre-FX right-channel difference for Triangle at
176.4 kHz, filter mode 0, polyphonic FX-on processing, frame 152. The final output
at that frame was bit-identical. Controls and rounding mode remained unchanged.
The cause is unknown. Subsequent successful diagnostics do not explain the earlier
negative. [Recorded evidence](measurements/k1-passive-001847b-negative.json).
This issue is carried as a known limitation in the release authorized by Robert.
