# Filter character: Drive and four modes

The MAIN page now has a filter-mode selector and a 0–24 dB Drive slider above
the existing filter controls. Raise Filter Mix above zero to hear either.
All four mixer sources feed the same per-voice filter, then Amp ADSR/velocity.

| Mode | Implementation / character |
| --- | --- |
| Low Pass 12 | Original two-pole TPT low-pass; default and old-project sound. |
| Low Pass 24 | Original low-pass followed by a second, non-resonant two-pole stage. Stronger high-frequency rejection; not a ladder-filter emulation. |
| High Pass 12 | Two-pole high-pass from the first TPT stage. |
| Band Pass 12 | Two-pole band-pass; each outer skirt falls at 6 dB/oct. Center gain is normalized with the damping coefficient. |

Cutoff, Resonance, Filter ADSR, key tracking and mod-wheel cutoff control work
in every mode. Resonance remains Q 0.5–10 in the first stage; the LP24 extra
stage uses Q 0.5 to avoid multiplying resonant peaks. BP bandwidth changes with
resonance. Modes crossfade with 10 ms one-pole smoothing; the second stage stays
warm while other modes play. Filter state is cleared on voice/engine reset.

Drive adds tanh saturation before filtering. Gain is 10^(dB/20); the saturated
branch is normalized by tanh(gain), with a gradual dry-to-saturated blend over
0–6 dB. Drive itself is smoothed over 10 ms. At 0 dB the input is unchanged.
Drive is a timbre control, not an output-loudness correction. Filter Mix 0 is
an exact dry path, bypassing both drive and filtering after its mix settles.

This first drive implementation runs at the host sample rate, without
oversampling. Strong drive on bright/high notes can create aliasing; improving
that quality/CPU tradeoff is a later DSP step. It adds no buffer latency and
makes no analog-model or mastering-limiter claims. The existing stereo output
peak guard remains downstream of the complete voice sum.

## Compatibility and tests

Append-only IDs 31 (`filter.drive_db`, 0–24, default 0) and 32 (`filter.mode`,
0–3, default 0). Existing IDs and mappings remain unchanged. Missing fields in
older projects default to undriven LP12. The v1 codec now writes 33 records,
396 payload bytes / 412 total bytes, plus the framework's separate bypass.

Tests compare rejection slopes and passbands, measure Drive's third harmonic,
check dry bypass/channel isolation, sweep mode/drive/cutoff/resonance, clear
all stages and stress all four sources over 16 voices in every mode at
44.1/48/96 kHz. Codec tests include the previous 31-record mixer state.

## Verified build — 2026-09-06

Code: `6c672379a638ba548546791b8550624ac9108fb2`.
[macOS CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34052415885) and
[Windows CI](https://github.com/RobCZart82/SAWSTAR/actions/runs/34052416013)
passed 12 test executables in Debug/Release and 47 VST3 validator checks each.

REAPER 7.79 on macOS loaded the previous mixer project as LP12 / Drive 0.
Its rendered 24-bit PCM was bit-identical to the preceding mixer-build demo
(2,194,626 audio bytes compared). The selector and Drive control were exercised;
a saved LP24 / Drive 12 dB project was closed and reopened with both restored.
The driven LP24 demo rendered at 44.1 kHz stereo / 24-bit, peak -11.571 dBFS,
zero clipped samples, no post-render normalization.

Manual Windows host testing of this build and exhaustive listening/automation
acceptance remain pending.
