# Stereo reverb

Original SAWSTAR eight-line feedback delay network (MIT). No new third-party
code, dependencies, samples or impulse responses. This is an algorithmic space,
not a sampled room or a claim to emulate a particular hardware reverb.

Signal path: voice sum -> chorus -> delay -> reverb -> Output / Level Boost ->
stereo peak guard. ADVANCED keeps LFO/performance on the left; CHORUS / DELAY /
REVERB buttons choose the effect panel on the right. Selecting a panel does not
turn its effect on/off. The main three tabs, keyboard and wheels remain fixed.

Controls: Off/On; Mix 0-100% (linear dry/wet); Size 0-100%; Decay 0.2-10 seconds;
Damping 500-16000 Hz. Lower Damping Hz darkens the tail. Size changes delay-line
lengths, with smoothing; changing it during a tail can bend pitch. Decay sets
the nominal low-frequency decay target; damping/interpolation shorten the
actual high-frequency tail. There is no freeze or separate pre-delay control.

All existing presets and older projects restore Reverb Off. Start Soft Pad One
with On, Mix 20%, Size 50%, Decay 2.5 s, Damping 6000 Hz. Moderate wet levels
retain the direct sound. At Mix 100% only the reverberated signal remains.

Off smoothly mutes the tail then invalidates it without a large buffer clear
in the audio callback. Mix zero keeps the enabled network running, but returns
exact dry samples. Reset clears memory. Effect history is transient; parameters
persist. No extra dry latency. The host must keep processing for the tail to
sound, and offline render bounds must include the desired decay.

An orthogonal Householder feedback matrix mixes eight damped delay lines.
Feedback gains derive from line lengths and the requested decay. Storage is
allocated only on Reset, never on Process or Set. Supported timing rates are
8-384 kHz; higher rates are capped. Smoothing uses double precision for delay
lengths. No locks, shared RNG or global mutable DSP state.

Append-only IDs: 54 reverb.enabled (Off), 55 reverb.mix (20%),
56 reverb.size (50%), 57 reverb.decay_s (2.5), 58 reverb.damping_hz (6000).

Tests: impulse response, stereo energy, decay/size/damping response, automation,
bypass/zero Mix, cleared history, reset, invalid inputs and old-state migration;
Soft Pad One dry identity and audible bounded reverb. DSP tests cover
44.1/48/96/192 kHz. Manual listening/UI/REAPER restart validation remains pending.
