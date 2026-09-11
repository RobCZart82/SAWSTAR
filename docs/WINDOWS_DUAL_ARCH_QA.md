# Windows x64 / ARM64 coexistence acceptance

Status: pending execution on Windows 11 ARM64; this is not covered by the
existing single-architecture install/uninstall CI jobs.

Use a disposable system with both new installers from the same commit.
Preserve a user preset as a sentinel. Verify these sequences:

1. Install x64, then ARM64. Confirm both Contents/x86_64-win and
   Contents/arm64-win binaries exist and their hashes match the packages.
2. Open each plugin in a matching host and check resources, fonts and presets.
3. Uninstall x64. ARM64 must still load with its resources intact.
4. Uninstall ARM64. Plugin files should be gone; user presets must remain.
5. Repeat with ARM64 installed first and ARM64 uninstalled first.
6. Repeat upgrading one architecture while the other remains installed.

The installers retain separate AppIds and use Inno shared-file reference
counting for the VST3 bundle files. Shared resource paths must not be removed
until the last owning installer is uninstalled. Older candidate installers did
not record these shared-file references: remove those candidates before this
coexistence test. Record OS, installers, commit, host architectures and results.
