# VST3 bypass timing diagnostic

The offline adapter test compiles the patched parameter-processing method
from the pinned adapter fixture. With bypass initially off, a host queue that
turns it on at sample 16 and off at sample 48 leaves it off before rendering.
A separate queue that turns it on at sample 48 applies bypass before rendering.
The test also checks that bypass points stay out of the MIDI controller path.

The host calls parameter processing before audio rendering. The current
adapter takes the last point per bypass queue, which loses the first pulse
and applies single future points prematurely. This diagnostic observes adapter
state and does not render audio in a DAW.

A timing fix needs to preserve bypass edges into rendering, keep synth and
effect state advancing during muted samples, and retain the final bypass
state for the next block. Replace the current characterization with checks for
audible output before, inside and after the bypass window when that fix lands.
Normal parameter automation and K1 remain separate open issues.

Local verification compiled and ran the adapter test with C++17 and warnings
enabled. The diagnostic makes no shipping code changes.
