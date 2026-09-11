#ifndef Arch
  #define Arch "x64"
#endif
#ifndef Stage
  #error Stage must identify the extracted package directory
#endif
#ifndef Version
  #define Version "1.0.0"
#endif
[Setup]
AppId=SAWSTAR-VST3-{#Arch}
AppName=SAWSTAR ({#Arch})
AppVersion={#Version}
AppPublisher=Gyuricza Róbert
AppPublisherURL=https://github.com/RobCZart82/SAWSTAR
DefaultDirName={autopf}\SAWSTAR-{#Arch}
DefaultGroupName=SAWSTAR ({#Arch})
DisableDirPage=yes
PrivilegesRequired=admin
#if Arch == "ARM64"
ArchitecturesAllowed=arm64
ArchitecturesInstallIn64BitMode=arm64
MinVersion=10.0.22000
#else
ArchitecturesAllowed=x64os
ArchitecturesInstallIn64BitMode=x64os
MinVersion=10.0
#endif
LicenseFile={#Stage}\LICENSE
OutputDir=..\..\dist
OutputBaseFilename=SAWSTAR-{#Version}-rc2-Windows-{#Arch}-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName=SAWSTAR ({#Arch})
[Files]
Source: "{#Stage}\SAWSTAR.vst3\*"; DestDir: "{commoncf64}\VST3\SAWSTAR.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#Stage}\docs\manuals\*"; DestDir: "{app}\manuals"; Flags: ignoreversion recursesubdirs
Source: "{#Stage}\licenses\*"; DestDir: "{app}\licenses"; Flags: ignoreversion recursesubdirs
Source: "{#Stage}\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Stage}\THIRD_PARTY_NOTICES.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Stage}\INSTALLATION.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Stage}\SYSTEM_REQUIREMENTS.md"; DestDir: "{app}"; Flags: ignoreversion
[Icons]
Name: "{group}\User Manual - English"; Filename: "{app}\manuals\SAWSTAR-User-Manual-EN.pdf"
Name: "{group}\Használati útmutató - Magyar"; Filename: "{app}\manuals\SAWSTAR-User-Manual-HU.pdf"
Name: "{group}\SAWSTAR website"; Filename: "https://github.com/RobCZart82/SAWSTAR"
