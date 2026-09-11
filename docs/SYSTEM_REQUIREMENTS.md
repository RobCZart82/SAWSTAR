# System requirements - candidate targets

Development tools and end-user requirements are separate. The following OS
versions are **build/installer targets, not yet verified minimum support claims**.

| Candidate | Target OS | Host architecture | Graphics |
| --- | --- | --- | --- |
| Windows x64 | Windows 10 64-bit or newer | x64 VST3 | OpenGL 2-capable driver |
| Windows ARM64 | Windows 11 ARM64 | native ARM64 VST3 | OpenGL 2-capable driver |
| macOS Universal | macOS 11 or newer | Intel x86_64 or Apple Silicon arm64 VST3 | Metal |

A plugin must match its host process architecture. Native ARM64 is different
from an x64 DAW running under emulation. No ARM64EC support is claimed.
macOS deployment target is explicitly 11.0 in the candidate workflow.
Windows compiler definitions target Windows 10 APIs; the ARM installer requires
Windows 11. This alone is not proof that every API/driver/host combination works.

Windows uses NanoVG/OpenGL, not DirectX; no separate DirectX runtime is specified.
The project selects static MSVC runtime (/MT Release, /MTd Debug). Inspect final
binary imports before claiming no separate VC++ Redistributable requirement.
The Windows UCRT/OS components still come from the operating system.

Visual Studio, CMake, Git, compilers and the VST3 SDK are development-only tools.
There is no separate factory-bank installation. No AU, CLAP or standalone app.

The GUI is 1280 x 760 logical pixels with 75/100/125% scaling. Allow room for
host window borders and OS display scaling; 1280 x 800 alone is not a guarantee
of fitting the 100% view. RAM recommendations such as 4/8 GB have not been
measured and are not advertised as tested minima.

## Acceptance record

RC1 Windows manual acceptance was reported by the user, without OS/host version
details. RC1 supports x64 Windows and arm64 Mac only. RC2 adds ARM64 Windows and
Intel Mac builds; each needs its own validation and real-host acceptance.
Minimum supported OS versions may be published only after testing the final
package on the corresponding minimum system. CI success on a newer OS does not
establish backward compatibility.
