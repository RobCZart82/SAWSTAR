# Offline paired premium engine deadline study

Source: a4270816229b848ed704dcdaf4ed9a20b709be11; Windows-2022Server-10.0.20348-SP0; AMD64; Release.

Candidate: rate-unrolled-fir; explicit FIR backend: SSE2.
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
| 48000 | 32 | 0.993818 | 0.918931 | 1 | 0 |
| 48000 | 64 | 0.996006 | 1.018719 | 4 | 4 |
| 48000 | 128 | 0.998039 | 0.961897 | 0 | 1 |
| 96000 | 32 | 0.995075 | 0.982593 | 4096 | 4096 |
| 96000 | 64 | 0.995215 | 1.019448 | 4096 | 4096 |
| 96000 | 128 | 0.996499 | 0.993633 | 4096 | 4096 |
| 192000 | 32 | 1.020520 | 1.006976 | 4096 | 4096 |
| 192000 | 64 | 1.018972 | 1.003714 | 4096 | 4096 |
| 192000 | 128 | 1.021967 | 1.012603 | 4096 | 4096 |

Validated 288 summary rows and 73728 raw blocks.
