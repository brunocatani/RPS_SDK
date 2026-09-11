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

# The SDK checks out LF while the runtime checkout may use CRLF. Compare the
# complete, case-sensitive header text after normalizing only those newlines.
$sdkText = (Get-Content -Raw -Encoding utf8 -LiteralPath $sdkHeader).Replace("`r`n", "`n")
foreach ($authority in @($sourceHeader, $paperSdkHeader)) {
    $authorityText = (Get-Content -Raw -Encoding utf8 -LiteralPath $authority).Replace("`r`n", "`n")
    if ($sdkText -cne $authorityText) {
        throw "RPS SDK PAPER header differs from $authority"
    }
}

Write-Host 'RPS SDK PAPER header sync checks passed.'
