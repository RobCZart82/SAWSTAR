# Master WIDE

OUTPUT has a WIDE switch and AMOUNT WIDE (0–100%). Default: off, amount 50%.
The existing Boost control remains above it. No concept image was changed.

The processor sits after chorus/delay/reverb, before output gain and the existing
stereo-linked peak guard. It multiplies the existing side signal by 1–2 while
preserving mid. At 0% it is neutral; 100% doubles side (+6 dB). It does not create
stereo from mono. More width can increase stereo level; the peak guard still
limits peaks to 0.98. When protection engages, the linked gain may also change
the mono level. Below that threshold the mono sum is preserved within rounding.
There is no delay, modulation, additional latency or memory allocation.

Enable, disable and amount automation ramp over at most 20 ms. Settled bypass is
bit-identical. Original oscillator WIDTH parameters remain independent.

IDs 90 (`output.wide`) and 91 (`output.wide_amount`) append to the existing IDs.
State encoding stays version 1: older states omit the new IDs and load with WIDE
off. Factory defaults remain off, preserving their sound. Both parameters are
saved with state and exposed to host automation.

`master_width` tests 44.1/48/96 kHz: exact bypass, mono sum, mono input, side gain,
switch smoothing, invalid amount and full-engine peak protection with 16 notes,
both oscillators, sub, noise and all effects. State tests cover the old 90-ID
format and new parameter round-trip. Native Windows visual testing remains manual.

Build validation for code `15dc96561e60f97e32134d95a854ad410a2de42e`:

- [macOS CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34232807006)
- [Windows CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34232807034)

No REAPER listening test or system plugin replacement is part of this change.
