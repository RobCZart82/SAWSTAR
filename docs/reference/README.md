# SAWSTAR GUI concept

![SAWSTAR GUI concept](SAWSTAR_GUI_Concept.png)

**Concept — work in progress.** This is a visual design reference, not a
screenshot of a working plugin. The three pages are MAIN / ADVANCED / PRESETS.

Based on the original user-provided `Aurora Synth.png` and the approved SAWSTAR
concept revisions. Updated on 2026-09-07 using the built-in image generation tool.
All three views share the fixed header (MAIN / ADVANCED / PRESETS followed by the
preset selector) and the lower pitch/mod wheels and keyboard above the status row.
The large SAWSTAR wordmark now has the two-line slogan **Simple Synth / Big Sound**
immediately to its right. AI image editing may introduce small changes;
this remains concept artwork, not a pixel-exact UI specification.

## Layout refinement — 2026-09-07

- Keep Simple Synth unchanged. Keep Big Sound's font size; adjust tracking so
  its rendered text width matches Simple Synth in all three shared headers.
  For implementation, measure both text runs rather than insert literal spaces.
- FILTER: KEY TRACK above the bottom-aligned Low Pass 24 selector.
- MIXER: four narrower vertical fader tracks, faint scale ticks and wider,
  rectangular handles; bottom-aligned NOISE selector below the faders.
- OUTPUT: vertical VOLUME fader with the same track/handle language and subtle
  scale; AMOUNT WIDE rotary control directly above the bottom-aligned WIDE switch.
- NOISE, filter-mode and WIDE controls share a consistent bottom inset.

This raster remains an approximate visual reference; the rules above define the
precise layout. White Noise/Pink Noise choices and AMOUNT WIDE describe the design
intent, not newly implemented DSP. The current engine has White/Dark Noise and
per-oscillator Width; this artwork update does not add Pink Noise or a separate
output-wide processor. Plugin controls and sound are unchanged by this update.
