# Stereo / ping-pong delay

Original SAWSTAR DSP (MIT), with no additional third-party dependency.
Path: voice sum -> chorus -> delay -> Output / Level Boost -> peak guard.

ADVANCED now groups LFO/performance on the left and effects on the right.
Delay controls: Off/On, Stereo/Ping-pong, Free ms/Tempo Sync, rhythmic division,
Mix, Time, Feedback and Tone. Time ranges from 1 to 2000 ms; sync replaces Time.
Divisions: 1/16, 1/8, dotted 1/8, 1/4, dotted 1/4, 1/2, 1/1.
Long sync values are capped at 2 seconds. Missing BPM falls back to 120 BPM.
Sync follows the host BPM, not transport/bar phase. Delay time changes glide
smoothly and can pitch-shift the existing echoes, as with a moving delay head.

Stereo preserves separate channels. Ping-pong feeds the mono average of the
input into the first left echo, then cross-feeds successive echoes right/left.
The dry signal always retains its original stereo placement. Tone (200-16000 Hz)
is a one-pole low-pass on the wet returns and feedback. Feedback is capped at
85%; there is no freeze or self-oscillation. Mix is linear dry/wet: 100% is wet only.

Off fades the wet output out, then invalidates history in constant time; old
repeats cannot return when re-enabled. Mix zero is exact dry output but keeps
the enabled delay running. Reset clears delay memory. Time/feedback/mix/mode/tone
changes have 20 ms one-pole smoothing. Parameters persist, echo buffers do not.
Note-off allows echoes to decay; no voice is needed to keep processing effects.

Buffers allocate only on engine Reset (preparation), never in Process or Set.
Storage supports 2 seconds at rates up to 384 kHz (higher timing rates clamp).
No extra dry-path latency. Feedback creates a decaying tail, potentially tens
of seconds at maximum time/feedback. The existing output peak guard remains last.

All eight factory presets and old projects default to Delay Off. Start on
SuperSaw One with On, Ping-pong, Tempo Sync, dotted 1/8, Mix 20%, Feedback 30%,
Tone 6000 Hz. No existing preset is overwritten.

Appended parameter IDs: 46 delay.enabled (Off), 47 delay.mix (20%),
48 delay.time_ms (350 ms), 49 delay.feedback (30%), 50 delay.tone_hz (6000 Hz),
51 delay.mode (Stereo), 52 delay.sync (Free ms), 53 delay.division (1/4).
