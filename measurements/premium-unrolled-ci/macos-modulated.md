# Offline paired premium engine deadline study

Source: a4270816229b848ed704dcdaf4ed9a20b709be11; macOS-14.8.9-arm64-arm-64bit-Mach-O; arm64; Release.

Candidate: rate-unrolled-fir; explicit FIR backend: NEON.
Compiled workload: modulated-v1; timed control setters: True.
Both paths use Drive factors by rate: {'48000': 4, '96000': 4, '192000': 2}.
16 Poly voices, FX, four filter modes. Ratios are study/reference.
Setup Drive: 20 dB; workload targets: [0, 12, 24] dB. CSV drive_db identifies setup, not the changing timeline.
Each table row uses 16 paired scene observations (four modes x four pairs).
Percentages use host audio time; counts combine the measured scenes only.
Offline wall time, buffer stores included, output checks outside timing.
No native host or portable realtime acceptance; no timing pass/fail threshold.

| Rate | Buffer | Median paired p50 ratio | Median paired p99 ratio | Reference over/4096 | Study over/4096 |
| --- | --- | --- | --- | --- | --- |
| 48000 | 32 | 1.002727 | 0.900480 | 21 | 5 |
| 48000 | 64 | 1.005603 | 1.044123 | 6 | 9 |
| 48000 | 128 | 1.001075 | 0.989455 | 10 | 18 |
| 96000 | 32 | 0.989654 | 0.902873 | 261 | 205 |
| 96000 | 64 | 1.003548 | 1.125357 | 224 | 361 |
| 96000 | 128 | 0.998846 | 1.083811 | 167 | 272 |
| 192000 | 32 | 0.995167 | 0.823691 | 316 | 310 |
| 192000 | 64 | 1.004855 | 0.850240 | 461 | 432 |
| 192000 | 128 | 0.999775 | 0.998343 | 580 | 662 |

Validated 288 summary rows and 73728 raw blocks.
