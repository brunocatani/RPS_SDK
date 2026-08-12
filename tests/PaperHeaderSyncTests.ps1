param(
    [string]$Root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path,
    [Parameter(Mandatory = $true)]
    [string]$PaperRoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$sdkHeader = Join-Path $Root 'SDK/PAPER/include/PAPERApi.h'
$sourceHeader = Join-Path $PaperRoot 'src/api/PAPERApi.h'
$paperSdkHeader = Join-Path $PaperRoot 'SDK/PAPER/include/PAPERApi.h'

foreach ($path in @($sdkHeader, $sourceHeader, $paperSdkHeader)) {
    if (-not (Test-Path -LiteralPath $path)) {
        throw "PAPER header sync input is missing: $path"
    }
}

$sdkHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $sdkHeader).Hash
foreach ($authority in @($sourceHeader, $paperSdkHeader)) {
    $authorityHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $authority).Hash
    if ($sdkHash -ne $authorityHash) {
        throw "RPS SDK PAPER header differs from $authority"
    }
}

Write-Host 'RPS SDK PAPER header sync checks passed.'
