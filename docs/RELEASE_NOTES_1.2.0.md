# SAWSTAR 1.2.0 rc1 — release notes draft

Development candidate, not a published final release. The public stable version
remains 1.0.4. [Release checklist and acceptance gates](RELEASE_1.2.0_CHECKLIST.md).

## Sound and presets

- Explicit Premium production integration build: calibrated lookup reference
  LP12/LP24/HP12/BP12 filters, aligned dry/wet path and 32-sample host latency.
- 30 built-in factory presets, including Wheelfire, Velvet Orbit, Neon Current,
  Glass Circuit, Copper Pulse and Polar Keys. Names include wheel/chord playing
  hints. The original 24 values, identities and indices are preserved.
- Robert approved the six exported new snapshots after listening; this remains
  distinct from native acceptance of the exact packaged RC and the entire bank.

## Fixes and compatibility

- Prevent rounded chorus ring reads from crossing channels or reading outside
  the buffer; regressions cover the boundary and Velvet Orbit trajectory.
- Skip unused ARP pattern expansion/sorting in bypass, retaining MIDI forwarding
  and rebuilding held notes on enable.
- Retain earlier control/LFO underflow, SUB frequency-ceiling, preset mutation
  contention and unchanged Classic Drive normalization fixes.
- Parameter IDs, SAWSTAR plugin identity and preset/state formats are retained.
  Classic projects remain readable, but the Premium filter changes their sound.
  Keep the old plugin and project backups. Classic and Premium production share
  the same identity and should not be installed side by side under that identity.

## Candidate scope

The default build remains Classic; Premium integration packages use the explicit
production flag. Windows ARM64 Premium RC is not qualified. General Windows/macOS
user feedback is positive, but exact-RC CPU/dropout, filter/rate/buffer policy,
GUI/state/lifecycle and final bank acceptance remain open. K1's preserved Rosetta
pre-FX numerical mismatch is unresolved; later successful runs do not explain it.
No final release date, tag or publication is claimed by this draft.
