# Offline paired premium engine deadline study

Source: a4270816229b848ed704dcdaf4ed9a20b709be11; Windows-2022Server-10.0.20348-SP0; AMD64; Release.

Candidate: rate-unrolled-fir; explicit FIR backend: SSE2.
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
| 48000 | 32 | 0.998830 | 0.953001 | 6 | 7 |
| 48000 | 64 | 0.998586 | 0.990143 | 0 | 0 |
| 48000 | 128 | 0.998868 | 1.007874 | 0 | 9 |
| 96000 | 32 | 0.999438 | 1.022302 | 4096 | 4096 |
| 96000 | 64 | 0.999542 | 0.974964 | 4096 | 4096 |
| 96000 | 128 | 0.998384 | 1.024838 | 4096 | 4096 |
| 192000 | 32 | 1.009016 | 1.006820 | 4096 | 4096 |
| 192000 | 64 | 1.010422 | 1.069296 | 4096 | 4096 |
| 192000 | 128 | 1.010323 | 0.990702 | 4096 | 4096 |

Validated 288 summary rows and 73728 raw blocks.
