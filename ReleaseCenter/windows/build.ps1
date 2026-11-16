[CmdletBinding()]
param()
$ErrorActionPreference = "Stop"
$root=$env:GITHUB_WORKSPACE
$name=$env:LIBERTY_PRODUCT_NAME
$target=$env:LIBERTY_TARGET
$slug=$env:LIBERTY_SLUG
$version=$env:LIBERTY_VERSION
if (!$name -or !$target -or !$slug -or !$version) { throw "Release metadata is missing" }
$base=Join-Path $root "build/${target}_artefacts/Release"
$bundle=Join-Path $base "VST3/$name.vst3"
if (!(Test-Path -LiteralPath $bundle -PathType Container)) { throw "VST3 bundle not found: $bundle" }
$binary=Get-ChildItem -LiteralPath $bundle -Recurse -File -Filter '*.vst3' | Where-Object { $_.DirectoryName -like '*x86_64-win*' } | Select-Object -First 1
if (!$binary -or $binary.Length -lt 100000) { throw "Invalid Windows x64 binary" }
$standalone=""
if ($env:LIBERTY_STANDALONE -eq "true") {
  $candidate=Join-Path $base "Standalone/$name.exe"
  if (Test-Path -LiteralPath $candidate -PathType Leaf) { $standalone=$candidate }
  else { throw "Expected standalone EXE missing: $candidate" }
}
$out=Join-Path $root "release-output"
New-Item -ItemType Directory -Force -Path $out | Out-Null
$env:LIBERTY_VST3_BUNDLE=$bundle
$env:LIBERTY_STANDALONE_EXE=$standalone
$env:LIBERTY_OUTPUT_DIR=$out

$paths=@(
  (Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6/ISCC.exe'),
  (Join-Path $env:ProgramFiles 'Inno Setup 6/ISCC.exe')
)
$iscc=$paths | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
if (!$iscc) {
  choco install innosetup --yes --no-progress --limit-output
  if ($LASTEXITCODE -ne 0) { throw "Inno Setup install failed" }
  $iscc=$paths | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
}
if (!$iscc) { throw "Inno Setup compiler not found" }
& $iscc "ReleaseCenter/windows/Liberty.iss"
if ($LASTEXITCODE -ne 0) { throw "Inno Setup compilation failed" }
$installer=Join-Path $out "LIBERTY-$slug-Windows-x64-v$version-UNSIGNED.exe"
if (!(Test-Path -LiteralPath $installer -PathType Leaf)) { throw "Missing installer: $installer" }
if ((Get-Item -LiteralPath $installer).Length -le 100000) { throw "Installer too small" }
Write-Host "Verified unsigned Windows installer: $installer"
