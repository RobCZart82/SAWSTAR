# GUI

The shell layout lives in src/plugin/SAWSTAR.cpp and uses shared PageButton
controls. MAIN, ADVANCED and PRESETS switch real control groups; MAIN exposes
five parameters and PRESETS can restore Init. The final concept is a reference,
not a pixel-exact description of this initial shell.
