# Stereo chorus

Original SAWSTAR DSP, MIT; no additional dependency or third-party source.

Path: voice sum / LFO amp-pan -> stereo chorus -> Output / Level Boost -> peak guard.
Two interpolated delay taps per channel (12 and 17 ms centres, up to +/-4 ms)
use offset sine phases. Left/right buffers remain separate, preserving the
source stereo placement. There is no feedback. The rate is independent of the
modulation LFO and does not consume its selected route.

ADVANCED contains Chorus Off/On, Mix (0-100% linear dry/wet), Rate (0.05-3 Hz)
and Depth (0-100%). Start with On, Mix 25%, Rate 0.3 Hz, Depth 35% on Soft Pad One.
At 100% Mix only the delayed voices remain; moderate Mix usually retains more
attack. Depth zero leaves fixed short delays, not bypass. Use Off or Mix zero
for dry audio. No tempo sync or extra preset is added in this milestone.

Four append-only parameter IDs 42-45 are saved through the existing state codec.
All existing factory presets and old states default to Off. Their dry samples
remain unchanged; parameters, enable and bypass transitions are smoothed over
20 ms to avoid abrupt changes. Off becomes exact passthrough after settling.
Delay history continues to receive audio while bypassed, and resets clear it.
Effect phase/history are transient, not serialized, as with the existing LFO.

Fixed buffers and per-sample processing use no allocation or locks. Delay storage
supports rates up to 384 kHz; higher rates use the capped chorus timing rate.
The dry path has no added latency. Wet delay tail is at most 21 ms at supported
rates. Stereo/mono-sum tone may change through the normal dry/delayed interference.

Tests cover exact bypass/zero Mix, stereo motion, bounds, automation extremes,
reset/tail and invalid settings at 44.1/48/96/192 kHz; Soft Pad One with chorus
and bypass identity at 44.1/48/96 kHz; state migration and current state roundtrip.
