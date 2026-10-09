# Isolated oscillator unchanged reference repeat study

Source: e3994b657b33da088438972f40de0e662ad7bd50
32 oscillator banks represent OSC1 + OSC2 for 16 voices; no engine, filter or FX timing.
8 alternating pairs of 131072 frames. Ratios are study/reference; smaller is favorable.
Static and audio-rate pitch modulation are separate controls. No native realtime acceptance.

CPU acceptance: not established. Production promotion: not allowed by this isolated study.
Median slowdown cells: 15/24; all 8 pairs slower: 0/24.
These are observations, not statistical significance; green CI validates the study only.

| Rate | Waveform | Pitch modulation | Median paired time ratio |
| --- | --- | --- | --- |
| 48000 | 0 | False | 0.996240 |
| 48000 | 0 | True | 1.000823 |
| 48000 | 1 | False | 1.000218 |
| 48000 | 1 | True | 0.997060 |
| 48000 | 2 | False | 1.001730 |
| 48000 | 2 | True | 1.000801 |
| 48000 | 3 | False | 1.002609 |
| 48000 | 3 | True | 1.002491 |
| 96000 | 0 | False | 0.998428 |
| 96000 | 0 | True | 1.001094 |
| 96000 | 1 | False | 0.998610 |
| 96000 | 1 | True | 1.000123 |
| 96000 | 2 | False | 1.002657 |
| 96000 | 2 | True | 1.002573 |
| 96000 | 3 | False | 1.000684 |
| 96000 | 3 | True | 1.003116 |
| 192000 | 0 | False | 1.000684 |
| 192000 | 0 | True | 0.996868 |
| 192000 | 1 | False | 0.997431 |
| 192000 | 1 | True | 0.997127 |
| 192000 | 2 | False | 0.999936 |
| 192000 | 2 | True | 0.999602 |
| 192000 | 3 | False | 1.000483 |
| 192000 | 3 | True | 1.006677 |

Campaign: extended-v1; comparison: reference-repeat.
Reference-repeat runs the identical SevenSaw function on both paths; it is a timing control, not a candidate gain.
All raw observations and ratio ranges are retained. No baseline subtraction, outlier exclusion or production acceptance.
Longer measurements reduce short-timer sensitivity; runner scheduling and background load remain uncontrolled.
