# Compact decimator storage in the research Drive

The fixed-factor research Drive previously allocated a 256-slot decimator ring
per channel for both 2× and 4× processing. The 2× FIR reads 65 oversampled
values; a 128-slot power-of-two ring is sufficient. The 4× FIR reads 129 values
and retains its 256-slot ring. Both rings remain mirrored, so descending and
ascending SIMD/scalar reads stay contiguous at every cursor position.

Only history capacity, mirror offset and cursor mask change. FIR coefficients,
phase order, lane accumulation order, gain smoothing and 32-host-sample latency
are preserved. `Clear`, channel isolation after invalid input, live-state copies
and `Init` still reset or preserve state according to their existing contracts.
Compile-time guards require a power-of-two ring with room for every FIR tap.

## Storage evidence

Local MSVC 19.44.35229 x64 Release object sizes:

| Fixed-factor Drive | Previous compact object | New object | Reduction |
| --- | ---: | ---: | ---: |
| 2× stereo | 11,344 bytes | 7,248 bytes | 4,096 bytes |
| 4× stereo | 12,384 bytes | 12,384 bytes | 0 bytes |

A bank of sixteen 2× Drive objects saves 65,536 bytes (64 KiB). The rate-scaled
research adapter stores both factors, so it also benefits from the smaller 2×
member even when its 4× path is selected. These are object-storage figures,
not a measurement of a loaded plugin's total memory or CPU time.

## Numerical and lifecycle evidence

`premium_drive_polyphase_identity` compares the candidate with two independent,
frozen implementations already in the repository: the sparse `715cd47` and
compact runtime-phase `d4c7822` fixtures. The local Release run compared
1,996,800 stereo frames bit for bit, at six rates from 8 to 384 kHz, with repeated
ring wraps, gain changes, clear/snap, invalid samples, copied live state,
impulses around wrap positions and reinitialization. New compile-time checks
require exactly 4,096 bytes less storage at 2× and unchanged storage at 4×
relative to the compact fixture.

The existing SIMD identity, rate-scaled routing, FIR-only cost control, tanh,
high-rate quality and spectral tests also passed in the local MSVC x64 Release
harness: seven existing tests in total, compiled directly from repository test
sources without the DaisySP engine dependency. Full
Windows/macOS and ASan/UBSan CI provides the platform and bounds-check gates.

## Remaining work

This change is confined to the offline research filter. It does not close the
CPU/deadline gate, approve the high-rate quality policy, integrate the new
filter into the shipping synth, or replace native REAPER acceptance. A cache
benefit is plausible but must be measured against the same implementation
with only ring capacity changed before claiming a CPU improvement.
