param([ValidateSet('x64','ARM64')][string]$Arch='x64')
$ErrorActionPreference='Stop'
# Run only on disposable Actions runners, never on a developer workstation.
if($env:GITHUB_ACTIONS -ne 'true'){throw 'Installer smoke test requires disposable CI'}
$setup=Get-ChildItem "dist/*-$Arch-Setup.exe" | Select-Object -First 1
if(!$setup){throw 'Installer missing'}
$folder=if($Arch -eq 'ARM64'){'arm64-win'}else{'x86_64-win'}
$binary="$env:CommonProgramFiles\VST3\SAWSTAR.vst3\Contents\$folder\SAWSTAR.vst3"
if(Test-Path $binary){throw 'Refusing to replace an existing plugin in smoke test'}
$presets="$env:APPDATA\SAWSTAR\Presets"
New-Item -ItemType Directory -Force $presets | Out-Null
$sentinel=Join-Path $presets 'installer-qa-sentinel.txt'
if(Test-Path $sentinel){throw 'Sentinel already exists'}
'preserve user files' | Set-Content $sentinel
$p=Start-Process $setup.FullName -ArgumentList '/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART' -Wait -PassThru
if($p.ExitCode -ne 0){throw "Install exit $($p.ExitCode)"}
if(!(Test-Path $binary)){throw 'Installed VST3 missing'}
if((Get-FileHash $binary).Hash -ne (Get-FileHash "build-plugin/out/SAWSTAR.vst3/Contents/$folder/SAWSTAR.vst3").Hash){throw 'Installed binary mismatch'}
$dir="$env:ProgramFiles\SAWSTAR-$Arch"
foreach($lang in @('EN','HU')){if(!(Test-Path "$dir\docs\manuals\SAWSTAR-User-Manual-$lang.pdf")){throw 'Manual missing'}}
$metadata=Get-Content (Join-Path $PSScriptRoot '../release.json') -Raw | ConvertFrom-Json
foreach($relative in @('docs/INSTALLATION.md','docs/SYSTEM_REQUIREMENTS.md','docs/THIRD_PARTY.md',"docs/RELEASE_NOTES_$($metadata.version).md",'THIRD_PARTY_NOTICES.md')){
 $installed=Join-Path $dir $relative
 if(!(Test-Path $installed)){throw "Installed document missing: $relative"}
 # Compare to the extracted, hashed package, including normalized notices.
 $source=Join-Path (Join-Path $env:RUNNER_TEMP "sawstar-installer-$Arch") $relative
 if((Get-FileHash $installed).Hash -ne (Get-FileHash $source).Hash){throw "Installed document mismatch: $relative"}
}
$p=Start-Process "$dir\unins000.exe" -ArgumentList '/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART' -Wait -PassThru
if($p.ExitCode -ne 0){throw "Uninstall exit $($p.ExitCode)"}
if(Test-Path $binary){throw 'Plugin binary remains after uninstall'}
if((Get-Content $sentinel) -ne 'preserve user files'){throw 'User file changed'}
Remove-Item $sentinel
Write-Output 'Installer/uninstaller, binary identity, manuals and user-file preservation: PASS'
