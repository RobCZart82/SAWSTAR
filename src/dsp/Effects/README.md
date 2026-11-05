# Stereo effects

Implemented effects are Chorus.h (interpolated modulated delay), Delay.h
(free/tempo-synced stereo or ping-pong feedback delay), Reverb.h (eight-line
feedback delay network) and Width.h (mid-preserving stereo widening).

The engine processes Chorus, Delay, Reverb, then Width. Delay and Reverb allocate
history in Init, not in sample processing. Clear discards history without
reallocation. Effect bypass ramps the wet mix down; delay/reverb discard their
history after becoming fully bypassed. TailContract.h in the plugin directory
reports a conservative host tail budget rather than truncating these tails.

Tests cover the individual effects, including the Chorus ring-boundary regression.
Native host CPU/dropout acceptance remains a separate release gate.
