# VST3 tail reporting — development contract

Baseline: main `0bc9799`, 2026-09-30. Previously SAWSTAR never called
`SetTailSize`, so the pinned iPlug2 processor reported zero samples to VST3.
[Steinberg's contract](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/classSteinberg_1_1Vst_1_1IAudioProcessor.html)
defines the report in samples and allows hosts to use it for offline processing
and suspension. A successful headless render does not establish what a particular
host will append or trim.

## Policy

The constructor publishes a default 44.1 kHz report. `OnReset` refreshes it for
the host sample rate. A fixed **380-second** maximum is converted with upward
rounding. Rates use the DSP's 8–384 kHz limits; invalid rates fall back to 44.1 kHz.
Even the highest supported rate remains below iPlug's infinite-tail sentinel.

The fixed maximum covers old FX history after changing a preset or shortening
release/feedback settings. It avoids a report based only on the new settings
discarding a still-audible old tail. It is deliberately conservative: hosts that
append the complete advertised duration may add **6 minutes 20 seconds**,
including silence for ordinary presets. Shorter history-aware reporting is a
future optimization requiring its own host and transition tests.

The report does not alter any sample, fade, envelope, filter or FX buffer. It is
not a timeout on held notes. Sustain and ARP HOLD must be released before a tail
measurement; new MIDI or continuing automation can generate further sound.

## Budget derivation

The production chain is amp envelope → chorus → delay → reverb. Allowances add:

| Stage | Conservative allowance |
| --- | --- |
| Amp release | `10 × ln(101)` = 46.151 s. The ADSR releases toward −0.01; its 10-second setting is a time constant. |
| Chorus | 21 ms maximum feed-forward tap, no feedback. |
| Delay | 196 s at 2-second delay / 85% feedback, using a −120 dB amplitude threshold and the geometric accumulated-history factor `1/(1−feedback)`. |
| Reverb | 132.705 s from a pessimistic RT120 memory/feedback model using the longest/shortest taps over the size range: `2 × 10 × (.0739 × 1.6) / (.0297 × .6)`. |
| Settling and rounding | Add 1 s, then round the 375.877 s sum upward to 380 s. |

This is a **conservative engineering budget verified by rendering**, not a
formal samplewise proof for arbitrary time-varying delay/reverb automation.
Reverb damping, feedback mixing and moving taps make that stronger assertion
unwarranted. The −120 dB convention is practical residual silence, not an exact
zero requirement for an IIR network.

## Automated verification

`tail_contract` compiles the real constructor/reset reporting statements with a
minimal host-report sink. Missing calls fail the extraction check; the baseline
without reporting fails this negative control. Reporting is checked at
44.1/48/96/192/384 kHz, 8 kHz, fractional rates and invalid/out-of-domain rates.

Render fixtures use the production ADSR and chorus/delay/reverb order with warm
history, maximum feedback/time/decay and output boost. Stereo and ping-pong,
small-to-large reverb and abrupt settings before release are covered. The chain
remains audible after 10 seconds; peak and RMS after the advertised end must be
below `1e-6` absolute amplitude. A complete 16-voice Synth run includes every
mixer source, maximum amp release, all effects, width and +24 dB boost; all notes
and pedal are released, voices become idle, and final stereo peak is measured.

Local Release and ASan/UBSan pass. These are headless tests. Platform plugin
builds and VST3 validator run in CI. Native REAPER render acceptance is pending.

Release/RelWithDebInfo runs the complete 380-second residual renders for the full
Synth and both FX fixtures at 8 kHz, plus the FX tail at 44.1 kHz. Sanitizer and
coverage CI enable `SAWSTAR_EXTENDED_TAIL_TESTS=ON` to run the same complete
fixtures. Default Debug checks a short 16-voice prefix (four-second warm history,
two seconds of maximum release plus a one-second observation); the voices must
remain active. Each lighter FX-chain fixture runs through the complete maximum
envelope release (48 seconds plus a one-second observation), checking an idle
envelope, finite output and continuing FX history. This bounded smoke run does
not claim to prove final full-engine convergence or residual silence.
All configurations keep the
full host-rate conversion checks, assertions and normal Debug instrumentation.
No DSP samples are skipped in the complete residual fixtures.
Windows foundation CTest uses two workers for independent tests with isolated
temporary roots. The CPU benchmark remains a separate, sequential step. The
15-minute job limit is unchanged; individual tests have an explicit
600-second fallback timeout instead of relying solely on workflow cancellation.

## Native release acceptance

1. Query the built VST3 tail at 44.1/48/96 kHz and after a sample-rate change.
2. Render ordinary dry presets and the maximum release/FX fixture after releasing
   all notes, sustain and ARP HOLD. Verify audible decay survives and rendering
   ends; record the host's actual trimming/append policy.
3. Change from long FX settings to a dry preset while history is still sounding.
4. Verify bypass/unbypass, editor close and host suspension on the same build.

The finite-report correction is code-complete independently of this manual
acceptance; a new release still requires the native checks.
