param([Parameter(Mandatory=$true)][string]$Archive, [ValidateSet('x64','ARM64')][string]$Arch='x64')
$ErrorActionPreference='Stop'
$stage=Join-Path $env:RUNNER_TEMP "sawstar-installer-$Arch"
if(Test-Path $stage){throw "Staging directory already exists: $stage"}
Expand-Archive -Path $Archive -DestinationPath $stage
$candidates=@("${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe", "$env:ProgramFiles\Inno Setup 6\ISCC.exe")
$iscc=$candidates | Where-Object {Test-Path $_} | Select-Object -First 1
if(!$iscc){throw 'Inno Setup 6 is required on the packaging runner.'}
& $iscc "/DStage=$stage" "/DArch=$Arch" packaging/windows/SAWSTAR.iss
if($LASTEXITCODE -ne 0){throw 'Inno Setup failed'}
Get-ChildItem dist/*Setup.exe | ForEach-Object {
 $hash=(Get-FileHash $_.FullName -Algorithm SHA256).Hash.ToLower()
 "$hash  $($_.Name)" | Set-Content ($_.FullName+'.sha256')
}
