# Offline paired premium engine deadline study

Source: a4270816229b848ed704dcdaf4ed9a20b709be11; macOS-14.8.9-arm64-arm-64bit-Mach-O; arm64; Release.

Candidate: rate-unrolled-fir; explicit FIR backend: NEON.
Compiled workload: stationary-v1; timed control setters: False.
Both paths use Drive factors by rate: {'48000': 4, '96000': 4, '192000': 2}.
16 Poly voices, FX, four filter modes. Ratios are study/reference.
Setup Drive: 20 dB; workload targets: [20] dB. CSV drive_db identifies setup, not the changing timeline.
Each table row uses 16 paired scene observations (four modes x four pairs).
Percentages use host audio time; counts combine the measured scenes only.
Offline wall time, buffer stores included, output checks outside timing.
No native host or portable realtime acceptance; no timing pass/fail threshold.

| Rate | Buffer | Median paired p50 ratio | Median paired p99 ratio | Reference over/4096 | Study over/4096 |
| --- | --- | --- | --- | --- | --- |
| 48000 | 32 | 1.004962 | 1.091409 | 27 | 17 |
| 48000 | 64 | 1.007103 | 2.208633 | 16 | 70 |
| 48000 | 128 | 0.987116 | 1.007890 | 30 | 42 |
| 96000 | 32 | 1.004077 | 1.064758 | 220 | 256 |
| 96000 | 64 | 1.003051 | 0.799609 | 200 | 99 |
| 96000 | 128 | 1.004242 | 1.118391 | 218 | 223 |
| 192000 | 32 | 0.983557 | 0.938808 | 581 | 307 |
| 192000 | 64 | 1.004890 | 0.971054 | 370 | 278 |
| 192000 | 128 | 0.999750 | 1.002627 | 433 | 394 |

Validated 288 summary rows and 73728 raw blocks.
