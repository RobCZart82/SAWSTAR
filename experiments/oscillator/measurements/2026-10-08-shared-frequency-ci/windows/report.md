# Isolated oscillator frequency sharing study

Source: eae61b720ba3b5f1c1aacc49936d9701580984f8
32 oscillator banks represent OSC1 + OSC2 for 16 voices; no engine, filter or FX timing.
Four alternating pairs. Ratios are study/reference; smaller is favorable.
Static and audio-rate pitch modulation are separate controls. No native realtime acceptance.

CPU acceptance: not established. Production promotion: not allowed by this isolated study.
Median slowdown cells: 21/24; all four pairs slower: 13/24.
These are observations, not statistical significance; green CI validates the study only.

| Rate | Waveform | Pitch modulation | Median paired time ratio |
| --- | --- | --- | --- |
| 48000 | 0 | False | 1.042226 |
| 48000 | 0 | True | 1.090494 |
| 48000 | 1 | False | 1.007129 |
| 48000 | 1 | True | 1.011875 |
| 48000 | 2 | False | 1.010574 |
| 48000 | 2 | True | 0.986554 |
| 48000 | 3 | False | 1.006078 |
| 48000 | 3 | True | 1.016117 |
| 96000 | 0 | False | 1.054252 |
| 96000 | 0 | True | 1.093719 |
| 96000 | 1 | False | 1.019767 |
| 96000 | 1 | True | 1.009340 |
| 96000 | 2 | False | 0.984028 |
| 96000 | 2 | True | 1.110004 |
| 96000 | 3 | False | 1.013169 |
| 96000 | 3 | True | 1.011835 |
| 192000 | 0 | False | 1.056831 |
| 192000 | 0 | True | 1.099992 |
| 192000 | 1 | False | 0.996564 |
| 192000 | 1 | True | 1.018915 |
| 192000 | 2 | False | 1.013878 |
| 192000 | 2 | True | 1.011031 |
| 192000 | 3 | False | 1.007744 |
| 192000 | 3 | True | 1.017487 |
