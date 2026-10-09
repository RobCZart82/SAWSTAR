# Isolated oscillator frequency sharing study

Source: eae61b720ba3b5f1c1aacc49936d9701580984f8
32 oscillator banks represent OSC1 + OSC2 for 16 voices; no engine, filter or FX timing.
Four alternating pairs. Ratios are study/reference; smaller is favorable.
Static and audio-rate pitch modulation are separate controls. No native realtime acceptance.

CPU acceptance: not established. Production promotion: not allowed by this isolated study.
Median slowdown cells: 13/24; all four pairs slower: 0/24.
These are observations, not statistical significance; green CI validates the study only.

| Rate | Waveform | Pitch modulation | Median paired time ratio |
| --- | --- | --- | --- |
| 48000 | 0 | False | 1.037590 |
| 48000 | 0 | True | 1.005549 |
| 48000 | 1 | False | 1.061959 |
| 48000 | 1 | True | 1.009305 |
| 48000 | 2 | False | 1.003065 |
| 48000 | 2 | True | 0.755366 |
| 48000 | 3 | False | 0.715778 |
| 48000 | 3 | True | 0.918928 |
| 96000 | 0 | False | 1.248779 |
| 96000 | 0 | True | 0.960356 |
| 96000 | 1 | False | 0.909136 |
| 96000 | 1 | True | 0.953611 |
| 96000 | 2 | False | 0.975050 |
| 96000 | 2 | True | 1.020913 |
| 96000 | 3 | False | 1.159222 |
| 96000 | 3 | True | 1.044847 |
| 192000 | 0 | False | 1.179949 |
| 192000 | 0 | True | 0.948629 |
| 192000 | 1 | False | 0.904257 |
| 192000 | 1 | True | 0.993193 |
| 192000 | 2 | False | 1.034827 |
| 192000 | 2 | True | 1.011374 |
| 192000 | 3 | False | 0.972593 |
| 192000 | 3 | True | 1.008904 |
