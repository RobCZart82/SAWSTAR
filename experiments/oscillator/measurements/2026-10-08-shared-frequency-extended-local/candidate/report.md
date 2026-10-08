# Isolated oscillator frequency sharing study

Source: e3994b657b33da088438972f40de0e662ad7bd50
32 oscillator banks represent OSC1 + OSC2 for 16 voices; no engine, filter or FX timing.
8 alternating pairs of 131072 frames. Ratios are study/reference; smaller is favorable.
Static and audio-rate pitch modulation are separate controls. No native realtime acceptance.

CPU acceptance: not established. Production promotion: not allowed by this isolated study.
Median slowdown cells: 3/24; all 8 pairs slower: 0/24.
These are observations, not statistical significance; green CI validates the study only.

| Rate | Waveform | Pitch modulation | Median paired time ratio |
| --- | --- | --- | --- |
| 48000 | 0 | False | 1.000037 |
| 48000 | 0 | True | 0.997869 |
| 48000 | 1 | False | 0.932257 |
| 48000 | 1 | True | 0.935820 |
| 48000 | 2 | False | 0.964139 |
| 48000 | 2 | True | 0.963181 |
| 48000 | 3 | False | 0.970191 |
| 48000 | 3 | True | 0.975178 |
| 96000 | 0 | False | 1.001212 |
| 96000 | 0 | True | 0.995641 |
| 96000 | 1 | False | 0.932669 |
| 96000 | 1 | True | 0.926423 |
| 96000 | 2 | False | 0.962566 |
| 96000 | 2 | True | 0.963884 |
| 96000 | 3 | False | 0.971730 |
| 96000 | 3 | True | 0.977599 |
| 192000 | 0 | False | 1.009570 |
| 192000 | 0 | True | 0.994258 |
| 192000 | 1 | False | 0.933943 |
| 192000 | 1 | True | 0.932687 |
| 192000 | 2 | False | 0.962414 |
| 192000 | 2 | True | 0.966868 |
| 192000 | 3 | False | 0.970941 |
| 192000 | 3 | True | 0.979231 |

Campaign: extended-v1; comparison: candidate.
Reference-repeat runs the identical SevenSaw function on both paths; it is a timing control, not a candidate gain.
All raw observations and ratio ranges are retained. No baseline subtraction, outlier exclusion or production acceptance.
Longer measurements reduce short-timer sensitivity; runner scheduling and background load remain uncontrolled.
